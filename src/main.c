#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tor_controller.h"
#include "tor_exe.h"
#include "tor_util.h"
#include "tor_service.h"

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

        /*
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
        */

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

    } while(strcmp(command, "quit"));

    socket_delete();
    
    return 0;
}