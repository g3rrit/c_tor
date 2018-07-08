#ifndef TOR_UTIL_H
#define TOR_UTIL_H

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
typedef SOCKET sockh;
#else
typedef int sockh;
#endif

int socket_init();

int socket_delete();

int socket_recv_all(sockh sock, char *data, int size);

int socket_send_all(sockh sock, char *data, int size);

#endif