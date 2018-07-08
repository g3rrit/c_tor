#ifndef TOR_UTIL_H
#define TOR_UTIL_H

int socket_init();

int socket_delete();

int socket_recv_all(int sock, char *data, int size);

int socket_send_all(int sock, char *data, int size);

#endif