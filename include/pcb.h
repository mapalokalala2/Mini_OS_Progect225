#ifndef PCB_H
#define PCB_H

#include "os.h"

// Define helper functions for manipulating PCB entries

void pcb_init_table(void);
pcb *pcb_get_table(int pid);
int pcb_add(const char *name, int burst, int priority, int mem_size);
void pcb_print_all(void);

#endif 
