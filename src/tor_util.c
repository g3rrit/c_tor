#include "tor_util.h"

#include <stdio.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "winsock2.h"
#include "ws2tcpip.h"
#include "windows.h"
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

int _socket_init = 0;

int socket_init()
{
    if(_socket_init)
    {
        printf("socket already started\n");
        return 1;
    }
#ifdef _WIN32
	WSADATA wsaData;

	int ri = 0;
	if ((ri = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		printf("WSAStartup failed\n");
		WSACleanup();
		return 0;
	}
#else
#endif

    _socket_init = 1;
    return 1;
}

int socket_delete()
{
    if(!_socket_init)
    {
        printf("socket not started\n");
        return 1;
    }
#ifdef _WIN32
    WSACleanup();
#else
#endif

    _socket_init = 0;
    return 1;
}


int socket_recv_all(int sock, char *data, int size)
{
    if(sock < 0 || !data)
        return 0;

    int bytes = 0;
    int total = 0;
    while(total < size)
    {
        bytes = recv(sock, data + total, size - total, 0);
        if(bytes < 0)
            break;
        total += bytes;
    }
    return total;
}

int socket_send_all(int sock, char *data, int size)
{
    if(sock < 0 || !data)
        return 0;

    int bytes = 0;
    int total = 0;
    while(total < size)
    {
        bytes = send(sock, data + total, size - total, 0); 
        if(bytes < 0)
            break;
        total += bytes;
    }
    return total;
}
