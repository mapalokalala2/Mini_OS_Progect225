#ifndef BANKER_H
#define BANKER_H
#define MAX_RESOURCES 5

#include "os.h"


void resource_init(int types, int total[]);
bool request_resources(int pid, int request[]);
void release_resources(int pid, int release[]); 
void show_resources(void); 
bool safety_check(void); // Changed signature
int set_max_claim(int pid, int max[]);


#endif
