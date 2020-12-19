
#ifndef LOG_H
#define LOG_H

void log_info(const char *function, const char *fmt, ...);
void log_warning(const char *function, const char *fmt, ...);
void log_error(const char *function, const char *fmt, ...);
void log_critical_error(const char *function, const char *fmt, ...);

#endif
