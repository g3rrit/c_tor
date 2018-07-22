#include "tor_exe.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "log.h"

int tor_process_status;

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <tchar.h>
#include <psapi.h>
#define TOR_EXE_NAME "/tor_bin/win/tor.exe"
PROCESS_INFORMATION tor_info;
HANDLE h_tor = 0;
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include <unistd.h>
#define TOR_EXE_NAME "/tor_bin/mac/tor"
pid_t tor_pid;
#endif

int tor_start(char *tor_bin_dir, char **argv)
{
    if(tor_is_running())
    {
        log_err("tor is already running\n");
        return 2;
    }
    
    char tor_bin_url[64];
    memset(tor_bin_url, 0, 64);

    int tor_dir_len = strlen(tor_bin_dir);
    strcpy(tor_bin_url, tor_bin_dir);
    strcpy(tor_bin_url + tor_dir_len, TOR_EXE_NAME);

    log_msg("executing tor exe: %s\n", tor_bin_url);

    for(int i = 0; argv[i] != 0; i++)
        log_msg("arg: %s\n", argv[i]);

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

    log_msg("win_arg: %s\n", arg_win);

    STARTUPINFO info={sizeof(info)};
    if(!CreateProcess(tor_bin_url, (LPSTR)arg_win, NULL, NULL, TRUE, 0, NULL, NULL, &info, &tor_info))
    {
        log_err("error creating tor process\n");
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
        return 1;
    }
    else if(tor_pid == 0)
    {
        //child - tor process

        execv(tor_bin_url, argv);

        log_err("failed to start tor process\n");
        return 0;
    }
    else
    {
        //error
        log_err("error forking process\n");
        return 0;
    }
#endif
}



int tor_is_running()
{
#ifdef _WIN32
    //get tor handle
    h_tor = 0;
    DWORD aprocesses[1024], cb_needed, cprocesses;

    if(!EnumProcesses(aprocesses, sizeof(aprocesses), &cb_needed))
    {
        log_err("error enumerating processes\n");
        return -1;
    }

    cprocesses = cb_needed/sizeof(DWORD);
    for(int i = 0; i < cprocesses; i++)
    {
        if(aprocesses[i] != 0)
        {
            TCHAR sz_process_name[MAX_PATH] = TEXT("<unknown>");

            HANDLE hprocess = OpenProcess(PROCESS_QUERY_INFORMATION |
                                            PROCESS_VM_READ,
                                            FALSE, aprocesses[i]);

            if(NULL != hprocess)
            {
                HMODULE hmod;
                DWORD cb_neededp; 

                if(EnumProcessModules(hprocess, &hmod, sizeof(hmod), &cb_neededp))
                {
                    GetModuleBaseName(hprocess, hmod, sz_process_name, sizeof(sz_process_name)/sizeof(TCHAR));
                }
            }

            if(!strcmp(sz_process_name, "tor.exe"))
            {
                log_msg("process found\n");
                h_tor = hprocess;
                break;
            }
        }
    }

    if(!h_tor)
    {
        log_err("handle for tor\n");
        return 0;
    }
    //

    DWORD exit_code;
    if(!GetExitCodeProcess(h_tor, &exit_code))
    {
        log_err("no tor process detected\n");
        return 0;
    }

    if(exit_code != STILL_ACTIVE)
    {
        log_err("exit code: %i\n", (int) exit_code);
        return 0;
    }

    return 1;
#else
    /*
    int status;
    int res;
    res = (int)waitpid(tor_pid, &status, WNOHANG);
    printf("status: %i\n", status);
    return !res;
    */
   	DIR* dir;
    struct dirent* ent;
    char buf[512];

    long  pid;
    char pname[100] = {0,};
    char state;
    FILE *fp=NULL; 

    if(!(dir = opendir("/proc"))) 
    {
        log_err("can't open /proc");
        return -1;
    }

    while((ent = readdir(dir)) != NULL) 
    {
        long lpid = atol(ent->d_name);
        if(lpid < 0)
            continue;
        snprintf(buf, sizeof(buf), "/proc/%ld/stat", lpid);
        fp = fopen(buf, "r");

        if(fp)
        {
            if((fscanf(fp, "%ld (%[^)]) %c", &pid, pname, &state)) != 3 )
            {
                log_err("fscanf failed \n");
                fclose(fp);
                closedir(dir);
                return -1; 
            }
            if(!strcmp(pname, "tor")) 
            {
                fclose(fp);
                closedir(dir);
                //return (pid_t)lpid;
                return 1;
            }
            fclose(fp);
        }
    }

    closedir(dir);

    return 0;
#endif
}

int tor_stop()
{
    if(!tor_is_running())
    {
        log_err("tor is not running\n");
        return 1;
    }

#ifdef _WIN32
    UINT exit_code = 0;
    if(!TerminateProcess(tor_info.hProcess, exit_code))
    {
        log_err("error killing tor\n");
        return 0;
    }

    return 1;
#else

    if(kill(tor_pid, SIGKILL) == -1)
    {
        log_err("error killing tor\n");
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

