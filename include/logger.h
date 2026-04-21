#ifndef LOGGER_H
#define LOGGER_H

void log_init(void);
void log_event(const char *fmt, ...);
void log_dump(void);

#endif // LOGGER_H
