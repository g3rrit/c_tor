#include "tor_controller.h"

#include "tor_exe.h"
#include "tor_util.h"
#include "log.h"
#include "socket.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

sock_t con_socket = 0;

int get_res_code(char *res)
{
    if(strlen(res) < 3)
    {
        log_err("invalid res string\n");
        return 0;
    }

    char res_str[4];
    res_str[3] = 0;
    memcpy(res_str, res, 3);
    int res_code = atoi(res_str);

    return res_code;
}

int tor_start_controller(char *port, char *auth)
{
    if(!tor_is_running())
    {
        log_err("tor needs to be running\n");
        return 0;
    }

    if(con_socket)
    {
        log_err("control socket already in use\n");
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
        log_err("getting address info\n");
        return 0;
    }

    ptr = res;

    con_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if(con_socket < 0)
    {
        log_err("opening socket\n");
        return 0;
    }   

    rc = connect(con_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if(rc < 0)
    {
        log_err("connecting to controlport\n");
        tor_stop_controller();
        return 0;
    }

    freeaddrinfo(res);

    char auth_cmd[64];
    sprintf(auth_cmd, "authenticate %s\r\n", auth);
    int auth_cmd_len = strlen(auth_cmd);

    if(send(con_socket, auth_cmd, auth_cmd_len, 0) != auth_cmd_len)
    {
        log_err("sending authentication command\n");
        tor_stop_controller();
        return 0;
    }

    if(recv(con_socket, auth_cmd, auth_cmd_len, 0) < 3)
    {
        log_err("receiving response\n");
        tor_stop_controller();
        return 0;
    }

    /*
    char ret_code[4];
    memcpy(ret_code, auth_cmd, 3);
    ret_code[3] = 0;
    int ret_val = atoi(ret_code);
    */
    int ret_val = get_res_code(auth_cmd);

    if(ret_val != 250)
    {
        log_err("authentification error\n| %i | %s\n", ret_val, auth_cmd);
        return 0;
    }

    return 1;
}

int tor_stop_controller()
{
    if(!con_socket)
    {
        log_err("no control socket in use\n");
        return 0;
    }
#ifdef _WIN32
    closesocket(con_socket);
#else
    close(con_socket);
#endif
    con_socket = 0;
    return 1;
}

int tor_send_command(char *command, char *res)
{
    if(!con_socket)
    {
        log_err("no control socket active\n");
        return 0;
    }

    int cmd_len = strlen(command);
    if(!socket_send_all(con_socket, command, cmd_len))
    {
        log_err("error sending complete command\n");
        return 0;
    }

    if(recv(con_socket, res, CON_RES_SIZE, 0) <= 0)
    {
        log_err("error receiving response\n");
        return 0;
    }

    int res_code = get_res_code(res);
    if(res_code != 250)
    {
        log_err("error: %i | %s\n", res_code, res);
        return 0;
    }

    return 1;
}