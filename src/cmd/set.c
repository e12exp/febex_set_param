#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "../command.h"
#include "../data.h"

uint8_t _set_get_interpret_range(char *range, int *min, int *max)
{
  char *p[2];

  p[0] = strtok(range, "-");
  if(p[0])
    p[1] = strtok(NULL, "-");
  else
    p[1] = NULL;

  if(strcmp(p[0], "*") == 0)
    *min = -1;
  else
    *min = atoi(p[0]);

  if(!p[1])
    *max = *min;
  else if(strcmp(p[1], "*") == 0)
    *max = -1;
  else
    *max = atoi(p[1]);

  return 1;
}

uint8_t _set_get_interpret_path(char *variable, int *sfp_first, int *sfp_last, int *module_first, int *module_last, int *channel_first, int *channel_last, char **name)
{
  char *path[5];
  int n = 0;

  path[0] = strtok(variable, ".");

  while(path[n] && n < 4)
    path[++n] = strtok(NULL, ".");

  if(n < 3 || n > 4)
  {
    printf("Invalid path for variable.\n");
    return 0;
  }

  _set_get_interpret_range(path[0], sfp_first, sfp_last);
  if(*sfp_first < 0)
    *sfp_first = 0;
  if(*sfp_last < 0)
    *sfp_last = g_num_sfp - 1;

  if(*sfp_first > g_num_sfp || *sfp_last > g_num_sfp)
  {
    printf("Invalid SFP\n");
    return 0;
  }

  _set_get_interpret_range(path[1], module_first, module_last);
  if(*module_first < 0)
   *module_first = 0;
  if(*module_last < 0)
   *module_last = g_num_modules[*sfp_first] - 1;

  if(n == 3)
  {
    *channel_first = *channel_last = -1;
    *name = path[2];
  }
  else
  {
    _set_get_interpret_range(path[2], channel_first, channel_last);
    if(*channel_first < 0)
      *channel_first = 0;
    if(*channel_last < 0)
      *channel_last = 15;

    *name = path[3];
  }

  return 1;
}

IMPL(set)
{
  ARGS_INIT
  ARG_STR(variable)
  ARG_STR(str_value)

//  int32_t value = atoi(str_value);
  char *valend;
  int32_t value = strtol(str_value, &valend, 0);
  int32_t val_min;
  int64_t val_max;
  conf_value_def_t *vardef;

  int sfp_first, sfp_last, module_first, module_last, channel_first, channel_last, sfp, mod, c;
  char *name;

  if(!_set_get_interpret_path(variable, &sfp_first, &sfp_last, &module_first, &module_last, &channel_first, &channel_last, &name))
    return 0;

  for(sfp = sfp_first; sfp <= sfp_last; sfp++)
  {
    for(mod = module_first; mod <= module_last; mod++)
    {
      for(c = channel_first; c <= channel_last; c++)
      {
         if(c >= g_arr_module_data[sfp][mod].firmware->num_channels)
           continue;

	 int32_t *conf_val = module_data_get(sfp, mod, c, name, &val_min, &val_max, &vardef);

 	 if(conf_val == NULL)
 	 {
 	   printf("Invalid configuration variable.\n");
 	   return 0;
 	 }

         if(vardef->type == conf_type_enum && valend == str_value)
         {
           // Enumeration value given as string
           if(!enum_get_value(vardef, str_value, &value))
           {
             printf("Invalid enumeration key for %s: %s\n", vardef->name, str_value);
             return 0;
           }
         }
         else if(*valend != '\0')
         {
           printf("Invalid value (not integer) for %s: %s\n", vardef->name, str_value);
           return 0;
         }

	 if(value < val_min || value > val_max)
	 {
	   printf("Value out of range. Allowed: %d - %" PRId64 "\n", val_min, val_max);
	   return 0;
	 }

         if(vardef->hooks.set != NULL)
         {
           // If hook is defined, it will tell us whether to still set the variable or not
           if((*vardef->hooks.set)(sfp, mod, c, name, &value) == 0)
             *conf_val = value;
         }
         else
 	  *conf_val = value;
      }
    }
  }

  return 1;
}

IMPL_HELP(set)
{
  printf("Set the value of a configuration parameter.\n\n"
      "  variable: Definition of parameter(s) to set\n"
      "  value: Value to set\n\n"
      "Note: The allowed values depend on the specific parameter.\n"
      "See \"help get\" for the description of the parameter definition.\n");

  return 1;
}

IMPL(get)
{
  ARGS_INIT
  ARG_STR(variable)

  int32_t val_min;
  int64_t val_max;
  int sfp_first, sfp_last, module_first, module_last, channel_first, channel_last, sfp, mod, c;
  char *name;
  conf_value_def_t *vardef;

  if(!_set_get_interpret_path(variable, &sfp_first, &sfp_last, &module_first, &module_last, &channel_first, &channel_last, &name))
    return 0;

  for(sfp = sfp_first; sfp <= sfp_last; sfp++)
  {
    for(mod = module_first; mod <= module_last; mod++)
    {
      for(c = channel_first; c <= channel_last; c++)
      {
        int32_t *conf_val = module_data_get(sfp, mod, c, name, &val_min, &val_max, &vardef);
        
        if(conf_val == NULL)
        {
          fprintf(stderr, "Invalid configuration variable.\n");
          return 0;
        }

        if(vardef->hooks.get != NULL)
          (*vardef->hooks.get)(sfp, mod, c, name, conf_val);

        if(c == -1)
          printf("%d.%03d.%-40s", sfp, mod, name);
        else
          printf("%d.%03d.%02d.%-37s", sfp, mod, c, name);

        if(vardef->type == conf_type_enum)
          printf(": %s (%d)", enum_get_value_display(vardef, *conf_val), *conf_val);
        else
          printf(" [%d - %5" PRId64 "]: %d", val_min, val_max, *conf_val);

        if(vardef->unit != NULL)
          printf(" (x %s)\n", vardef->unit);
        else
          printf("\n");
      }
    }
  }

  return 1;
}

IMPL_HELP(get)
{
  printf("Print the value of a configuration parameter.\n"
      "  variable: Definition of parameter(s) to get (see below)\n\n"
      "Parameter definition:\n"
      "  For module-wide parameters the basic syntax is\n"
      "  > get <sfp>.<module>.<parameter_name>\n"
      "  and for per-channel parameters the basic syntax is\n"
      "  > get <sfp>.<module>.<channel>.<parameter_name>\n"
      "  where\n"
      "    <sfp> is the index of SFP to use (0 - 3),\n"
      "    <module> is the index of the desired module in the given the SFP (0 - 255),\n"
      "    <channel> is the channel number to use (depending on firmware) and\n"
      "    <parameter_name> is the name of the parameter to get/set.\n\n"
      "  For the numeric values <sfp>, <module> and <channel>, ranges and wildcards "
      "are allowed to get/set the values of multiple modules/channels at once:\n"
      "    Ranges may be given as\n"
      "      <first>-<last>\n"
      "      where <first> and <last> are the IDs of the first/last SFP/module/channel to use.\n\n"
      "    An asterisk * is the wildcard character meaning\n"
      "      - All SFPs/modules/channels if used allone\n"
      "      - The first SFP/module/channel if used as <first> in a range (i.e. index 0)\n"
      "      - The last available SFP/module/channel if used as <last> in a range.\n\n"
      "  Examples:\n"
      "    > get 0.2.mau_peaktime\n"
      "    Get the value of the parameter \"mau_peaktime\" of the third module (index 2) "
      "in the first SFP chain (index 0).\n\n"
      "    > set 0.1.3.cfd_threshold_high 200\n"
      "    Set the value of the parameter \"cfd_threshold_high\" of the fourth channel (index 3) "
      "of the second module (index 1) in the first SFP chain (index 0) to '200'.\n\n"
      "    > set *.*.*.cfd_threshold_high 200\n"
      "    Set the value of the parameter \"cfd_threshold_high\" of ALL channels in all modules in all SFP chains to '200'.\n\n"
      "    > get 0.1-3.5.cfd_threshold_low\n"
      "    Get the value of the parameter \"cfd_threshold_low\" of the sixth channel (index 5) "
      "of the modules with IDs 1 - 3 of the first SFP chain (index 0).\n\n"
      "    > set 1.2.*.cfd_threshold_low 100\n"
      "    Set the value of the parameter \"cfd_threshold_low\" of all channels of the "
      "third module (index 2) in the second SFP chain (index 1) to '100'.\n\n"
      "  Of course, any combination of literals, ranges and wildcards are allowed. Be creative.\n");

  return 1;
}

