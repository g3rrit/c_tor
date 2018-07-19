#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tor_controller.h"
#include "tor_exe.h"
#include "tor_util.h"
#include "tor_service.h"
#include "socket.h"

void test_tor_service(char *command)
{
    if(!strcmp(command, "start"))
    {
        if(!tor_service_init(".."))
            printf("error\n");
    }
    else if(!strcmp(command, "add"))
    {
        if(!tor_service_add(5555, service_id))
            printf("error\n");

        printf("id: %s\n", service_id);
    }
    else if(!strcmp(command, "remove"))
    {
        if(!tor_service_remove(service_id))
            printf("error\n");
    }
    else if(!strcmp(command, "delete"))
    {
        if(!tor_service_delete())
            printf("error\n");
    }
}

void test_tor_exe(char *command)
{
    if(!strcmp(command, "start"))
    {
        const char *cmd[] = {
            " ",
            "--controlport",
            "9051",
            0
        };
        tor_start("..", cmd);
    }
    else if(!strcmp(command, "running"))
    {
        if(tor_is_running())
            printf("running\n");
        else
            printf("killed\n");
    }
    else if(!strcmp(command, "kill"))
    {
        if(tor_stop())
            printf("killed tor process\n");
    }
    else if(!strcmp(command, "start_controller"))
    {
        if(tor_start_controller("9051", ""))
            printf("successfully started tor controller\n");
    }
    else if(!strcmp(command, "stop_controller"))
    {
        if(tor_stop_controller())
            printf("successfully stoped tor controller\n");
    }
}

struct local_env_t
{
    char *host_id;
};

void host_cb(char *host_id, struct local_env_t *local_env)
{
    local_env->host_id = host_id;
    printf("host_id set to: %s\n", local_env->host_id);
}

void client_cb(sock_t s, void *env)
{
    printf("client connected\n");

    char buffer[64];
    memset(buffer, 0, 64);

    socket_recv_all(s, buffer, 5);

    printf("buffer: %s\n", buffer);

    socket_close(s);
}

void test_tor_host(char *command)
{
    struct local_env_t local_env;

    if(!memcmp(command, "start_host", 10))
    {
        printf("starting host\n");
        host_start("4444", &host_cb, &local_env, &client_cb, 0);
    }
    else if(!memcmp(command, "connect", 7))
    {
        printf("connecting\n");
        char host_id[17];
        strcpy(host_id, command + 8);
        sock_t s = socket_connect(host_id, "4444");
        socket_send_all(s, "aaaaa", 5);
        socket_close(s);
    }
}

int main(int argc, char **argv)
{
#define COMMAND_LEN     256
    char command[COMMAND_LEN];

    socket_init();

    char service_id[16];
    do
    {
        memset(command, 0, COMMAND_LEN);

        fgets(command, COMMAND_LEN - 1, stdin);

        //remove controll tokens from command
        for(char *ptr = command; *ptr != 0; ptr++)
            if(*ptr == '\t' || *ptr == '\n' || *ptr == '\r')
                *ptr = 0;

        printf("command: %s\n", command);

        test_tor_host(command);

    } while(strcmp(command, "quit"));

    socket_delete();
    
    return 0;
}