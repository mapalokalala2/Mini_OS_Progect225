//this is where deadlock handling will be done

// the aim is that before granting a resource request, the os ask: 
//if i give this out now, will i still be able to satify all other pending requests?
// if the answer is no, then the request is denied and the process must wait until resources are released by other processes. This way, the system can
// avoid entering an unsafe state that could lead to deadlock.

//note to self: a deadlock occurs when a set of processes are waiting for each other to release 
//resources, creating a cycle of dependencies that cannot be resolved. 
//The Banker's Algorithm is a resource allocation and deadlock avoidance algorithm that tests for safety by simulating 
//the allocation of predetermined maximum possible amounts of all resources, and then makes an "s-state" check to test for possible activities before deciding whether allocation should be allowed to continue.


#include <stdbool.h>
#include <string.h>
#include "../include/os.h"
#include "../include/logger.h"
#include "../include/banker.h"

bool banker_init(void) {
    // TODO: setup available / max / allocation matrices

    return true;
}

bool banker_request(int pid, int request_size) {
    // TODO: evaluate safety algorithm and either allocate or deny
    (void)pid; (void)request_size;
    return false;
}

void banker_release(int pid) {
    // TODO: release resources held by pid
    (void)pid;
}
