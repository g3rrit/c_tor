#include "tor_service.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <time.h>

#include "tor_controller.h"
#include "tor_exe.h"
#include "log.h"
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
        log_err("starting tor\n");
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
        log_err("starting tor controller\n");
        tor_stop();
        return 0;
    }

    return 1;
}

int tor_service_add(char *port, char *service_id)
{
    char res_buffer[CON_RES_SIZE];
    char command[64];
    memset(command, 0, 64);
    sprintf(command, "ADD_ONION NEW:BEST Flags=DiscardPK Port=%s\r\n", port);
    if(!tor_send_command(command, res_buffer))
    {
        log_err("adding service\n");
        return 0;
    }
    log_msg("response: %s\n", res_buffer);
    memcpy(service_id, res_buffer + 14, 16);

    return 1;
}

int tor_service_remove(char *service_id)
{
    char res_buffer[CON_RES_SIZE];
    char command[] = "DEL_ONION xxxxxxxxxxxxxxxx\r\n";
    memcpy(command + 10, service_id, 16);
    log_msg("command: %s\n", command);
    if(!tor_send_command(command, res_buffer))
    {
        log_err("removing service\n");
        return 0;
    }
    log_msg("response: %s\n", res_buffer);

    return 1;
}

int tor_service_delete()
{
    if(!tor_stop_controller())
    {
        log_err("stopping tor controller\n");
        return 0;
    }

    if(!tor_stop())
    {
        log_err("stopping tor\n");
        return 0;
    }

    return 1;
}

