#ifndef LOGGER_H
#define LOGGER_H

void log_init(void);
void log_event(const char *fmt, ...);
void show_log(void);
void log_dump(void);// thisgit  is a cleaning function for safty
#endif
