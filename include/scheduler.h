#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "os.h"

int scheduler_selection(sched sched_type, int time_quantum, segment *segs , int max_segments);

#endif 
