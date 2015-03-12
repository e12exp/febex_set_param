#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../command.h"

void print_help_cmd(cmd_impl_t *cmd)
{
  int i;

  printf("  %s ", cmd->name);
  if(cmd->subcmd != NULL)
    printf("%s ", cmd->subcmd);

  for(i = 0; i < cmd->num_args_required; i++)
    printf("<%s> ", cmd->args_required[i]);

  for(i = 0; i < cmd->num_args_optional; i++)
    printf("[%s = %s] ", cmd->args_optional[2*i], cmd->args_optional[2*i + 1]);

  printf("\n");
}

IMPL(help)
{
  ARGS_INIT
  ARG_STR(command)
  ARG_STR(sub_command)

  if(argc < 1 || command == NULL)
    command = NULL;

  if(argc < 2 || sub_command == NULL || strcmp(sub_command, "") == 0)
    sub_command = NULL;

  if(command != NULL && strcmp(command, "meaningoflife") == 0)
  {
    printf("Deep Blue says 42.\nSource: Douglas Adams\n");
    return 1;
  }

  if(command == NULL)
    printf("Available commands:\n");
  else
    printf("Usage: ");

  int i;

  for(i = 0; i < g_num_commands; i++)
  {
    if(command == 0 || strcmp(command, commands[i].name) == 0)
    {
      if(sub_command == NULL || (commands[i].subcmd != NULL && strcmp(sub_command, commands[i].subcmd) == 0))
	print_help_cmd(&commands[i]);
    }
  }

  return 1;
}

