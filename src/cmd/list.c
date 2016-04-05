#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "../command.h"
#include "../data.h"
#include "../consoleinterface.h"
#include "../paramdef.h"
#include "../binaryval.h"

IMPL(list)
{
  ARGS_INIT
  ARG_INT(sfp)
  ARG_INT(mod)

  int c, v, sfp_min, sfp_max, mod_min, mod_max, n_var;
  conf_value_data_t *var;

  int64_t val_min, val_max;

  firmware_def_t *fw;

  print_num_modules();

  if(sfp < 0)
  {
    sfp_min = 0; sfp_max = g_num_sfp - 1;
  }
  else
  {
    sfp_min = sfp_max = sfp;
  }
  if(sfp_max >= g_num_sfp)
  {
    fprintf(stderr, "Invalid SFP\n");
    return 0;;
  }

  if(mod < 0)
  {
    mod_min = 0; mod_max = -1;
  }
  else
  {
    mod_min = mod_max = mod;
  }

  if(!(mod_max == -1 || mod_max < g_num_modules[sfp_max]))
  {
    fprintf(stderr, "Invalid module\n");
    return 0;
  }

  for(sfp = sfp_min; sfp <= sfp_max; sfp++)
  {
    printf("SFP %d\n", sfp);

    for(mod = mod_min; mod <= (mod_max == -1 ? g_num_modules[sfp] - 1 : mod_max); mod++)
    {
      fw = g_arr_module_data[sfp][mod].firmware;

      printf("+- Module %d\n", mod);
      printf("     Firmware %s (0x%08x - 0x%08x, Recommended 0x%08x)\n", fw->name, fw->fw_min, fw->fw_max,
          fw->fw_recommended);

      for(c = -1; c < fw->num_channels; c++)
      {
        if(c == -1)
          n_var = fw->num_global_config_vars;
        else
          n_var = fw->num_channel_config_vars;

        for(v = 0; v < n_var; v++)
        {
          if(c == -1)
            var = &g_arr_module_data[sfp][mod].arr_global_cfg[v];
          else
            var = &g_arr_module_data[sfp][mod].arr_channel_cfg[c][v];
          
          if(g_display_level > var->value_def->display_level)
            continue;
          
          val_min = 0;
          val_max = var->value_def->bitmask >> var->value_def->lowbit;

          // Execute hook to get value
          if(var->value_def->hooks.get != NULL)
            (*var->value_def->hooks.get)(sfp, mod, c, var->value_def->name, &var->value_data);

          if(var->value_def->vsigned)
          {
          	val_min = -(val_max/2 + 1);
          	val_max /= 2;
          }

          if(c == -1)
            printf("%d.%03d.%-40s", sfp, mod, var->value_def->name);
          else
            printf("%d.%03d.%02d.%-37s", sfp, mod, c, var->value_def->name);

          if(g_display_level <= expert)
            // Show variable address
            printf(" (0x%06x)", var->value_def->addr);

          if(var->value_def->type == conf_type_mask)
          {
              printf(": 0b");
              print_binary_val(var->value_data, val_max);
          }
          else if(var->value_def->type == conf_type_enum)
          {
              printf(" [");
              for(var->value_def->enum_value_current = var->value_def->enum_value_list; var->value_def->enum_value_current != NULL; var->value_def->enum_value_current = var->value_def->enum_value_current->next)
              {
                printf("%s%s", var->value_def->enum_value_current->display,
                    var->value_def->enum_value_current->next != NULL ? ", " : "");
              }
              printf("]: %s (%d)\n", enum_get_value_display(var->value_def, var->value_data), var->value_data);
          }
          else
          {
              printf(" [%" PRId64 " - %5" PRId64 "]: %d\n", 
                 val_min, val_max, var->value_data);
          }
        }
        printf("\n");
      }
    }
  }

  return 1;
}

IMPL_HELP(list)
{
  printf("List configuration parameters of one or more modules.\n"
      "  sfp: Index of SFP (0 - 3) of which the parameters should be listed (Default: all)\n"
      "  module: Index of module of which the parameters should be listed (Default: all)\n\n"
      "Note (1): Without any parameters, list will give you a complete dump of all parameters.\n"
      "Note (2): Depending on the current display level, some configuration parameters "
      "may be omitted.\n"
      "See \"help display\" for details on the display level.\n");

  return 1;
}

