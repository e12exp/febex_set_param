#include "../data.h"
#include "../consoleinterface.h"
#include "../paramdef.h"
#include "../binaryval.h"

IMPL(list)
{
  ARGS_INIT
  ARG_INT(sfp)
  ARG_INT(mod)

  int c, v, sfp_min, sfp_max, mod_min, mod_max;
  conf_value_data_t *var;

  int32_t val_min, val_max;

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

      for(v = 0; v < fw->num_global_config_vars; v++)
      {
	var = &g_arr_module_data[sfp][mod].arr_global_cfg[v];
	val_min = 0;
	val_max = var->value_def->bitmask >> var->value_def->lowbit;

	if(var->value_def->vsigned)
	{
		val_min = -(val_max/2 + 1);
		val_max /= 2;
	}

        if(var->value_def->type == conf_type_mask)
        {
            printf("%d.%03d.%-40s (0x%06x): 0b", sfp, mod, var->value_def->name,
                  var->value_def->addr);
            print_binary_val(var->value_data, val_max);
        }
        else
        {
	    printf("%d.%03d.%-40s (0x%06x) [%d - %5d]: %d\n", sfp, mod,
	       var->value_def->name, var->value_def->addr,
	       val_min, val_max, var->value_data);
        }
      }
      printf("\n");

      if(fw->num_channel_config_vars == 0)
        continue;

      for(c = 0; c < 16; c++)
      {
	for(v = 0; v < fw->num_channel_config_vars; v++)
	{
	  var = &g_arr_module_data[sfp][mod].arr_channel_cfg[c][v];
	  val_min = 0;
	  val_max = var->value_def->bitmask >> var->value_def->lowbit;

	  if(var->value_def->vsigned)
	  {
		val_min = -(val_max/2 + 1);
		val_max /= 2;
	  }

          if(var->value_def->type == conf_type_mask)
          {
              printf("%d.%03d.%02d.%-37s (0x%06x): 0b", sfp, mod, c, var->value_def->name,
                    var->value_def->addr);
              print_binary_val(var->value_data, val_max);
          }
          else
          {
	    printf("%d.%03d.%02d.%-37s (0x%06x) [0 - %5d]: %d\n", sfp, mod, c,
	      var->value_def->name, var->value_def->addr,
	      val_max, var->value_data);
          }
	}
	printf("\n");
      }
    }
  }

  return 1;
}

