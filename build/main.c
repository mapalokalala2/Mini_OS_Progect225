/**
 * main.c - Command Line Interface (CLI) for the Mini OS.
 * Provides a menu-driven interface to manage processes, memory, 
 * scheduling, and deadlock prevention.
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "../include/os.h"
#include "../include/pcb.h"
#include "../include/scheduler.h"
#include "../include/memory.h"
#include "../include/banker.h"
#include "../include/logger.h"

// Callback for CLI version of Banker's Algorithm output
static void cli_banker_output_callback(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("\n"); 
    va_end(args);
}
static segment segs [MAX_SEGMENTS];

// Helper to pause execution so the user can read output before clearing the screen
void continue_prompt(void) {
    while (getchar() != '\n'); // Clear the input buffer
    printf("Press Enter to continue...");
    while (getchar() != '\n');
    printf("\033[H\033[J"); //this will clear the screen after the user presses enter...
}

// Optional screen clearing utility
void clear_screen(void) {
    printf("would you like to clear the screen? (y/n): ");
    char clear_choice;
    scanf(" %c", &clear_choice);
    if (clear_choice == 'y' || clear_choice == 'Y') {
        printf("\033[H\033[J");
    }
}

void show_memory(void) {
    memory_display();
}

void show_logs(void) {
    show_log();
}

// Displays the main navigation menu
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
    // Core module initialization
    log_init(); 
    init_system();
    memory_init();
    

    log_event("Mini OS initialized successfully.");
    printf("Mini OS initialized successfully.\n");
    int choice = 1, dl_choice = 1;
    int burst, priority, mem, pid, time_quantum;
    char name[MAX_NAME_LEN];
    sched sched_type;

    // Main execution loop
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
                // Process Creation Logic
                clear_screen();
                printf("YOU HAVE SELECTED TO CREATE AN EMERGENCY PROCESS\n");
                printf("Enter process name: ");
                // this will Clear the newline character left in the buffer by the previous scanf
                while (getchar() != '\n'); 
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
                continue_prompt();
                
                break;
            case 2:
                // List and potentially delete processes
                clear_screen();
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
                continue_prompt();

                break;
            case 3:
                // Show current memory allocation map
                clear_screen();
                show_memory();
                continue_prompt();
                break;
            case 4:
                // Display system event logs
                clear_screen();
                show_logs();
                continue_prompt();
                break;
            case 5:
                // Round Robin Scheduling
                clear_screen();
                // Run Scheduler (Round Robin)
                printf("Enter time quantum: ");
                scanf("%d", &time_quantum);
                scheduler_selection(SCHED_ROUNDROBIN, time_quantum, segs, MAX_SEGMENTS);
                continue_prompt();
                break;
            case 6:
                // FCFS Scheduling
                clear_screen(); 
                // Run Scheduler (FCFS)
                scheduler_selection(SCHED_FCFS, 0, segs, MAX_SEGMENTS);
                continue_prompt();
                break;
            case 7:
                // Priority-based Scheduling
                clear_screen();
                // Run Scheduler (Priority)
                 scheduler_selection(SCHED_PRIORITY, 0, segs, MAX_SEGMENTS);
                continue_prompt();
                break;
            case 8: 
                // Banker's Algorithm Sub-menu for Deadlock management
                dl_choice = 1; // Reset to allow re-entry into the sub-menu
                while(dl_choice != 4) {
                    clear_screen();
                    printf("Bankers Algorithm (Deadlock Avoidance)\n");
                    printf("1. Initialize\n");
                    printf("2. Set Max Claim\n");
                    printf("3. Run Banker's Simulation\n");
                    printf("4. Back\n");
                    printf("Enter your choice: ");
                    scanf("%d", &dl_choice);
                    switch(dl_choice) {
                        // Logic for Resource init, Max claim settings, and Safety check
                        case 1:
                            printf("Enter total units for single resource: ");
                            int total;
                            scanf("%d", &total);
                            resource_init(total, cli_banker_output_callback);
                            continue_prompt();
                            break;
                        case 2:
                            int pid_v, max_v; // Declare variables here
                            printf("Enter PID: ");
                            scanf("%d", &pid_v);
                            printf("Enter Max Claim: "); scanf("%d", &max_v);
                            set_process_max_claim(pid_v, max_v);
                            continue_prompt();
                            break;
                        case 3:
                            if (run_banker_simulation(cli_banker_output_callback)) {
                                printf("System is SAFE.\n");
                            } else {
                                printf("System is UNSAFE.\n");
                            }
                            continue_prompt();
                            break;
                        case 4:
                            printf("Returning to main menu...\n");
                            break;
                        default:
                            printf("Invalid choice. Returning to main menu.\n");
                    }
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
