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
#include <string.h>
#include "../include/os.h"
#include "../include/logger.h"
#include "../include/banker.h"


static int available[MAX_RESOURCES];
static int max_claim[MAX_PROCESSES][MAX_RESOURCES];
static int allocation[MAX_PROCESSES][MAX_RESOURCES];
static int need[MAX_PROCESSES][MAX_RESOURCES];
static int total_resources[MAX_RESOURCES];

static int find_process_index(int pid){
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if(get_process_table()[i].pid == pid) {
            return i;
        } 
    }
    return -1; // not found
}

void resource_init(int types, int total[]) {
    if (types > MAX_RESOURCES) {
        log_event("Warning: Attempted to initialize %d resources. Capped at %d.", types, MAX_RESOURCES);
        types = MAX_RESOURCES;
    }
    for (int i = 0; i < types; i++) { 
        available[i] = total[i];
        total_resources[i] = total[i];
    }
    // Ensure remaining resource slots are zeroed
    for (int i = types; i < MAX_RESOURCES; i++) {
        available[i] = 0;
        total_resources[i] = 0;
    }

    memset(max_claim, 0, sizeof(max_claim));
    memset(allocation, 0, sizeof(allocation));
    memset(need, 0, sizeof(need));
    log_event("Banker's Algorithm initialized with %d resource types.", types);
}

int set_max_claim(int pid, int max[]) {
    int index = find_process_index(pid);
    if (index == -1) {
        log_event("Error: Process %d not found when setting max claim.", pid);
        return -1;
    }
    for (int i = 0; i < MAX_RESOURCES; i++) {
        max_claim[index][i] = max[i];
        need[index][i] = max[i]; // initially, need is equal to max claim
    }
    log_event("Max claim set for process %d.", pid);
    return 0;
}

bool request_resources(int pid, int request[]) {
    int index = find_process_index(pid);
    if (index == -1) {
        log_event("Error: Process %d not found when requesting resources.", pid);
        return false;
    }
    // Check if request is less than need
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (request[i] > need[index][i]) {
            log_event("Process %d requested more than its declared maximum claim.", pid);
            return false;
        }
    }
    // Check if request is less than available
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (request[i] > available[i]) {
            log_event("Process %d requested resources that are not currently available.", pid);
            return false;
        }
    }
    // Simulate allocation
    for (int i = 0; i < MAX_RESOURCES; i++) {
        available[i] -= request[i];
        allocation[index][i] += request[i];
        need[index][i] -= request[i];
    }
    // Check for safety
    if (safety_check(pid, request)) {
        log_event("Resources allocated to process %d.", pid);
        return true;
    } else {
        // Rollback allocation
        for (int i = 0; i < MAX_RESOURCES; i++) {
            available[i] += request[i];
            allocation[index][i] -= request[i];
            need[index][i] += request[i];
        }
        log_event("Resource request by process %d denied to avoid unsafe state.", pid);
        return false;
    }
}

void release_resources(int pid, int release[]) {
    int index = find_process_index(pid);
    if (index == -1) {
        log_event("Error: Process %d not found when releasing resources.", pid);
        return;
    }
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (release[i] > allocation[index][i]) {
            log_event("Process %d attempted to release more resources than allocated.", pid);
            return;
        }
    }
    for (int i = 0; i < MAX_RESOURCES; i++) {
        available[i] += release[i];
        allocation[index][i] -= release[i];
        need[index][i] += release[i];
    }
    log_event("Process %d released resources.", pid);
}

void show_resources(void) {
    printf("\nAvailable Resources:\n");
    for (int i = 0; i < MAX_RESOURCES; i++) {
        printf("Resource %d: %d\n", i, available[i]);
    }
    printf("\nProcess Allocations and Needs:\n");
    pcb *table = get_process_table();
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (table[i].pid != 0) {
            printf("Process Index %d (PID %d):\n", i, table[i].pid);
            printf("  Allocation: ");
            for (int j = 0; j < MAX_RESOURCES; j++) printf("%d ", allocation[i][j]);
            printf("\n  Need:       ");
            for (int j = 0; j < MAX_RESOURCES; j++) printf("%d ", need[i][j]);
            printf("\n");
        }
    }
}

bool safety_check(int pid, int request[]) {
    pcb *table = get_process_table();
    int work[MAX_RESOURCES];
    bool finish[MAX_PROCESSES] = {false};

    for (int i = 0; i < MAX_RESOURCES; i++) {
        work[i] = available[i];
    }

    for (int i = 0; i < MAX_PROCESSES; i++) {
        // If slot is empty, treat as already finished
        finish[i] = (table[i].pid == 0);
    }

    while (true) {
        bool found = false;
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (!finish[i]) {
                bool can_finish = true;
                for (int j = 0; j < MAX_RESOURCES; j++) {
                    if (need[i][j] > work[j]) {
                        can_finish = false;
                        break;
                    }
                }
                if (can_finish) {
                    for (int j = 0; j < MAX_RESOURCES; j++) {
                        work[j] += allocation[i][j];
                    }
                    finish[i] = true;
                    found = true;
                }
            }
        }
        if (!found) {
            break;
        }
    }

    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!finish[i]) {
            return false; // Not safe
        }
    }
    return true; // Safe
 }
