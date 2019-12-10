#ifndef _CONSOLEINTERFACE_H_
#define _CONSOLEINTERFACE_H_

#include "paramdef.h"

#include <stdint.h>
#include <stdio.h>

void print_num_modules();
void get_command(char *cmd, int *argc, char **argv, FILE* fd, char* cmdline);
void free_command(char *cmd, int argc, char **argv);

int eval_print_loop(FILE* fd);

extern display_level_t g_display_level;
extern bool g_is_batch;

#endif

