#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "readfile.h"
#include "data.h"

file_data_t **g_file_data;
uint8_t g_num_files;
uint8_t g_active_file;

void file_data_init()
{
  g_file_data = NULL;
  g_num_files = 0;
  g_active_file = 0;
}

file_data_t *file_data_add(const char *filename)
{
  uint8_t num_files = g_num_files + 1;
  size_t fname_len;
  file_data_t **file_data = (file_data_t**)malloc(sizeof(file_data_t*) * num_files);
  file_data_t *file;

  if(g_num_files != 0 && g_file_data != NULL)
  {
    memcpy(file_data, g_file_data, sizeof(file_data_t*)*g_num_files);
    free(g_file_data);
  }

  file = (file_data_t*)calloc(1, sizeof(file_data_t));
  file->version = FILEVERSION_RECENT;

  fname_len = strlen(filename);
  file->filename = (char*)calloc(1, fname_len);
  strcpy(file->filename, filename);

  g_file_data = file_data;
  g_file_data[g_num_files++] = file;

  return file;
}

void file_data_close(uint8_t fileno)
{
  file_data_t **file_data;

  if(fileno >= g_num_files)
    return;

  file_data_free(g_file_data[fileno]);

  if(g_num_files == 1)
  {
    // Last open file
    g_num_files = 0;
    free(g_file_data);
    g_file_data = NULL;
  }
  else
  {
    file_data = (file_data_t**)malloc(sizeof(file_data_t*) * (g_num_files-1));
    if(fileno != 0)
    {
      memcpy(file_data, g_file_data, sizeof(file_data_t*) * fileno);
    }
    if(fileno != g_num_files - 1)
    {
      memcpy(&file_data[fileno], &g_file_data[fileno+1], sizeof(file_data_t*) * (g_num_files-fileno-1));
    }

    free(g_file_data);
    g_file_data = file_data;
    g_num_files--;
  }
}

void file_data_free(file_data_t *file)
{
  uint8_t sfp, mod;

  if(file->module_data != NULL)
  {
    for(sfp = 0; sfp < file->num_sfp; sfp++)
    {
      for(mod = 0; mod < file->num_modules[sfp]; mod++)
      {
        module_data_free(&file->module_data[sfp][mod]);
      }
      free(file->module_data[sfp]);
    }
    free(file->module_data);
    file->module_data = NULL;
  }

  if(file->num_modules != NULL)
  {
    free(file->num_modules);
    file->num_modules = NULL;
  }

  if(file->filename != NULL)
  {
    free(file->filename);
    file->filename = NULL;
  }
}

//void module_data_init(file_data_t *file)
//{
//  file->filename = NULL;
//
//  file->module_data = NULL;
//  file->num_sfp = 0;
//  file->num_modules = NULL;
//}

void module_data_prepare(uint8_t sfp, uint8_t module, module_data_t *data, firmware_def_t *firmware)
{
  uint8_t c = 0, ncv, ngv;
  conf_value_def_t *v;
  firmware_def_t *fw;

  data->sfp = sfp;
  data->module = module;
  fw = data->firmware = firmware;

  data->arr_global_cfg = (conf_value_data_t*)malloc(fw->num_global_config_vars * sizeof(conf_value_data_t));

  data->arr_channel_cfg = (conf_value_data_t**)malloc(firmware->num_channels * sizeof(conf_value_data_t*));

  for(c = 0; c < firmware->num_channels; c++)
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
      for(c = 0; c < fw->num_channels; c++)
      {
        data->arr_channel_cfg[c][ncv].value_def = v;
        data->arr_channel_cfg[c][ncv].value_data = 0;
      }
      ncv++;
    }

    v = conf_list_next(fw);
  }
}

void module_data_free(module_data_t *data)
{
  uint8_t c;

  if(data->arr_global_cfg != NULL)
  {
    free(data->arr_global_cfg);
  }

  if(data->arr_channel_cfg != NULL)
  {
    for(c = 0; c < data->firmware->num_channels; c++)
    {
      free(data->arr_channel_cfg[c]);
    }
    free(data->arr_channel_cfg);
  }

  data->arr_global_cfg = NULL;
  data->arr_channel_cfg = NULL;
}

uint8_t module_data_add_sfp(file_data_t *file, uint8_t num)
{
  uint8_t *tmp_num_modules = (uint8_t*)malloc(file->num_sfp + num);
  module_data_t **tmp_arr_module_data = (module_data_t**)malloc(sizeof(module_data_t*) * (file->num_sfp + num));

  if(file->num_modules != NULL)
  {
    memcpy(tmp_num_modules, file->num_modules, sizeof(uint8_t) * file->num_sfp);
    memcpy(tmp_arr_module_data, file->module_data, sizeof(module_data_t*) * file->num_sfp);
    free(file->num_modules);
    free(file->module_data);
  }

  file->num_modules = tmp_num_modules;
  file->module_data = tmp_arr_module_data;
  memset(&file->num_modules[file->num_sfp], 0, num);
  memset(&file->module_data[file->num_sfp], 0, sizeof(module_data_t*) * num);
  file->num_sfp += num;

  return file->num_sfp - 1;
}

int16_t module_data_add_module(file_data_t *file, uint8_t sfp, uint8_t num, uint32_t firmware)
{
  uint8_t m;
  firmware_def_t *fw;

  module_data_t *tmp_arr_module_data = (module_data_t*)malloc(sizeof(module_data_t) * (num + file->num_modules[sfp]));

  if(file->module_data[sfp] != NULL)
  {
    memcpy(tmp_arr_module_data, file->module_data[sfp], sizeof(module_data_t) * file->num_modules[sfp]);
    //    free(g_arr_module_data[sfp]);
  }

  //  g_arr_module_data[sfp] = tmp_arr_module_data;

  for(m = file->num_modules[sfp]; m < file->num_modules[sfp] + num; m++)
  {
    if(!(fw = get_firmware_def(firmware)))
    {
      fprintf(stderr, "Invalid firmware ID: 0x%08x (SFP %d, module %d)\n", firmware, sfp, m);
      free(tmp_arr_module_data);
      return -1;
    }

    module_data_prepare(sfp, m, &tmp_arr_module_data[m], fw);
  }

  free(file->module_data[sfp]);
  file->module_data[sfp] = tmp_arr_module_data;

  file->num_modules[sfp] += num;

  return file->num_modules[sfp] - 1;
}

uint8_t module_data_remove_module(file_data_t *file, uint8_t sfp, uint8_t module, uint8_t num)
{
  if(sfp >= file->num_sfp || module >= file->num_modules[sfp])
    return 0;

  if(module + num > file->num_modules[sfp])
    num = file->num_modules[sfp] - module;

  module_data_t *tmp_arr_module_data;
  if(file->num_modules[sfp] - num > 0)
    tmp_arr_module_data = (module_data_t*)malloc(sizeof(module_data_t) * (file->num_modules[sfp] - num));
  else
    tmp_arr_module_data = NULL;

  if(module != 0)
    memcpy(tmp_arr_module_data, file->module_data[sfp], module * sizeof(module_data_t));

  if(file->num_modules[sfp] - num - module > 0)
    memcpy(&tmp_arr_module_data[module], &file->module_data[sfp][module + num], (file->num_modules[sfp] - num - module) * sizeof(module_data_t));

  free(file->module_data[sfp]);
  file->module_data[sfp] = tmp_arr_module_data;
  file->num_modules[sfp] -= num;

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

int32_t *module_data_get(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, char *name, int32_t *val_min, int64_t *val_max, conf_value_def_t **def)
{
  if(sfp >= file->num_sfp)
    return NULL;

  if(module >= file->num_modules[sfp])
    return NULL;

  conf_value_data_t *d;

  if(channel < 0)
  {
    d = config_array_search(file->module_data[sfp][module].arr_global_cfg,
        file->module_data[sfp][module].firmware->num_global_config_vars, name);
  }
  else if(channel >= file->module_data[sfp][module].firmware->num_channels)
  {
    return NULL;
  }
  else
  {
    d = config_array_search(file->module_data[sfp][module].arr_channel_cfg[channel],
        file->module_data[sfp][module].firmware->num_channel_config_vars, name);
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

