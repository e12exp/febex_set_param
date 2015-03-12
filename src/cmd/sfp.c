#include <stdlib.h>
#include <stdio.h>

#include "../command.h"
#include "../data.h"
#include "../consoleinterface.h"

IMPLS(add, sfp)
{
  ARGS_INIT
  ARG_INT(num)

  int sfp_id = module_data_add_sfp(num);

  printf("ID of (last) inserted SFP: %d\n", sfp_id);
  
  print_num_modules();

  return 1;
}
