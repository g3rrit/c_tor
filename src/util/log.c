#include "log.h"

#include <stdio.h>

void log_msg(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    printf(">");
    vprintf(fmt, args);

    va_end(args);
}

void log_err(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    printf("error>");
    vprintf(fmt, args);

    va_end(args);
}

