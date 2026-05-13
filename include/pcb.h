#ifndef PCB_H
#define PCB_H

#include "os.h"

// Define helper functions for manipulating PCB entries
//note to self: these functions are not strictly necessary as the core logic 
//could directly manipulate the process_table, but they provide a cleaner interface and encapsulation for PCB management, 
//which can be beneficial for maintainability and readability of the code. 
//They also allow for additional logic to be added in one place if needed (e.g., logging, validation) without having to modify the core scheduling or process management code.

void pcb_init_table(void);
pcb *pcb_get_table(int pid);
int pcb_add(const char *name, int burst, int priority, int mem_size);
void pcb_print_all(void);
void delete_process(int pid);
void delete_all_processes(void);
#endif 
