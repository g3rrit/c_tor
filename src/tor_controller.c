#include "tor_controller.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include "Windows.h"
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
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
    if(GetExitCodeProcess(tor_info.hProcess, &exit_code) != STILL_ACTIVE)
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

int tor_send_command(char *command, char *res)
{

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

