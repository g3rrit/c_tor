#include "socks.h"

#include <stdio.h>

#include "log.h"
#include <string.h>

sock_t socks_socket(char *ip, char *port);

int socks5_start(sock_t sock);

int socks5_auth(sock_t sock, char *usrname, char *password);

int socks5_request(sock_t sock, char *ip, char *port);

sock_t socks_connect(char *dest_ip, char *dest_port, char *ip, char *port, char *usrname, char *password)
{
    sock_t sock = socks_socket(ip, port);
    if(!sock)
        return 0;

    if(!socks5_start(sock))
        goto fail;

    if(!socks5_auth(sock, usrname, password))
        goto fail;

    if(!socks5_request(sock, dest_ip, dest_port))
        goto fail;

    log_msg("successfully established a connection\n");

    return sock;

fail:
    if(sock > 0)
        close(sock);
    return 0;
}

sock_t tor_connect(char *ip, char *port)
{
    log_msg("dest_ip: %s - dest_port: %s\n", ip, port);
    return socks_connect(ip, port, "127.0.0.1", "9050", "usr", "pwd");
}

sock_t socks_socket(char *ip, char *port)
{
    //establish socket
    uint16_t up = 0;
    sscanf((const char*) port, "%hu", &up);
    struct sockaddr_in addr = 
    {
        .sin_family = AF_INET,
        .sin_port = htons(up),
        .sin_addr.s_addr = inet_addr((const char*) ip),
    };
    sock_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        return 0;
    
    if(connect(sock, (struct sockaddr*) &addr, sizeof(addr)))
    {
        close(sock);
        return 0;
    }
    return sock;
}

int socks5_start(sock_t sock)
{
    uint8_t tx[3] = { 0x5, 0x1, 0x2 };    // version, number of methods, method
    uint8_t rx[2] = { 0 };                // version accepted method

    if(!socket_send_all(sock, tx, sizeof(tx)))
        return 0;

    if(!socket_recv_all(sock, rx, sizeof(rx)))
        return 0;

    if(tx[0] != rx[0] || tx[2] != rx[1])
        return 0;

    return 1;
}

int socks5_auth(sock_t sock ,char *usrname, char *password)
{
    uint32_t usr_l = strnlen((const char*) usrname, 256);    
    uint32_t pwd_l = strnlen((const char*) password, 256);

    if(usr_l == 256 || usr_l == 0 || pwd_l == 256 || pwd_l == 0)
        return 0;

    if(!socket_send_all(sock, (unsigned char*) "\x01", 1))       // auth method version
        return 0;

    if(!socket_send_all(sock, (unsigned char*) &usr_l, 1))       // username length
        return 0;

    if(!socket_send_all(sock, usrname, usr_l))
        return 0;

    if(!socket_send_all(sock, (unsigned char*) &pwd_l, 1))       //pasword length
        return 0;

    if(!socket_send_all(sock, password, pwd_l))
        return 0;

    uint8_t rx[2] = { 0 };                                  // auth method version, status
    if(!socket_recv_all(sock, rx, sizeof(rx)))
            return 0;

    if(rx[0] != 0x1 || rx[1] != 0x0)
        return 0;

    return 1;
}


int socks5_request(sock_t sock, char *ip, char *port)
{
    uint32_t ip_l = strnlen((const char*) ip, 256);

    if(ip_l == 256 || !ip_l)
        return 0;

    uint16_t up = 0;
    sscanf((const char*) port, "%hu", &up);
    up = htons(up);

    if(!up)
        return 0;

    uint8_t tx[4] = { 0x5, 0x1, 0x0, 0x3 };         // version, command, reserverd, type
    if(!socket_send_all(sock, tx, 4))
        return 0;

    if(!socket_send_all(sock, (uint8_t*) &ip_l, 1))
        return 0;

    if(!socket_send_all(sock, ip, ip_l))
        return 0;

    if(!socket_send_all(sock, (uint8_t*) &up, 2))
        return 0;

    uint8_t rx[256] = { 0 };                        //version, reply, reserved, type, address, port

    if(!socket_recv_all(sock, rx, 4))
        return 0;

    if(rx[0] != tx[0] || rx[1] || rx[2])
        return 0;

    switch(rx[3])
    {
        case 0x1:
            if(!socket_recv_all(sock, rx, 4))
                return 0;
            break;
        case 0x3:
            if(!socket_recv_all(sock, rx, 1))
                return 0;
            if(!socket_recv_all(sock, rx, rx[0]))
                return 0;
            break;
        case 0x4:
            if(!socket_recv_all(sock, rx, 16))
                return 0;
            break;
        default:
            return 0;
    }

    //test: socks still sends 2 bytes
    if(!socket_recv_all(sock, rx, 2))
        return 0;

    return 1;
}

