#ifndef _DATA_H_
#define _DATA_H_

#include <stdint.h>

#include "paramdef.h"
#include "filestruct.h"

typedef struct
{
  conf_value_def_t  *value_def;
  int32_t	    value_data;
}
conf_value_data_t;

typedef struct
{
  uint8_t sfp;
  uint8_t module;

  firmware_def_t *firmware;

  conf_value_data_t *arr_global_cfg;
  conf_value_data_t **arr_channel_cfg;
}
module_data_t;

typedef struct s_file_data
{
  module_data_t   *(module_data[4]);
  uint8_t         num_sfp;
  uint8_t         num_modules[4];

  char            *filename;
  uint8_t         version;
  uint8_t         read_only;

  regblock_list_ptr_t regdata;
}
file_data_t;

// Array of pointers to keep pointers to individual file structures
// valid even if new files are added/removed and thus the array is moved
// in memory.
extern file_data_t  **g_file_data;
extern uint8_t      g_num_files;
extern uint8_t      g_active_file;

void file_data_init();
file_data_t *file_data_add(const char *filename);
void file_data_free(file_data_t *file);

void file_data_close(uint8_t fileno);

//void module_data_init(file_data_t *file);
uint8_t module_data_add_sfp(file_data_t *file, uint8_t num);
int16_t module_data_add_module(file_data_t *file, uint8_t sfp, uint8_t num, uint32_t firmware);

uint8_t module_data_remove_module(file_data_t *file, uint8_t sfp, uint8_t module, uint8_t num);

void module_data_prepare(uint8_t sfp, uint8_t module, module_data_t *data, firmware_def_t *firmware);
void module_data_free(module_data_t *data);

int32_t *module_data_get(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, char *name, int32_t *val_min, int64_t *val_max, conf_value_def_t **def);

#endif

