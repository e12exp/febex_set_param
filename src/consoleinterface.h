#ifndef _CONSOLEINTERFACE_H_
#define _CONSOLEINTERFACE_H_

#include "paramdef.h"

#include <stdint.h>

void print_num_modules();
void get_command(char *cmd, int *argc, char **argv);
void free_command(char *cmd, int argc, char **argv);

extern display_level_t g_display_level;

#endif

