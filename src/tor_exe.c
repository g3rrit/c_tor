#include "tor_exe.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int tor_process_status;

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "winsock2.h"
#include "ws2tcpip.h"
#include "windows.h"
#define TOR_EXE_NAME "/tor_bin/win/tor.exe"
PROCESS_INFORMATION tor_info;
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define TOR_EXE_NAME "/tor_bin/mac/tor"
pid_t tor_pid;
#endif

int tor_start(char *tor_bin_dir, char **argv)
{
    if(tor_is_running())
    {
        printf("tor is already running\n");
        return 1;
    }
    
    char tor_bin_url[64];
    memset(tor_bin_url, 0, 64);

    int tor_dir_len = strlen(tor_bin_dir);
    strcpy(tor_bin_url, tor_bin_dir);
    strcpy(tor_bin_url + tor_dir_len, TOR_EXE_NAME);

    printf("executing tor exe: %s\n", tor_bin_url);

    for(int i = 0; argv[i] != 0; i++)
        printf("arg: %s\n", argv[i]);

#ifdef _WIN32
    char arg_win[64];
    memset(arg_win, 0, 64);
    char *cp_ptr = arg_win;
    for(int i = 0; argv[i] != 0; i++)
    {
        strcpy(cp_ptr, argv[i]);
        cp_ptr += strlen(argv[i]);
        *cp_ptr = ' ';
        cp_ptr++;
    }

    printf("win_arg: %s\n", arg_win);

    STARTUPINFO info={sizeof(info)};
    if(!CreateProcess(tor_bin_url, (LPSTR)arg_win, NULL, NULL, TRUE, 0, NULL, NULL, &info, &tor_info))
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
    if(!tor_is_running())
    {
        printf("tor is not running\n");
        return 1;
    }

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

