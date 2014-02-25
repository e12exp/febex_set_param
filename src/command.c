#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "command.h"

#define IMPL(_name) int cmd_impl_ ## _name (int argc, char **argv)

#define IMPLS(_name, _subcmd) int cmd_impl_ ## _name ##  _ ## _subcmd (int argc, char **argv) 
#define ARGS_INIT int _impls_arg_n = 0;

#define ARG_STR(_name) char *_name = argv[_impls_arg_n++];
#define ARG_INT(_name) int _name = atoi(argv[_impls_arg_n++]);

#define ARGS(...) { __VA_ARGS__ }
#define ARG(_name, _def) #_name, #_def

#define CMD(_name, _num_req, _req, _num_opt, _opt) {#_name, NULL, _num_req, _req, _num_opt, _opt, cmd_impl_ ## _name}
#define CMDS(_name, _subcmd, _num_req, _req, _num_opt, _opt) {#_name, #_subcmd, _num_req, _req, _num_opt, _opt, cmd_impl_## _name ## _ ## _subcmd}

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
}
cmd_impl_t;

#include "commands.def"

uint8_t check_arguments(cmd_impl_t *cmd, int *argc, char **argv)
{
  int num_args_required = cmd->num_args_required;
  int num_args_opt = cmd->num_args_optional;
  int l, i;
  char *s;

//  printf("req: %d, opt: %d\n", num_args_required, num_args_opt);
//  for(i = 0; i < num_args_required; i++)
//    printf("req %s\n", cmd->args_required[i]);
//  for(i = 0; i < num_args_opt; i++)
//    printf("opt %s (%s)\n", cmd->args_optional[2*i], cmd->args_optional[2*i+1]);

  if(*argc < num_args_required)
  {
    fprintf(stderr, "Too few arguments.\n");
    return 0;
  }

  if(*argc - num_args_required < num_args_opt)
  {
    for(i = *argc - num_args_required ; i < num_args_opt; i++)
    {
      //printf("i, argc: %d\n", *argc);

      s = cmd->args_optional[2*i+1];

      //printf("s: %s\n", s);

      l = strlen(s);

      //printf("l: %d\n", l);

      argv[*argc] = (char*)malloc(sizeof(char) * (l + 1));
      strncpy(argv[(*argc)++], s, l+1);
    }
  }

  return 1;
}

uint8_t print_lasagne()
{
  printf("        (\\w/)\n");
  printf("        (..  \\\n");
  printf("       _/  )  \\______\n");
  printf("      (oo /'\\        )`,\n");
  printf("       `--' (v  __( / ||\n");
  printf("             |||  ||| ||\n");
  printf("            //_| //_|\n");
  printf(" (c)  Max Winkel, Ben Pietras, Michael Bendel, jv\n");

  return 1;
}

uint8_t interpret_command(char *cmd, int argc, char **argv)
{
  unsigned int i;

//  printf("Command: %s\n", cmd);

//  printf("argc: %d\n", argc);

//  for(i = 0; i < argc; i++)
//    printf("argv[%d]: %s\n", i, argv[i]);

  if(strcmp(cmd, "exit") == 0)
    return 0;

  if(strcmp(cmd, "lasagne") == 0)
    return print_lasagne();

  for(i = 0; i < sizeof(commands) / sizeof(cmd_impl_t); i++)
  {
    if(strcmp(cmd, commands[i].name) == 0)
    {
      if(commands[i].subcmd == NULL)
      {
	if(!check_arguments(&commands[i], &argc, argv))
	{
	  fprintf(stderr, "Invalid arguments for command %s\n", cmd);
	  return 1;
	}
        (*commands[i].func)(argc, argv);
	return 1;
      }
      else if(argc >= 1 && strcmp(argv[0], commands[i].subcmd) == 0)
      {
	argc--;
	if(!check_arguments(&commands[i], &argc, &argv[1]))
	{
	  argc++;
	  fprintf(stderr, "Invalid arguments for command %s %s\n", cmd, argv[0]);
	  return 1;
	}
	argc++;
	(*commands[i].func)(argc, &argv[1]);
	return 1;
      }
    }
  }

  fprintf(stderr, "Unkown command %s.\n", cmd);

  return 1;
}

