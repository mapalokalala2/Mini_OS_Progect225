#ifndef BANKER_H
#define BANKER_H

#include "os.h"

/* 1-Dimensional Banker's Algorithm Interface */
typedef void (*banker_output_callback)(const char *format, ...);
void resource_init(int total, banker_output_callback output_func);
int set_process_max_claim(int pid, int max);
bool run_banker_simulation(banker_output_callback output_func);

#endif
