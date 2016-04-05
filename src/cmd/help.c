#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../command.h"
#include "../paramdef.h"

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

  firmware_def_t *fw = NULL;
  conf_value_def_t *cfg = NULL;

  uint8_t verbose = 1;

  if(argc < 1 || command == NULL)
  {
    command = NULL;
    verbose = 0;
  }

  if(argc < 2 || sub_command == NULL || strcmp(sub_command, "") == 0)
  {
    sub_command = NULL;
  }

  if(command != NULL && strcmp(command, "meaningoflife") == 0)
  {
    printf("Deep Thought says 42.\nSource: Douglas Adams\n");
    return 1;
  }

  if(command == NULL)
    printf("Available commands:\n");

  int i;
  uint8_t command_found = 0;

  for(i = 0; i < g_num_commands; i++)
  {
    if(command == 0 || strcmp(command, commands[i].name) == 0)
    {
      if(sub_command == NULL || (commands[i].subcmd != NULL && strcmp(sub_command, commands[i].subcmd) == 0))
      {
        command_found = 1;

        print_help_cmd(&commands[i]);
        if(verbose && ((sub_command == NULL && commands[i].subcmd == NULL) || (sub_command != NULL && commands[i].subcmd != NULL)) && commands[i].func_help != NULL)
        {
          printf("\n");
          (*commands[i].func_help)(0, NULL);
        }
      }
    }
  }

  if(command != NULL && !command_found)
  {
    // Try to get help on a parameter
    for(fw = fw_list_first(); fw != NULL; fw = fw_list_next())
    {
      if(sub_command == NULL || strcmp(sub_command, fw->name) == 0)
      {
        for(cfg = conf_list_first(fw); cfg != NULL; cfg = conf_list_next(fw))
        {
          if(strcmp(command, cfg->name) == 0)
          {
            command_found = 1;
            printf("%s parameter %s (Firmware %s)\n", (cfg->global ? "Module" : "Channel"),
                cfg->name, fw->name);
            if(cfg->description != NULL)
            {
              printf("%s\n\n", cfg->description);
            }
            else
            {
              printf("No help available for this parameter.\n\n");
            }
          }
        }
      }
    } 
  }

  if(command == NULL)
  {
    printf("\nType help <command> to get help on a specific command or\n"
             "     help <parameter> [firmware] to get help on a specific parameter.\n\n"
             "You can use the \"list\" command to see the available parameters.\n");
  }

  return 1;
}

