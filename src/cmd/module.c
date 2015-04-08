#include <stdlib.h>
#include <stdio.h>

#include "../command.h"
#include "../data.h"
#include "../consoleinterface.h"

IMPLS(add, module)
{
  ARGS_INIT
  ARG_INT(sfp)
  ARG_INT(num)
  ARG_STR(firmware)

  uint32_t fw_id;

  if(sfp >= g_num_sfp)
  {
    fprintf(stderr, "Invalid SFP.\n");
    return 0;
  }

  if(!(fw_id = get_firmware_id(firmware)))
  {
    fprintf(stderr, "Invalid firmware name.\n");
    return 0;
  }

  int module_id = module_data_add_module(sfp, num, fw_id);

  printf("ID of (last) inserted module: %d\n", module_id);

  print_num_modules(); 

  return 1;
}

IMPLS(rm, module)
{
  ARGS_INIT
  ARG_INT(sfp)
  ARG_INT(module)
  ARG_INT(num)

  if(!module_data_remove_module(sfp, module, num))
  {
    fprintf(stderr, "Module(s) could not be removed.\n");
    return 0;
  }

  print_num_modules();
  return 1;
}

IMPLS(cp, module)
{
  ARGS_INIT
  ARG_INT(src_sfp)
  ARG_INT(src_mod)
  ARG_INT(dst_sfp)
  ARG_INT(num)

  int num_org, id_last, m, c, v;
  firmware_def_t *fw;

  if(src_sfp >= g_num_sfp || src_mod >= g_num_modules[src_sfp])
  {
    fprintf(stderr, "Invalid source.\n");
    return 0;
  }

  if(dst_sfp >= g_num_sfp)
  {
    fprintf(stderr, "Invalid destination.\n");
    return 0;
  }

  num_org = g_num_modules[dst_sfp];

  fw = g_arr_module_data[src_sfp][src_mod].firmware;
  id_last = module_data_add_module(dst_sfp, num, fw->id);

  for(m = num_org; m <= id_last; m++)
  {
    for(v = 0; v < fw->num_global_config_vars; v++)
    {
      g_arr_module_data[dst_sfp][m].arr_global_cfg[v].value_data
	= g_arr_module_data[src_sfp][src_mod].arr_global_cfg[v].value_data;
    }
    for(c = 0; c < fw->num_channels; c++)
    {
      for(v = 0; v < fw->num_channel_config_vars; v++)
      {
	g_arr_module_data[dst_sfp][m].arr_channel_cfg[c][v].value_data
	  = g_arr_module_data[src_sfp][src_mod].arr_channel_cfg[c][v].value_data;
      }
    }
  }

  print_num_modules();

  return 0;
}

