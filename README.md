# CS225 Mini-OS Student Starter

This folder contains the skeleton and setup instructions for your Mini-OS project.

## 1) Project structure

Create these files and folders:

- `include/`
  - `os.h` (types and prototypes)
  - `pcb.h` (PCB data structure if split)
  - `scheduler.h`
  - `memory.h`
  - `banker.h`
  - `logger.h`

- `src/`
  - `main.c` (entry and simple CLI/driver)
  - `pcb.c`
  - `scheduler.c`
  - `memory.c`
  - `banker.c` (optional)
  - `logger.c`

- `Makefile`
- `README.md` (report summary, instructions, run examples)

## 2) Minimal skeleton code

`include/os.h`:
```c
#ifndef OS_H
#define OS_H

#include <stdbool.h>

#define MAX_PROCESSES 64
#define MAX_NAME_LEN 32
#define MEMORY_SIZE 1024

typedef enum { NEW, READY, RUNNING, WAITING, TERMINATED } proc_state_t;

typedef struct {
    int pid;
    char name[MAX_NAME_LEN];
    int priority;
    int burst_time;
    int remaining_time;
    int arrival_time;
    proc_state_t state;
    int mem_addr;
    int mem_size;
} pcb_t;

typedef enum { SCHED_FCFS, SCHED_ROUNDROBIN } sched_t;

void init_system(void);
int create_process(const char *name, int burst, int priority, int mem_size);
void list_processes(void);
int run_scheduler(sched_t stype, int time_quantum);
void show_memory(void);
void show_logs(void);

#endif // OS_H
```

`src/main.c`:
```c
#include "../include/os.h"
#include <stdio.h>

int main(void) {
    init_system();
    // TODO: create demo processes and call run_scheduler
    printf("Mini-OS skeleton running\n");
    return 0;
}
```

`src/scheduler.c`:
```c
#include "../include/os.h"

int run_scheduler(sched_t stype, int time_quantum) {
    (void)stype;
    (void)time_quantum;
    // TODO: implement FCFS/RR in this function
    return 0;
}
```

## 3) Build command

- `gcc -Iinclude src/*.c -o mini_os`

## 4) Notes for students

- Implement modules incrementally: data model -> FCFS -> RR -> memory + banking -> UI/logging.
- Keep functions small and test each part.
- Save README and report content in `student-report.md` or similar.
