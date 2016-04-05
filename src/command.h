#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <stdint.h>

// Command implementation
#define IMPL(_name) int cmd_impl_ ## _name (int argc, char **argv)
#define DECL(_name) IMPL(_name)

// Command help implementation
#define IMPL_HELP(_name) int cmd_help_impl_ ## _name (int argc, char **argv)
#define DECL_HELP(_name) IMPL_HELP(_name)

// Sub-command implementation
#define IMPLS(_name, _subcmd) int cmd_impl_ ## _name ##  _ ## _subcmd (int argc, char **argv) 
#define DECLS(_name, _subcmd) IMPLS(_name, _subcmd)

// Sub-command help implementation
#define IMPLS_HELP(_name, _subcmd) int cmd_help_impl_ ## _name ##  _ ## _subcmd (int argc, char **argv) 
#define DECLS_HELP(_name, _subcmd) IMPLS_HELP(_name, _subcmd)

#define ARGS_INIT int _impls_arg_n = 0;
#define ARG_STR(_name) char *_name = argv[_impls_arg_n++];
#define ARG_INT(_name) int _name = atoi(argv[_impls_arg_n++]);

void register_commands();
uint8_t interpret_command(char *cmd, int argc, char **argv);

typedef int (*cmd_cb_t)(int argc, char **argv);

typedef struct
{
  char *name;
  char *subcmd;
  int num_args_required;
  char *args_required[32];
  int num_args_optional;
  char *args_optional[32];

  cmd_cb_t func;
  cmd_cb_t func_help;
}
cmd_impl_t;

extern cmd_impl_t commands[];
extern int g_num_commands;

#endif

