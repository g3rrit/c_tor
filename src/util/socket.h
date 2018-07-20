#ifndef SOCKET_H
#define SOCKET_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT  0x501 
#include "winsock2.h"
#include "ws2tcpip.h"
#include "windows.h"
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

#ifdef _WIN32
typedef SOCKET sock_t;
#else
typedef int sock_t;
#endif

int socket_init();

int socket_delete();

int socket_recv_all(sock_t sock, char *data, int size);

int socket_send_all(sock_t sock, char *data, int size);

sock_t socket_connect(char *id, char *port);

int socket_close(sock_t s);

#endif