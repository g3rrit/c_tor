#ifndef TOR_SERVICE_H
#define TOR_SERVICE_H

int tor_service_init(char *tor_bin_dir);

int tor_service_add(int port, char *service_id);

int tor_service_remove(char *service_id);

int tor_service_delete();

#endif