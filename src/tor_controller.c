#include "tor_controller.h"

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

int tor_controller_socket;

int tor_process_status;

#ifdef _WIN32
#define TOR_EXE_NAME "/tor_bin/win/tor.exe"
PROCESS_INFORMATION tor_info;
#else
#define TOR_EXE_NAME "/tor_bin/mac/tor"
pid_t tor_pid;
#endif


int tor_start(char *tor_bin_dir, char **argv)
{
    char tor_bin_url[64];
    memset(tor_bin_url, 0, 64);

    int tor_dir_len = strlen(tor_bin_dir);
    strcpy(tor_bin_url, tor_bin_dir);
    strcpy(tor_bin_url + tor_dir_len, TOR_EXE_NAME);

    printf("executing tor exe: %s\n", tor_bin_url);

#ifdef _WIN32
    STARTUPINFO info={sizeof(info)};
    if(!CreateProcess(tor_bin_url, (LPSTR)argv, NULL, NULL, TRUE, 0, NULL, NULL, &info, &tor_info))
    {
        printf("error creating tor process\n");
        return 0;
        /*
        WaitForSingleObject(processInfo.hProcess, INFINITE);
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        */
    }

    return 1;
#else
    tor_pid = fork();
    if(tor_pid > 0)
    {
        //parent
        printf("in parent\n");
        return 1;
    }
    else if(tor_pid == 0)
    {
        //child - tor process

        execv(tor_bin_url, argv);

        printf("failed to start tor process\n");
        return 0;
    }
    else
    {
        //error
        printf("error forking process\n");
        return 0;
    }
#endif
}

int tor_is_running()
{
#ifdef _WIN32
    DWORD exit_code;
    if(!GetExitCodeProcess(tor_info.hProcess, &exit_code))
    {
        printf("no tor process detected\n");
        return 0;
    }

    if(exit_code != STILL_ACTIVE)
    {
        printf("exit code: %i\n", (int) exit_code);
        return 0;
    }

    return 1;
#else
    int status;
    int res;
    res = (int)waitpid(tor_pid, &status, WNOHANG);
    printf("status: %i\n", status);

    return !res;
#endif
}

int tor_stop()
{
#ifdef _WIN32
    UINT exit_code = 0;
    if(!TerminateProcess(tor_info.hProcess, exit_code))
    {
        printf("error killing tor | might be dead already\n");
        return 0;
    }

    return 1;
#else

    if(kill(tor_pid, SIGKILL) == -1)
    {
        printf("error killing tor | might be dead already\n");
        return 0;
    }
    
    /*
    pid_t tpid;
    do {
       tpid = wait(&tor_process_status);
       if(tpid != pid) printf("process terminated: %i\n", tpid);
    } while(tpid != pid);
    printf("tor process status: %i\n", tor_process_status);
    */

    return 1;
#endif
}

#ifdef _WIN32
HANDLE con_socket = 0;
#else
int con_socket = 0;
#endif

int tor_start_controller(int port, char *auth)
{
#ifdef _WIN32
#else
#endif
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

    auth_cmd[3] = 0;
    int ret_val = atoi(auth_cmd);
    auth_cmd[3] = ' ';

    if(ret_val != 100)
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