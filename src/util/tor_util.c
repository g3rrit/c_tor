#include "tor_util.h"

#include <time.h>
#include <stdint.h>

void sleep_ms(int milliseconds)
{
#ifdef _WIN32
    Sleep(milliseconds);
#else
    struct timespec _ts;                                
    _ts.tv_sec = milliseconds / 1000;                   
    _ts.tv_nsec = (milliseconds % 1000) * 1000000;      
    nanosleep(&_ts, 0);
#endif
}

double get_delta_time()
{
    static uint64_t time_l = 0;
    static uint64_t time_n = 0;

    time_l = time_n;
    time_n = clock();

    return ((float)(time_n - time_l))/(float)CLOCKS_PER_SEC;
}

