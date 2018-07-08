#include "tor_service.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include <time.h>
#endif

#include "tor_controller.h"
#include "tor_exe.h"

#include "tor_util.h"

int tor_service_init(char *tor_bin_dir)
{
    char *argv[] = {
        "",
        "--controlport",
        "9051",
        0
    };
    if(!tor_start(tor_bin_dir, argv))
    {
        printf("error starting tor\n");
        return 0;
    }

    while(!tor_is_running())
    {
#ifdef _WIN32
        Sleep(1000);
#else
        struct timespec _ts;                                
        _ts.tv_sec = 1000 / 1000;                   
        _ts.tv_nsec = (1000 % 1000) * 1000000;      
        nanosleep(&_ts, 0);
#endif
    }

    if(!tor_start_controller("9051", ""))
    {
        printf("error starting tor controller\n");
        tor_stop();
        return 0;
    }

    return 1;
}

int tor_service_add(int port, char *service_id)
{
    char res_buffer[CON_RES_SIZE];
    char command[64];
    memset(command, 0, 64);
    sprintf(command, "ADD_ONION NEW:BEST Flags=DiscardPK Port=%i\r\n", port);
    if(!tor_send_command(command, res_buffer))
    {
        printf("error adding service\n");
        return 0;
    }
    printf("response: %s\n", res_buffer);
    memcpy(service_id, res_buffer + 14, 16);

    return 1;
}

int tor_service_remove(char *service_id)
{
    /*
    if(strlen(service_id) != 16)
    {
        printf("invalid service id\n");
        return 0;
    }
    */

    char res_buffer[CON_RES_SIZE];
    char command[] = "DEL_ONION xxxxxxxxxxxxxxxx\r\n";
    memcpy(command + 10, service_id, 16);
    printf("command: %s\n", command);
    if(!tor_send_command(command, res_buffer))
    {
        printf("error removing service\n");
        return 0;
    }
    printf("response: %s\n", res_buffer);

    return 1;
}

int tor_service_delete()
{
    if(!tor_stop_controller())
    {
        printf("error stopping tor controller\n");
        return 0;
    }

    if(!tor_stop())
    {
        printf("error stopping tor\n");
        return 0;
    }

    return 1;
}

