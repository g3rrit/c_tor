#ifndef THREAD_MAP_H
#define THREAD_MAP_H

#include "mutex.h"

struct thread_map_node_t
{
    char *name;
    void *value;
    struct thread_map_node_t *next;
};

struct thread_map_t
{
    mutex_declare(list_mutex);

    struct thread_map_node_t *head;
    int size;
};

int thread_map_init(struct thread_map_t *this);

int thread_map_delete(struct thread_map_t *this);

int thread_map_add(struct thread_map_t *this, char *name, void *value);

void *thread_map_remove(struct thread_map_t *this, char *name);

void *thread_map_get(struct thread_map_t *this, char *name);

void *thread_map_for_each(struct thread_map_t *this, 
                            void *(*fun)(struct thread_map_t *this, void *element, char *name, void *ref), 
                            void *ref);


#endif