#ifndef BANKER_H
#define BANKER_H

#include "os.h"

bool banker_init(void);
bool banker_request(int pid, int request_size);
void banker_release(int pid);

#endif // BANKER_H
