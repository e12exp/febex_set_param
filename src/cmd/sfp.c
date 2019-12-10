#include <stdlib.h>
#include <stdio.h>

#include "../command.h"
#include "../data.h"
#include "../consoleinterface.h"

IMPLS(add, sfp)
{
  ARGS_INIT
  ARG_INT(num)

  int sfp_id = module_data_add_sfp(g_file_data[g_active_file], num);

  if (!g_is_batch)
    {
      printf("ID of (last) inserted SFP: %d\n", sfp_id); 
      print_num_modules();
    }
  return 1;
}

IMPLS_HELP(add, sfp)
{
  printf("Add new SFP.\n"
      "  num: Number of SFPs to add (1 - 4)\n");

	return 1;
}
