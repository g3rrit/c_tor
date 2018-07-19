#ifndef SOCKS_H
#define SOCKS_H

#include "socket.h"

//returns socket on success
//  on failure returns 0
sock_t socks_connect(char *dest_ip, char *dest_port, char *ip, char *port, char *usrname, char *password);

//returns socket on success
//  on failure returns 0
sock_t tor_connect(char *ip, char *port);

#endif
