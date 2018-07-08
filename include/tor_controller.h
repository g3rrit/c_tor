#ifndef TOR_CONTROLLER_H
#define TOR_CONTROLLER_H

extern int tor_controller_socket;

int tor_start(char *tor_bin_dir, char **argv);

int tor_is_running();

int tor_send_command(char *command, char *res);

int tor_stop();

#endif