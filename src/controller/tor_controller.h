#ifndef TOR_CONTROLLER_H
#define TOR_CONTROLLER_H

#define CON_RES_SIZE    256

int tor_start_controller(char *port, char *auth);

int tor_stop_controller();

int tor_send_command(char *command, char *res);

#endif