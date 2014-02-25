#ifndef _CONSOLEINTERFACE_H_
#define _CONSOLEINTERFACE_H_

#include <stdint.h>

void print_num_modules();
void get_command(char *cmd, int *argc, char **argv);
void free_command(char *cmd, int argc, char **argv);

#endif

