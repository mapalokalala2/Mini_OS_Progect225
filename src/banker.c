//this is where deadlock handling will be done

// the aim is that before granting a resource request, the os ask: 
//if i give this out now, will i still be able to satify all other pending requests?
// if the answer is no, then the request is denied and the process must wait until resources are released by other processes. This way, the system can
// avoid entering an unsafe state that could lead to deadlock.

//note to self: a deadlock occurs when a set of processes are waiting for each other to release 
//resources, creating a cycle of dependencies that cannot be resolved. 
//The Banker's Algorithm is a resource allocation and deadlock avoidance algorithm that tests for safety by simulating 
//the allocation of predetermined maximum possible amounts of all resources, and then makes an "s-state" check to test for possible activities before deciding whether allocation should be allowed to continue.

//we will be using bankers algerithm, which is a reasorce allocation and deadlock avoidance strategy.it will woke 
//by simulating the allocation of resorceses and only granting a request if the resulting staed is safe.
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../include/os.h"
#include "../include/logger.h"
#include "../include/banker.h"

/* 1-Dimensional Banker's Algorithm Data Structures */
static int total_units = 0;
static int available_units = 0;
static int max_claim[MAX_PROCESSES];
static int allocation[MAX_PROCESSES];
static int need[MAX_PROCESSES];
static bool finished[MAX_PROCESSES];

static int find_process_index(int pid){
    if (pid <= 0) return -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if(get_process_table()[i].pid == pid) return i;
    }
    return -1;
}

void resource_init(int total, banker_output_callback output_func) {
    total_units = total;
    available_units = total;
    memset(max_claim, 0, sizeof(max_claim));
    memset(allocation, 0, sizeof(allocation));
    memset(need, 0, sizeof(need));
    memset(finished, 0, sizeof(finished));
    log_event("Banker's Algorithm initialized with %d total units (1D Model).", total); // Keep system log
    if (output_func) output_func("Banker's Algorithm initialized with %d total units (1D Model).", total); // Output to GUI/CLI
}

int set_process_max_claim(int pid, int max) {
    int index = find_process_index(pid);
    if (index == -1) return -1;

    if (max > total_units) {
        log_event("Error: Max claim %d for PID %d exceeds total system units %d.", max, pid, total_units);
        return -1;
    }

    max_claim[index] = max;
    allocation[index] = 0;
    need[index] = max;
    finished[index] = false;

    log_event("Max claim set for PID %d: %d. (Used=0, Need=%d)", pid, max, max);
    return 0;
}

/* Helper to perform a standard Banker's safety check on current state */
static bool is_state_safe(int work, const int current_alloc[], const int current_need[], int n_procs, const pcb* table) {
    bool finish_check[MAX_PROCESSES] = {false};
    int count = 0;
    int active_indices[MAX_PROCESSES];
    int active_count = 0;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (table[i].pid != 0 && table[i].state != TERMINATED) {
            active_indices[active_count++] = i;
        }
    }

    while (count < active_count) {
        bool found = false;
        for (int i = 0; i < active_count; i++) {
            int idx = active_indices[i];
            if (!finish_check[idx] && current_need[idx] <= work) {
                work += current_alloc[idx];
                finish_check[idx] = true;
                found = true;
                count++;
            }
        }
        if (!found) return false;
    }
    return true;
}

bool run_banker_simulation(banker_output_callback output_func) {
    pcb *table = get_process_table();
    int active_count = 0;
    int active_indices[MAX_PROCESSES];
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (table[i].pid != 0 && table[i].state != TERMINATED) {
            active_indices[active_count++] = i;
        }
        finished[i] = false; // Reset finished status for all processes
        allocation[i] = 0; // Reset allocation
    }
    available_units = total_units;

    
    if (output_func) output_func("--- Initial Allocation (2 units per process) ---");
    for (int k = 0; k < active_count; k++) {
        int idx = active_indices[k];
        int initial_grant = (max_claim[idx] < 2) ? max_claim[idx] : 2;
        if (available_units >= initial_grant) {
            allocation[idx] = initial_grant;
            available_units -= initial_grant;
            if (output_func) output_func("PID %d: Initially allocated %d units.", table[idx].pid, initial_grant);
        }
    }
    for (int i = 0; i < MAX_PROCESSES; i++) {
        need[i] = max_claim[i] - allocation[i];
    }

    if (active_count == 0) {
        printf("No active processes to simulate.\n");
        return true;
    }

    if (output_func) output_func("\n--- Automated Banker's Lifecycle Simulation ---");
    int completed_procs = 0;
    int stuck_iterations = 0;

    while (completed_procs < active_count) {
        if (output_func) output_func("\n[Current Table] Available Units: %d", available_units);
        if (output_func) output_func("%-10s | %-5s | %-5s | %-5s | %-8s", "Process", "Used", "Max", "Need", "Status");
        for (int k = 0; k < active_count; k++) {
            int idx = active_indices[k];
            if (output_func) output_func("PID %-6d | %-5d | %-5d | %-5d | %-8s",
                                         table[idx].pid, allocation[idx], max_claim[idx], need[idx],
                                         finished[idx] ? "FINISHED" : "ACTIVE");
        }

        bool progress_made = false;
        
        // Selection logic: Look for the process with the smallest non-zero need first
        // it will iterate through the active processes and try to grant resources to the one with the smallest need, as this is often a good heuristic for making progress in the simulation.
        
        while (true) {
            int idx = -1;
            int min_need = 1000000;

            for (int k = 0; k < active_count; k++) {
                int i = active_indices[k];
                if (!finished[i] && need[i] > 0) {
                    if (need[i] < min_need) {
                        min_need = need[i];
                        idx = i;
                    }
                }
            }

            if (idx == -1) break; // No more active processes with needs
            
            // Banker thinking: "Can I give 1 unit to this process?"
            if (available_units > 0 && need[idx] > 0) { // Only try to allocate if units are available and needed
                if (output_func) output_func("Banker: Checking if granting 1 unit to PID %d is safe...", table[idx].pid);
                
                // Hypothetical allocation
                available_units--;
                allocation[idx]++;
                need[idx]--;

                if (is_state_safe(available_units, allocation, need, active_count, table)) { // active_count is not used in is_state_safe
                    if (output_func) output_func("Banker: [SAFE] Request granted to PID %d.", table[idx].pid);
                    progress_made = true;
                    
                    if (need[idx] == 0) { // Process has finished its need
                        if (output_func) output_func("Banker: PID %d has met its Max Claim and is running...", table[idx].pid);
                        if (output_func) output_func("Banker: PID %d finished! Releasing %d units back to system.", table[idx].pid, allocation[idx]);
                        available_units += allocation[idx];
                        allocation[idx] = 0;
                        finished[idx] = true;
                        completed_procs++;
                    }
                    break; // Move to next simulation step
                } else {
                    // Rollback hypothetical allocation
                    available_units++;
                    allocation[idx]--;
                    need[idx]++;
                    if (output_func) output_func("Banker: [UNSAFE] Request from PID %d denied to avoid potential deadlock.", table[idx].pid);
                    
                    // Since the smallest need failed, we break to avoid an infinite loop in the 'while'
                    // and let the progress check handle it.
                    break; 
                }
            } else if (need[idx] == 0 && !finished[idx]) {
                // Handle processes that might have finished during initial allocation
                available_units += allocation[idx];
                allocation[idx] = 0;
                finished[idx] = true;
                completed_procs++;
                progress_made = true;
                break;
            } else {
                break;
            }
        }

        if (!progress_made) {
            stuck_iterations++;
            if (stuck_iterations > active_count) { // If we've iterated through all active processes without making progress
                if (output_func) output_func("\n!! UNSAFE STATE DETECTED !!");
                if (output_func) output_func("Reason: No process can be granted resources without potentially causing a deadlock.");
                if (output_func) output_func("Available units (%d) cannot satisfy any remaining process Needs.", available_units);
                log_event("Simulation ended: UNSAFE state / Deadlock.");
                return false;
            }
        } else {
            stuck_iterations = 0;
        }
    }

    if (output_func) output_func("\n>> SUCCESS: All processes finished. System returned to initial available units: %d", available_units);
    log_event("Simulation ended: All processes finished successfully.");
    return true;
}
