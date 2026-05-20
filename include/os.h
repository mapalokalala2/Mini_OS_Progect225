#ifndef OS_H
#define OS_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define MAX_SEGMENTS 256
#define MAX_PROCESSES 64
#define MAX_NAME_LEN 32
#define MEMORY_SIZE 1024


typedef enum { // contains the enums for the process states
    NEW, 
    READY, 
    RUNNING, 
    WAITING, 
    TERMINATED 
} proc_state;
//
typedef struct {
    //this just contians information of the process.
    int pid;
    char name[MAX_NAME_LEN];
    int priority;
    int burst_time;
    int remaining_time;
    int arrival_time;
    proc_state state;
    int mem_address;
    int mem_size;
} pcb;

typedef enum { 
        SCHED_FCFS,
        SCHED_ROUNDROBIN,
        SCHED_PRIORITY } sched;

typedef struct {
    int pid;
    int start_time;
    int duration;
    int end_time;
} segment;

//Function Prototypes
void init_system(void);
int create_process(const char *name, int burst, int priority, int mem_size);
void list_of_processes(void);
int scheduler_selection(sched stype, int time_quantum, segment *segments, int max_segments);
void show_memory(void);
void show_logs(void);
void pcb_print_all(void);
int pcb_add(const char *name, int burst, int priority, int mem_size);
pcb *get_process_table(void);
int get_process_count(void);
void terminate_process(int pid);

#endif
