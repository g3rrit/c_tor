#ifndef HOST_H
#define HOST_H

#include "socket.h"

//todo add callback for address
int host_start(char *port, 
                void (*host_started)(char *host_id, void *env), void *host_env,
                void (*client_cb)(sock_t s, void *env), void *client_env,
                int stop_on_connection);

int host_stop(char *id);

int host_delete();

#endif