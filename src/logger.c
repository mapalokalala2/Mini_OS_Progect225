#include <stdio.h>
#include <stdarg.h>
#include "../include/logger.h"

void log_init(void) {
    // TODO: initialize log buffer or file output
}

void log_event(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    // TODO: append formatted log to buffer/file
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

void log_dump(void) {
    // TODO: output collected log events
}
