#include <stdio.h>
#include "../include/os.h"
#include "../include/pcb.h"
#include "../include/scheduler.h"
#include "../include/memory.h"
#include "../include/banker.h"
#include "../include/logger.h"

static segment segs [MAX_SEGMENTS];

// Glue functions to match os.h declarations with module implementations
void show_memory(void) {
    memory_display();
}

void show_logs(void) {
    show_log();
}

void view_menu(void) {
    printf("\n================ Mini OS Menu ===============\n");
    printf(" Welcome to the Mini OS!\n");
    printf(" 1. Create an EMERGENCY Process\n");
    printf(" 2. List Processes\n");
    printf(" 3. Show Memory Map\n");
    printf(" 4. Show System Logs\n");
    printf(" 5. Run Scheduler (Round Robin)\n");
    printf(" 6. Run Scheduler (FCFS)\n");
    printf(" 7. Run Scheduler (Priority)\n");
    printf(" 8. Deadlock handling\n");
    printf(" 9. Exit\n");
    printf("==============================================\n");
}
    


int main(void) {
    // this is the initialisation of the system, it will call all the init functions for the different modules
    log_init(); 
    init_system();
    memory_init();
    

    log_event("Mini OS initialized successfully.");
    printf("Mini OS initialized successfully.\n");
    int choice = 1;
    int burst, priority, mem, pid, time_quantum;
    char name[MAX_NAME_LEN];
    int num_resources = MAX_RESOURCES; // Track how many are actually used
    sched sched_type;

    while(choice != 9) {
        view_menu();
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n'); // Clear the input buffer
            continue;
        }

        switch(choice) {
            case 1:
                printf("YOU HAVE SELECTED TO CREATE AN EMERGENCY PROCESS\n");
                printf("Enter process name: ");
                // Clear the newline character left in the buffer by the previous scanf
                while (getchar() != '\n'); 
                // Read the entire line, including spaces, up to MAX_NAME_LEN-1 characters
                fgets(name, MAX_NAME_LEN, stdin);
                name[strcspn(name, "\n")] = 0; // Remove the trailing newline character if present
                printf("Enter burst time: ");
                scanf("%d", &burst);
                printf("Enter priority (lower number = higher priority)(1-10): ");
                scanf("%d", &priority);
                printf("Enter memory size: ");
                scanf("%d", &mem);
                pid = create_process(name, burst, priority, mem);
                printf("Process created with PID: %d\n", pid);
                break;
            case 2:
                list_of_processes();
                if(get_process_count() != 0) {
                    printf("would you like to delete a process? (y/n): ");
                    char del_choice;
                    scanf(" %c", &del_choice);
                    if (del_choice == 'y'|| del_choice == 'Y') {
                        printf("Enter PID of process to delete: ");
                        int del_pid;
                        scanf("%d", &del_pid);
                        delete_process(del_pid);
                    }
                }

                break;
            case 3:
                show_memory();
                break;
            case 4:
                show_logs();
                break;
            case 5:
                // Run Scheduler (Round Robin)
                printf("Enter time quantum: ");
                scanf("%d", &time_quantum);
                scheduler_selection(SCHED_ROUNDROBIN, time_quantum, segs, MAX_SEGMENTS);
                break;
            case 6:
                // Run Scheduler (FCFS)
                 scheduler_selection(SCHED_FCFS, 0, segs, MAX_SEGMENTS);
                break;
            case 7:
                // Run Scheduler (Priority)
                 scheduler_selection(SCHED_PRIORITY, 0, segs, MAX_SEGMENTS);
                break;
            case 8:
                printf("Running Deadlock Handling...\n");
                printf("1. Initialize Resources\n");
                printf("2. Set Process Max Claim (Required before Request)\n");
                printf("3. Make Resource Request\n");
                printf("4. Release Resources\n");
                printf("5. Show Resources\n");
                printf("6. Safety Check\n");
                printf("Enter your choice: ");
                int dl_choice;
                scanf("%d", &dl_choice);
                switch(dl_choice) {
                    case 1:
                        resource_init(MAX_RESOURCES, (int[]){10, 5, 7, 3, 8}); // Example initialization
                        num_resources = MAX_RESOURCES;
                        printf("Resources initialized to default values.\n");
                        break;
                    case 2:
                        printf("Enter PID: ");
                        int max_pid;
                        scanf("%d", &max_pid);
                        int max_vals[MAX_RESOURCES];
                        printf("Enter Max Claim for %d resources (space separated): ", num_resources);
                        for(int i = 0; i < num_resources; i++) scanf("%d", &max_vals[i]);
                        set_max_claim(max_pid, max_vals);
                        break;
                    case 3:
                        printf("Enter PID of process making request: ");
                        int req_pid;
                        scanf("%d", &req_pid);
                        int request[MAX_RESOURCES];
                        printf("Enter resource request for %d resources (space separated): ", num_resources);
                        for (int i = 0; i < num_resources; i++) {
                            scanf("%d", &request[i]);
                        }
                        request_resources(req_pid, request);
                        break;
                    case 4:
                        printf("Enter PID of process releasing resources: ");
                        int rel_pid;
                        scanf("%d", &rel_pid);
                        int release[MAX_RESOURCES];
                        printf("Enter resources to release for %d resources (space separated): ", num_resources);
                        for (int i = 0; i < num_resources; i++) {
                            scanf("%d", &release[i]);
                        }
                        release_resources(rel_pid, release);
                        break;
                    case 5:
                        show_resources();
                        break;
                    case 6:
                        // The safety_check function now checks the current global state
                        if (safety_check()) {
                            printf("The system is in a safe state after this request.\n");
                        } else {
                            printf("The system would be in an unsafe state after this request.\n");
                        }
                        break;
                    default:
                        printf("Invalid choice. Returning to main menu.\n");
                }

                break;
            case 9:
                printf("Exiting Mini OS...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    return 0;
}
