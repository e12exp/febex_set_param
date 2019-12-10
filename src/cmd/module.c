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

  if(sfp >= g_file_data[g_active_file]->num_sfp)
  {
    fprintf(stderr, "Invalid SFP.\n");
    return 0;
  }

  if(!(fw_id = get_firmware_id(firmware)))
  {
    fprintf(stderr, "Invalid firmware name.\n");
    return 0;
  }

  int module_id = module_data_add_module(g_file_data[g_active_file], sfp, num, fw_id);


  if (!g_is_batch)
    {  
      printf("ID of (last) inserted module: %d\n", module_id);
      print_num_modules(); 
    }

  return 1;
}

IMPLS_HELP(add, module)
{
  printf("Add module(s) to given SFP.\n"
      "  sfp: Index of SFP (0 - 3) to add modules to\n"
      "  num: Number of modules to add (Default: 1)\n"
      "  firmware: Name of firmware to use for the module\n"
      "            If omitted, the default firmware is used.\n"
      "            Use the command \"firmware list\" to see the available firmwares.\n\n"
      "Note: New modules will always be added to the end of the existing module chain.\n");

  return 1;
}

IMPLS(rm, module)
{
  ARGS_INIT
  ARG_INT(sfp)
  ARG_INT(module)
  ARG_INT(num)

  if(!module_data_remove_module(g_file_data[g_active_file], sfp, module, num))
  {
    fprintf(stderr, "Module(s) could not be removed.\n");
    return 0;
  }

  print_num_modules();
  return 1;
}

IMPLS_HELP(rm, module)
{
  printf("Delete module(s) from a given SFP.\n"
      "  sfp: Index of SFP (0 - 3) of which the module(s) should be deleted\n"
      "  module: Index of first module in the chain to delete\n"
      "  num: Number of modules to delete (Default: 1)\n\n"
      "Note: If the modules are not deleted from the end of the chain,\n"
      "the following modules will be shifted up in the chain.\n");

  return 1;
}

IMPLS(cp, module)
{
  ARGS_INIT
  ARG_INT(src_sfp)
  ARG_INT(src_mod)
  ARG_INT(dst_sfp)
  ARG_INT(num)
  ARG_INT(src_file)

  file_data_t *file_dst = g_file_data[g_active_file];

  if(src_file < 0)
    src_file = g_active_file;

  if(src_file >= g_num_files)
  {
    fprintf(stderr, "Invalid source file number.\n");
    return 0;
  }

  file_data_t *file_src = g_file_data[src_file];

  int num_org, id_last, m, c, v;
  firmware_def_t *fw;

  if(src_sfp >= file_src->num_sfp || src_mod >= file_src->num_modules[src_sfp])
  {
    fprintf(stderr, "Invalid source.\n");
    return 0;
  }

  if(dst_sfp >= file_dst->num_sfp)
  {
    fprintf(stderr, "Invalid destination.\n");
    return 0;
  }

  num_org = file_dst->num_modules[dst_sfp];

  fw = file_src->module_data[src_sfp][src_mod].firmware;
  id_last = module_data_add_module(file_dst, dst_sfp, num, fw->id);

  for(m = num_org; m <= id_last; m++)
  {
    for(v = 0; v < fw->num_global_config_vars; v++)
    {
      file_dst->module_data[dst_sfp][m].arr_global_cfg[v].value_data
        = file_src->module_data[src_sfp][src_mod].arr_global_cfg[v].value_data;
    }
    for(c = 0; c < fw->num_channels; c++)
    {
      for(v = 0; v < fw->num_channel_config_vars; v++)
      {
        file_dst->module_data[dst_sfp][m].arr_channel_cfg[c][v].value_data
          = file_src->module_data[src_sfp][src_mod].arr_channel_cfg[c][v].value_data;
      }
    }
  }

  print_num_modules();

  return 1;
}

IMPLS_HELP(cp, module)
{
  printf("Copy module within one SFP or from one to another.\n"
      "  src-sfp:   Index of SFP (0 - 3) of the source module\n"
      "  src-mod:   Module number in given SFP chain to copy\n"
      "  dst-sfp:   Index of SFP (0 - 3) to which the copied modules should be inserted\n"
      "  num:       Number of identical copies to insert (Default: 1)\n"
      "  src-file:  Source file to copy module from (Default: Active file)\n\n"
      "Note: The command will create <num> copies of one and the same source module.\n");

  return 1;
}

