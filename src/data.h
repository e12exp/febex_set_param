#ifndef _DATA_H_
#define _DATA_H_

#include <stdint.h>

#include "paramdef.h"

typedef struct
{
  conf_value_def_t  *value_def;
  uint32_t	    value_data;
}
conf_value_data_t;

typedef struct
{
  uint8_t sfp;
  uint8_t module;

  conf_value_data_t *arr_global_cfg;
  conf_value_data_t **arr_channel_cfg;
}
module_data_t;

extern module_data_t **g_arr_module_data;
extern uint8_t g_num_sfp;
extern uint8_t *g_num_modules;

void module_data_init();
uint8_t module_data_add_sfp(uint8_t num);
uint8_t module_data_add_module(uint8_t sfp, uint8_t num);

uint8_t module_data_remove_module(uint8_t sfp, uint8_t module, uint8_t num);

void module_data_prepare(uint8_t sfp, uint8_t module, module_data_t *data);

uint32_t *module_data_get(uint8_t sfp, uint8_t module, int8_t channel, char *name, uint32_t *val_max);

#endif

