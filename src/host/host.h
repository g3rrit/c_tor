#ifndef HOST_H
#define HOST_H

#include "socket.h"

//todo add callback for address
int host_start(char *port, void (*client_cb)(void *env), void *env);

int host_stop(char *ip);

int host_delete();

#endif