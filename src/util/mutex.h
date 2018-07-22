#ifndef MUTEX_H
#define MUTEX_H

#include "log.h"

//-----WINDOWS-----
#ifdef _WIN32

#define mutex_declare(name)         HANDLE name

#define mutex_init(name)            name = CreateMutex(0, FALSE, 0);     \
                                    if(!name)                            \
                                        log_err("creating mutex\n");

#define mutex_delete(name)          CloseHandle(name);


#define mutex_lock(name)            WaitForSingleObject(name, INFINITE);

#define mutex_unlock(name)          ReleaseMutex(name);


#else
//-----UNIX--------
#include <pthread.h>

#define mutex_declare(name)         pthread_mutex_t name

#define mutex_init(name)            if(pthread_mutex_init(&name, 0))     \
                                        log_err("creating mutex\n");

#define mutex_delete(name)          if(pthread_mutex_destroy(&name))     \
                                        log_err("deleting mutex\n");

#define mutex_lock(name)            pthread_mutex_lock(&name);

#define mutex_unlock(name)          pthread_mutex_unlock(&name);

#endif


#endif
