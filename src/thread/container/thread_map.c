#include "thread_map.h"

#include <string.h>
#include <stdlib.h>

#define map_for_each    for(struct thread_map_node_t *entry = this->head; entry->next; entry = entry->next)

int thread_map_init(struct thread_map_t *this)
{
    mutex_init(this->list_mutex);

    this->size = 0;
    this->head = 0;

    return 1;
}

int thread_map_delete(struct thread_map_t *this)
{
    struct thread_map_node_t *f_entry = 0;

    mutex_lock(this->list_mutex);
    map_for_each
    {
        if(f_entry)
            free(f_entry); 

        f_entry = entry;
    }
	if(f_entry)
		free(f_entry);
	
    this->head = 0;
    this->size = 0;
    mutex_unlock(this->list_mutex);

    mutex_delete(this->list_mutex);
    
    return 1;
}

int thread_map_add(struct thread_map_t *this, char *name, void *value)
{
    struct thread_map_node_t *entry = malloc(sizeof(struct thread_map_node_t));
    entry->name = name;
    entry->value = value;
    entry->next = this->head;

    mutex_lock(this->list_mutex);
    this->head = entry;
    this->size++;
    mutex_unlock(this->list_mutex);

    return 1;
}

void *thread_map_remove(struct thread_map_t *this, char *name)
{
    struct thread_map_node_t *p_entry = 0;

    mutex_lock(this->list_mutex);
    map_for_each
    {
        if(!strcmp(entry->name, name))
        {
            void *res = entry->value;
            if(p_entry)
                p_entry->next = entry->next;
            else
                this->head = entry->next;

            this->size--;

            mutex_unlock(this->list_mutex);
            return res;
        }
		
		p_entry = entry;
    }
    mutex_unlock(this->list_mutex);

    return 0;
}

void *thread_map_get(struct thread_map_t *this, char *name)
{
    mutex_lock(this->list_mutex);
    map_for_each
    {
        if(!strcmp(entry->name, name))
        {
            void *res = entry->value;
            mutex_unlock(this->list_mutex);
            return res;
        }
    }
    mutex_unlock(this->list_mutex);

    return 0;
}

void *thread_map_for_each(struct thread_map_t *this, 
                            void *(*fun)(struct thread_map_t *this, void *element, char *name, void *ref), 
                            void *ref)
{
    mutex_lock(this->list_mutex);
    map_for_each
    {
        void *res = fun(this, entry->value, entry->name, ref);
        if(res)
        {
            mutex_unlock(this->list_mutex);
            return res;
        }
    }
    mutex_unlock(this->list_mutex);

    return 0;
}

