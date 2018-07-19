#include "thread.h"

#include "log.h"

#ifdef _WIN32
#else
#include "pthread.h"
#endif


int thread_create(void (*fun)(void *arg), void *arg)
{
#ifdef _WIN32 
    HANDLE handle;
    DWORD id;
#else
    pthread_t handle;
#endif

#ifdef _WIN32
    if(!(handle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)fun, arg, 0, &id)))
    {
        log_err("creating thread\n");
        return 0;
    }
    CloseHandle(handle);
#else
    if(pthread_create(&handle, 0, (void *(*)(void*))fun, arg))
    {
        log_err("creating thread\n");
        return 0;
    }
    pthread_detach(handle);
#endif
    return 1;
}

void thread_exit()
{
#ifdef _WIN32
    ExitThread(0);
#else
    pthread_exit(0);
#endif
}
