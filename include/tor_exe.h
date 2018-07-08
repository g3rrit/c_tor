#ifndef TOR_EXE_H
#define TOR_EXE_H

int tor_start(char *tor_bin_dir, char **argv);

int tor_is_running();

int tor_stop();

#endif

