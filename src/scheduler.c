#include <stdio.h>
#include <stdbool.h>

#include "../include/logger.h"
#include "../include/scheduler.h"
#include "../include/os.h"

static int process_complete[MAX_PROCESSES]; // static because its valuse must persist in memory. stores when each process finishes executing.


int scheduler_selection(sched sched_type, int time_quantum, segment *segs , int max_segments) {
    //notes:
    //segment *segs will recode the timeline blocks which will be used to draw the gant chatt

    pcb *tbl = get_process_table();
    int active = get_process_count(), time = 0, segment_n = 0, total_burst_time = 0;
    int n = MAX_PROCESSES;
    

    // this loop will reset the environment before running the schedule

    for(int i = 0; i < n; i++){
        if ( tbl[i].pid <= 0){
            continue;
        } 
        tbl[i].remaining_time = tbl[i].burst_time;// gives every process its full time back
        tbl[i].state =  READY;
        total_burst_time += tbl[i].burst_time;
        process_complete[i] = 0;
    }
    
// ========================================================
//                       FCFS
// ========================================================

    if( sched_type == SCHED_FCFS){
        for(int i = 0; i < n; i++){
            if (tbl[i].pid <= 0){
                continue;
            }

            pcb *pro = &tbl[i];// this peace of code creats a pointer pro to the current process being accessed
            pro -> state = RUNNING;

            if(segment_n < MAX_SEGMENTS){
                segs[segment_n].pid = pro ->pid;
                segs[segment_n].start_time = time;
                segs[segment_n].duration = pro ->burst_time;
                segment_n++;
            }

// come back later
            time += pro ->burst_time;
            pro ->remaining_time = 0;
            pro ->state = TERMINATED;
    process_complete[i] = time;


        }
    

// ========================================================
//                   ROUND ROBIN
// ========================================================
// using circular scan implementation
    }else if(sched_type == SCHED_ROUNDROBIN){
        int finished_count = 0;
        int i = 0;// this will be the current index of the table;
        
        while(finished_count < active){
            pcb *pro = &tbl[i];
            
            if (pro ->pid > 0 && pro ->remaining_time > 0){
                //calculation of how much time to give
                int use = (pro ->remaining_time < time_quantum) ? pro ->remaining_time : time_quantum;
                pro ->state = RUNNING;
                
                if(segment_n < MAX_SEGMENTS){
                    segs[segment_n].pid = pro ->pid;
                    segs[segment_n].start_time = time;
                    segs[segment_n].duration = use;
                    segment_n++;
                }
                //simulation of execution
                pro->remaining_time -= use;
                time += use;

                if( pro ->remaining_time == 0){
                    pro->state = TERMINATED;
                    finished_count++;
                    process_complete[i] = time;
                }else{
                    pro ->state = READY;
                }
            }
            // this is called the circular trick:
            // increment i, but wrap back to 0 when it reaches n, this way we can keep scanning the process table in a circular manner until all processes are finished.
            i = (i + 1) % n;
        }
    }
// ========================================================
//                   PRIORITY
// ========================================================
    else if(sched_type == SCHED_PRIORITY){
        int finished_count = 0;
        while(finished_count < active){
            int highest_priority_index = -1;
            for(int j = 0; j < n; j++){
                if(tbl[j].pid > 0 && tbl[j].remaining_time > 0){
                    if(highest_priority_index == -1 || tbl[j].priority < tbl[highest_priority_index].priority) {
                        highest_priority_index = j;
                    } else if (tbl[j].priority == tbl[highest_priority_index].priority) {
                        // Tie-breaker: use arrival time just incase two processes have the same priority, the one that arrived first gets scheduled first.
                        if (tbl[j].arrival_time < tbl[highest_priority_index].arrival_time) {
                            highest_priority_index = j;
                        }
                    }
                }
            }
            if(highest_priority_index != -1){
                pcb *pro = &tbl[highest_priority_index];
                pro ->state = RUNNING;

                if(segment_n < MAX_SEGMENTS){
                    segs[segment_n].pid = pro ->pid;
                    segs[segment_n].start_time = time;
                    segs[segment_n].duration = pro ->burst_time;
                    segment_n++;
                }

                time += pro ->burst_time;
                pro ->remaining_time = 0;
                pro ->state = TERMINATED;
                process_complete[highest_priority_index] = time;
                finished_count++;
            }
        }
    }

    int total_tat = 0, total_wt = 0, stats_active = 0;
    for(int i = 0; i < n; i++){
        if (tbl[i].pid <= 0){
            continue;
        }
        // Formula: Turnaround Time = Completion Time - Arrival Time
        int tat = process_complete[i] - tbl[i].arrival_time;
        
        // Formula: Waiting Time = Turnaround Time - Burst Time
        int wt = tat - tbl[i].burst_time;
        total_tat += tat;
        total_wt += wt;
        stats_active++;

        printf("Process %d: TAT = %d, WT = %d\n", tbl[i].pid, tat, wt);
        
    }
    if (stats_active > 0){
    printf("Average TAT = %f\n", (float)total_tat / stats_active);
    printf("Average WT = %f\n", (float)total_wt / stats_active);
    }

    double utilization = (double)total_burst_time / (time > 0 ? time : 1) * 100.0; // Denominator should not be 0
    printf("CPU Utilization = %.2f%%\n", utilization);

    log_event("scheduler done. Time = %d utilisation = %.2f%%", time, utilization); // Corrected log_event arguments

    return segment_n;
}
