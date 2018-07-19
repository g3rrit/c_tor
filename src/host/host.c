#include "host.h"

#include <stdlib.h>
#include <string.h>

#include "thread_map.h"
#include "log.h"
#include "tor_util.h"
#include "socket.h"
#include "tor_service.h"
#include "thread.h"

#define DELTA_WAIT_FOR_HOST_TIME 1500   //in ms
#define MAX_WAIT_FOR_HOST_TIME 5000     //in ms

struct thread_map_t _host_map;
volatile int _host_map_started = 0;

struct client_env_t
{
    void (*client_cb)(sock_t s, void *env);
    void *env;
    sock_t s;
};

struct host_t
{
    int is_running;
    int is_stopped;
    int has_stopped;
};

#define host_map_add(ip, host)      thread_map_add(&_host_map, ip, host)
#define host_map_remove(ip)         thread_map_remove(&_host_map, ip)
#define host_map_get(ip)            thread_map_get(&_host_map, ip)

void _client_cb(struct client_env_t *c_env)
{
    c_env->client_cb(c_env->s, c_env->env);
    free(env);
}

int host_start(char *port, 
                void (*host_started)(char *host_id, void *env), void *host_env,
                void (*client_cb)(sock_t s, void *env), void *client_env);
{
    if(!_host_map_started)
        thread_map_init(&_host_map);

    int ret_val = 0;

    //start tor hidden service
    char host_id[17];
    if(!tor_service_add(port, host_id))
    {
        log_err("starting tor service\n");
        return 0;
    }
    //---

    //init host
    struct addrinfo hints;
    struct addrinfo *ptr = 0;
    struct addrinfo *res = 0;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    log_msg("getaddrinfo for port: %s\n", port);

    int rc = getaddrinfo("127.0.0.1", port, &hints, &res);
    if(rc != 0)
    {
        log_err("getting addrinfo\n");
        goto ret;
    }

    ptr = res;

    sock_t hs = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if(hs < 0)
    {
        log_err("creating host socket\n");
        goto ret;
    }

    log_msg("created socket\n");

    rc = bind(hs, ptr->ai_addr, ptr->ai_addrlen);
    if(rc == -1)
    {
        socket_close(hs);
        log_err("binding host socket\n");
        goto ret;
    }
    //-----

    //listen on host
    rc = listen(hs, 10);
    if(rc == -1)
    {
        socket_close(hs);
        log_err("listening on host\n");
        goto ret;
    }

    host_started(host_id, host_env);

    log_msg("listening\n");
    struct host_t *host = malloc(sizeof(struct host_t));
    host->is_running = 1;
    host->is_stopped = 0;
    host->has_stopped = 0;

    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    fd_set master_set;
    fd_set working_set;
    FD_ZERO(&master_set);
    int max_sd = (int)hs;
    FD_SET(hs, &master_set);
    while(host->is_running)
    {
        memcpy(&working_set, &master_set, sizeof(master_set));
        rc = select(max_sd + 1, &working_set, 0, 0, &timeout);

        if(!host->is_running)
            break;

        if(rc < 0)
        {
            log_err("host select\n");
            break;
        }
        if(rc == 0)
        {
            continue;
        }

        sock_t c_socket = -1;
        do
        {
            c_socket = accept(hs, 0, 0);
            if(c_socket != -1)
            {
                log_msg("accepted client connection");
                struct client_env_t *c_env = malloc(sizeof(struct client_env));
                c_env->client_cb = client_cb;
                c_env->env = client_env;
                e_env->s = c_socket;
                
                thread_create(_client_cb, c_env);
            }
        } while(1);

    }

    host->is_stopped = 1;
    host_stop(host_id);
    //-----

    ret_val = 1;

ret:
    if(!tor_service_remove(host_id))
        log_err("removing tor service\n");

    return ret_val;
}

int host_stop(char *id)
{
    log_msg("stopping host ip: %s\n", ip);

    if(!_host_map_started)
    {
        log_err("host map not started\n");
        return 0;
    }

    struct host_t *host = host_map_get(ip);
    if(!host)
    {
        log_err("host not found\n");
        return 0;
    }

    if(host->has_stopped)
        return 1;

    if(!host_map_remove(ip))
        log_err("removing host from map\n");

    host->is_running = 0;

    int time_waited = 0;
    while(!host->is_stopped || time_waited <= MAX_WAIT_FOR_HOST_TIME)
    {
        sleep_ms(DELTA_WAIT_FOR_HOST_TIME);
        time_waited += DELTA_WAIT_FOR_HOST_TIME;
    }
    sleep_ms(500);      //wait for host to finish
    host->has_stopped = 1;

    if(!host->is_stopped)
    {
        log_err("Waited to long for host to stop\n");
        return 0;
    }

    free(host);

    return 1;
}

void *host_map_free_for_each(struct thread_map_t *this, struct host_t *host, char *id, void *ref)
{
    if(!host_stop(id))
    {
        log_err("stopping host ip: %s\n", id);
        return 0;
    }

    log_msg("host stopped: %s\n", id);
    return 0;
}

int host_delete()
{
    log_msg("stopping hosts\n");

    if(!_host_map_started)
    {
        log_msg("host not started\n");
        return 1;
    }

    if(thread_map_for_each(&_host_map, &host_map_free_for_each, 0))
    {
        log_err("stopping all hosts\n");
        return 0;
    }

    _host_map_started = 0;
    return 1;
}

