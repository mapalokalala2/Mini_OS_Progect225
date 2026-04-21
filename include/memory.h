#ifndef MEMORY_H
#define MEMORY_H

#include "os.h"

void memory_init(void);
int memory_allocation(int pid, int size);
void memory_free(int pid);
void memory_display(void);

#endif
