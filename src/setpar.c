#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "paramdef.h"
#include "filestruct.h"
#include "readfile.h"
#include "data.h"
#include "consoleinterface.h"
#include "command.h"

int main(int argc, char **argv)
{
  uint8_t stat;
  char cmd[64];
  int cmd_argc;
  char *cmd_argv[32];

  if(argc < 2)
  {
    fprintf(stderr, "Usage: %s file [cmd [cmd args]]\n", argv[0]);
    return -1;
  }

  module_data_init();
  register_vars();
  register_commands();
  g_display_level = user;
  
  if(readfile(argv[1]))
  {
    fill_data_from_file();
  }
  else
    printf("Creating new database.\n");

  if (argc!=2)
    {
      //interpret command returns 1 on success, 2 on error
      //and 0 on quit
      int res=interpret_command(argv[2], argc-3, &(argv[3]))!=1;
      if (!res 
	  && strcmp("get", argv[2]) 
	  && strcmp("list", argv[2])
	  && strcmp("help", argv[2])
	  )
	{
	  fill_regdata_from_module_data();
	  res+=write_file()!=1;
	}
      return res;
    }

  print_num_modules();

  stat = 1;

  do
  {
    get_command(cmd, &cmd_argc, cmd_argv);
    if(cmd_argc == -1)
    {
      continue;
    }
    stat = interpret_command(cmd, cmd_argc, cmd_argv);
    free_command(cmd, cmd_argc, cmd_argv);
  }
  while(stat);

  printf("Bye!\n");

  return 0;  
}

