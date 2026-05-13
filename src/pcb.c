#include <string.h>
#include "../include/os.h"
#include "../include/pcb.h"
#include "../include/memory.h"
#include "../include/logger.h"


extern void banker_release_all(int pid);// extern is a key word used to declare a function that is defined in another file. in this case, it allows the banker.c file 
                                        // to call the banker_release_all function which is defined in the banker.c file, without causing a compilation error due to 
                                        // missing function definition. it tells the compiler that the function exists and will be linked during the linking phase of compilation.

static pcb process_table[MAX_PROCESSES];
static int process_count = 0;
static int clock_tick = 0;
static int next_pid = 1;

//System initalization


void init_system(void){
    for(int i = 0; i < MAX_PROCESSES; i++){
        process_table[i].state = TERMINATED;
        process_table[i].mem_address= -1;
        process_table[i].pid =0; // this will creat an empty slot;
    }
    next_pid = 1;
    process_count = 0;
    printf("THE SYSTEM IS INITIALISED \n");
    log_event("The system was initalised");
}

//process creation

int create_process(const char *name, int burst_time, int priority, int mem_size){
    if(process_count >= MAX_PROCESSES){
        return -1;
    }

    int p_slot = -1;
    for(int i = 0; i < MAX_PROCESSES; i++){
        if(process_table[i].pid == 0){
            p_slot = i;
            break;
        }
    }

    if(p_slot == -1) return -1;

    pcb *pro = &process_table[p_slot];
    pro->pid = next_pid++;
    pro->arrival_time = clock_tick++;
    pro->burst_time = burst_time;
    pro->remaining_time = burst_time;
    pro->priority = priority;
    pro->state = READY;
    pro->mem_size = mem_size;
    pro->mem_address = memory_allocation(pro->pid, mem_size); // Call the correct allocation function

    strncpy(pro->name, name, MAX_NAME_LEN - 1);
    pro->name[MAX_NAME_LEN - 1] = '\0';
    
    process_count++;
    printf("process '%d' named '%s' has been successfully created\n", pro->pid, pro->name);
    log_event("process created successfully");
    
    return pro->pid;
}

void list_of_processes(void){
    pcb_print_all();
}


void pcb_init_table(void) {
    memset(process_table, 0, sizeof(process_table));
    process_count = 0;
}

// pcb *pcb_get_table(int pid) {
//     for (int i = 0; i < MAX_PROCESSES; i++) {
//         if (process_table[i].pid == pid) {
//             return &process_table[i];
//         }
//     }
//     return NULL;
// }

int pcb_add(const char *name, int burst, int priority, int mem_size) {
    return create_process(name, burst, priority, mem_size);
}

void pcb_print_all(void) {
    printf("\n%-5s %-15s %-12s %-10s %-10s %-10s\n", 
           "PID", "Name", "State", "Priority", "Memory", "Rem. Time");
    printf("--------------------------------------------------------------------------\n");
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid != 0) {
            const char* state_str;
            switch(process_table[i].state) {
                case READY:      state_str = "READY"; break;
                case RUNNING:    state_str = "RUNNING"; break;
                case WAITING:    state_str = "WAITING"; break;
                case TERMINATED: state_str = "TERMINATED"; break;
                default:         state_str = "UNKNOWN"; break;
            }
            
            printf("%-5d %-15s %-12s %-10d %-10d %-10d\n", 
                   process_table[i].pid, 
                   process_table[i].name, 
                   state_str, 
                   process_table[i].priority, 
                   process_table[i].mem_size, 
                   process_table[i].remaining_time);
        }
    }
}

pcb *get_process_table(void){
    return process_table;
}

int get_process_count(void){
    return process_count;
}

void delete_process(int pid) {
    if (pid <= 0) {
        return;
    }
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid == pid) {
            log_event("process '%d' named '%s' has been successfully DELETED", pid, process_table[i].name);
            memory_free(process_table[i].pid);
            process_table[i].pid = 0; // Mark as empty for reuse, consistent with init_system
            process_table[i].state = TERMINATED;
            process_count--;
            return; // Optimization: process found and deleted
        }
    }
}

void terminate_process(int pid) {
    delete_process(pid);
}

void delete_all_processes(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].pid != 0) {
            log_event("process '%d' named '%s' has been successfully DELETED", process_table[i].pid, process_table[i].name);
            memory_free(process_table[i].pid); 
            process_table[i].pid = 0; // Mark as empty for reuse, consistent with init_system
            process_table[i].state = TERMINATED;
        }
    }
    process_count = 0;
}