#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

void log_msg(char *fmt, ...);

void log_err(char *fmt, ...);

#endif