#include "tor_controller.h"

#include "tor_exe.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
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
HANDLE con_socket = 0;
#else
int con_socket = 0;
#endif

int tor_start_controller(char *port, char *auth)
{
    if(!tor_is_running())
    {
        printf("tor needs to be running\n");
        return 0;
    }

    if(con_socket)
    {
        printf("controll socket already in use\n");
        return 0;
    }

    int rc = 0;
    struct addrinfo hints;
    struct addrinfo *res = 0;
    struct addrinfo *ptr = 0;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    //hints.ai_flags = AI_PASSIVE;

    rc = getaddrinfo(0, port, &hints, &res);
    if(rc != 0)
    {
        printf("error getting address info\n");
        return 0;
    }

    ptr = res;

    con_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if(con_socket < 0)
    {
        printf("error opening socket\n");
        return 0;
    }   

    rc = connect(con_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if(rc < 0)
    {
        printf("error connecting to controlport\n");
        tor_stop_controller();
        return 0;
    }

    freeaddrinfo(res);
    /*
    con_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(con_socket < 0)
    {
        printf("error creating controll socket\n");
        return 0;
    }

    unsigned long addr;
    struct sockaddr_in addr_in;
    memset(&addr_in, 0, sizeof(struct sockaddr_in));

    addr = inet_addr("127.0.0.1");
    memcpy(&addr_in.sin_addr, &addr, sizeof(addr));
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons(port);

    if(connect(con_socket, (struct sockaddr*)&addr_in, sizeof(addr_in)) < 0)
    {
        printf("error connecting to controll port\n");
        tor_stop_controller();
        return 0;
    }
    */

    char auth_cmd[64];
    sprintf(auth_cmd, "authenticate %s\r\n", auth);
    int auth_cmd_len = strlen(auth_cmd);

    if(send(con_socket, auth_cmd, auth_cmd_len, 0) != auth_cmd_len)
    {
        printf("error sending authentication command\n");
        tor_stop_controller();
        return 0;
    }

    if(recv(con_socket, auth_cmd, auth_cmd_len, 0) < 3)
    {
        printf("error receiving response\n");
        tor_stop_controller();
        return 0;
    }

    char ret_code[3];
    memcpy(ret_code, auth_cmd, 3);
    int ret_val = atoi(ret_code);

    if(ret_val != 250)
    {
        printf("authentification error\n| %s\n", auth_cmd);
        return 0;
    }

    return 1;
}

int tor_stop_controller()
{
    if(!con_socket)
    {
        printf("no controll socket in use\n");
        return 0;
    }
#ifdef _WIN32
    closesocket(con_socket);
    CloseHandle(con_socket);
#else
    close(con_socket);
#endif
    con_socket = 0;
    return 1;
}

int tor_send_command(char *command, char *res)
{

}