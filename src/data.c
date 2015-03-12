#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "data.h"

module_data_t **g_arr_module_data;
uint8_t g_num_sfp;
uint8_t *g_num_modules;

void module_data_init()
{
  g_arr_module_data = NULL;
  g_num_sfp = 0;
  g_num_modules = NULL;
}

void module_data_prepare(uint8_t sfp, uint8_t module, module_data_t *data, firmware_def_t *firmware)
{
  uint8_t c = 0, ncv, ngv;
  conf_value_def_t *v;
  firmware_def_t *fw;

  data->sfp = sfp;
  data->module = module;
  fw = data->firmware = firmware;


  data->arr_global_cfg = (conf_value_data_t*)malloc(fw->num_global_config_vars * sizeof(conf_value_data_t));
  
  data->arr_channel_cfg = (conf_value_data_t**)malloc(16 * sizeof(conf_value_data_t*));

  for(c = 0; c < 16; c++)
    data->arr_channel_cfg[c] = (conf_value_data_t*)malloc(fw->num_channel_config_vars * sizeof(conf_value_data_t));

  v = conf_list_first(fw);
  ngv = 0;
  ncv = 0;
  while(v)
  {
    if(v->global)
    {
      data->arr_global_cfg[ngv].value_def = v;
      data->arr_global_cfg[ngv++].value_data = 0;
    }
    else
    {
      for(c = 0; c < 16; c++)
      {
	data->arr_channel_cfg[c][ncv].value_def = v;
	data->arr_channel_cfg[c][ncv].value_data = 0;
      }
      ncv++;
    }

    v = conf_list_next(fw);
  }
}

uint8_t module_data_add_sfp(uint8_t num)
{
  uint8_t *tmp_num_modules = (uint8_t*)malloc(g_num_sfp + num);
  module_data_t **tmp_arr_module_data = (module_data_t**)malloc(sizeof(module_data_t*) * (g_num_sfp + num));

  if(g_num_modules != NULL)
  {
    memcpy(tmp_num_modules, g_num_modules, sizeof(uint8_t) * g_num_sfp);
    memcpy(tmp_arr_module_data, g_arr_module_data, sizeof(module_data_t*) * g_num_sfp);
    free(g_num_modules);
    free(g_arr_module_data);
  }

  g_num_modules = tmp_num_modules;
  g_arr_module_data = tmp_arr_module_data;
  memset(&g_num_modules[g_num_sfp], 0, num);
  memset(&g_arr_module_data[g_num_sfp], 0, sizeof(module_data_t*) * num);
  g_num_sfp += num;

  return g_num_sfp - 1;
}

int16_t module_data_add_module(uint8_t sfp, uint8_t num, uint32_t firmware)
{
  uint8_t m;
  firmware_def_t *fw;

  module_data_t *tmp_arr_module_data = (module_data_t*)malloc(sizeof(module_data_t) * (num + g_num_modules[sfp]));

  if(g_arr_module_data[sfp] != NULL)
  {
    memcpy(tmp_arr_module_data, g_arr_module_data[sfp], sizeof(module_data_t) * g_num_modules[sfp]);
//    free(g_arr_module_data[sfp]);
  }

//  g_arr_module_data[sfp] = tmp_arr_module_data;
  
  for(m = g_num_modules[sfp]; m < g_num_modules[sfp] + num; m++)
  {
    if(!(fw = get_firmware_def(firmware)))
    {
      fprintf(stderr, "Invalid firmware ID: 0x%08x (SFP %d, module %d)\n", firmware, sfp, m);
      free(tmp_arr_module_data);
      return -1;
    }

    module_data_prepare(sfp, m, &tmp_arr_module_data[m], fw);
  }

  free(g_arr_module_data[sfp]);
  g_arr_module_data[sfp] = tmp_arr_module_data;

  g_num_modules[sfp] += num;

  return g_num_modules[sfp] - 1;
}

uint8_t module_data_remove_module(uint8_t sfp, uint8_t module, uint8_t num)
{
  if(sfp >= g_num_sfp || module >= g_num_modules[sfp])
    return 0;

  if(module + num > g_num_modules[sfp])
    num = g_num_modules[sfp] - module;

  module_data_t *tmp_arr_module_data;
  if(g_num_modules[sfp] - num > 0)
    tmp_arr_module_data = (module_data_t*)malloc(sizeof(module_data_t) * (g_num_modules[sfp] - num));
  else
    tmp_arr_module_data = NULL;

  if(module != 0)
    memcpy(tmp_arr_module_data, g_arr_module_data[sfp], module * sizeof(module_data_t));

  if(g_num_modules[sfp] - num - module > 0)
     memcpy(&tmp_arr_module_data[module], &g_arr_module_data[sfp][module + num], (g_num_modules[sfp] - num - module) * sizeof(module_data_t));

  free(g_arr_module_data[sfp]);
  g_arr_module_data[sfp] = tmp_arr_module_data;
  g_num_modules[sfp] -= num;

  return 1;
}

conf_value_data_t *config_array_search(conf_value_data_t *array, int n, char *name)
{
  int i;

  for(i = 0; i < n; i++)
  {
    if(strcmp(array[i].value_def->name, name) == 0)
      return &array[i];
  }

  return NULL;
}

int32_t *module_data_get(uint8_t sfp, uint8_t module, int8_t channel, char *name, int32_t *val_min, int64_t *val_max, conf_value_def_t **def)
{
  if(sfp >= g_num_sfp)
    return NULL;

  if(module >= g_num_modules[sfp])
    return NULL;

  conf_value_data_t *d;

  if(channel < 0)
  {
    d = config_array_search(g_arr_module_data[sfp][module].arr_global_cfg,
	g_arr_module_data[sfp][module].firmware->num_global_config_vars, name);
  }
  else if(channel >= 16)
    return NULL;
  else
  {
    d = config_array_search(g_arr_module_data[sfp][module].arr_channel_cfg[channel],
	g_arr_module_data[sfp][module].firmware->num_channel_config_vars, name);
  }

  if(d == NULL)
    return NULL;

  if(val_max != NULL)
  {
    if(d->value_def->vsigned)
      *val_max = (int64_t)((int32_t)d->value_def->bitmask >> (int32_t)d->value_def->lowbit) / 2;
    else
      *val_max = (int64_t)((uint32_t)d->value_def->bitmask >> (uint32_t)d->value_def->lowbit);
  }

  if(val_min != NULL)
  {
	if(d->value_def->vsigned)
		*val_min = -(*val_max) - 1;
	else
		*val_min = 0;
  }

  if(def != NULL)
    *def = d->value_def;

  return &d->value_data;
}

