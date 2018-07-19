//compile with -lpthread
//thread wrapper file
#ifndef THREAD_H
#define THREAD_H

//typedef (void (*)(void*)) thread_fun;

int thread_create(void (*fun)(void *arg), void *arg);

void thread_exit();

#endif


