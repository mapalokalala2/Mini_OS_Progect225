#include <stdbool.h>
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
