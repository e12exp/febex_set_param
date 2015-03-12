#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "../command.h"
#include "../data.h"

uint8_t _set_get_interpret_range(char *range, int *min, int *max)
{
  char *p[2];
  int n = 0;

  n = 0;
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
  int32_t value = strtol(str_value, NULL, 0);
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
	 int32_t *conf_val = module_data_get(sfp, mod, c, name, &val_min, &val_max, &vardef);

 	 if(conf_val == NULL)
 	 {
 	   printf("Invalid configuration variable.\n");
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
 	   printf("Invalid configuration variable.\n");
 	   return 0;
 	 }

        if(vardef->hooks.get != NULL)
          (*vardef->hooks.get)(sfp, mod, c, name, conf_val);

	if(c == -1)
	  printf("%d.%03d.%-40s [%d - %5" PRId64 "]: %d\n", sfp, mod, name, val_min, val_max, *conf_val);
	else
	  printf("%d.%03d.%02d.%-37s [%d - %5" PRId64 "]: %d\n", sfp, mod, c, name, val_min, val_max, *conf_val);
      }
    }
  }

  return 1;
}

