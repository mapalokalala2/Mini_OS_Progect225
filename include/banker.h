#ifndef BANKER_H
#define BANKER_H

#include "os.h"


void resource_init(int types, int total[]);
bool request_resources(int pid, int request[]);
void release_resources(int pid, int release[]); 
void show_resources(void);
int safety_check(int pid, int request[]);
int set_max_claim(int pid, int max[]);


#endif // BANKER_H
