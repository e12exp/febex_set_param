#ifndef _PARAMDEF_H_
#define _PARAMDEF_H_

#include <stdint.h>

typedef enum
{
  conf_type_int = 1,
  conf_type_macro_qpid = 2,
  conf_type_mask = 3,
  conf_type_enum = 4
}
conf_type_t;

typedef enum
{
  hidden = 0,
  expert = 1,
  user = 6,
  beginner = 10
}
display_level_t;

struct s_file_data;
typedef int (*variable_callback_t)(struct s_file_data *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);

typedef struct
{
  variable_callback_t set;
  variable_callback_t get;
}
variable_hooks_t;

typedef struct s_enum_val
{
  int32_t value;
  const char *display;

  struct s_enum_val *next;
}
enum_val_t;

typedef struct
{
  char *name;
  char *description; 
  char *unit;

  conf_type_t type;

  uint8_t global;
  uint32_t addr;
  uint32_t channel_offset;

  uint32_t bitmask;
  uint8_t lowbit;
  uint32_t channel_shift;

  uint8_t vsigned;

  enum_val_t *enum_value_list;
  enum_val_t *enum_value_current;

  display_level_t display_level;

  variable_hooks_t hooks;
}
conf_value_def_t;

typedef struct s_conf_list
{
  conf_value_def_t v;

  struct s_conf_list *next;
}
conf_list_t;

typedef struct
{
  uint32_t id;
  char *name;
  char *description;

  uint32_t fw_min;
  uint32_t fw_max;
  uint32_t fw_recommended;

  uint8_t num_channels;

  uint16_t num_global_config_vars;
  uint16_t num_channel_config_vars;

  conf_list_t *conf_list_first;
  conf_list_t *conf_list_current;
}
firmware_def_t;

typedef struct s_firmware_list
{
  firmware_def_t fw;

  struct s_firmware_list *next;
}
firmware_list_t;

extern uint8_t g_num_global_config_vars;
extern uint8_t g_num_channel_config_vars;

extern conf_list_t *g_conf_list_first;
extern conf_list_t *g_conf_list_current;

conf_value_def_t *conf_list_first(firmware_def_t *fw);
conf_value_def_t *conf_list_next(firmware_def_t *fw);

firmware_def_t *fw_list_first();
firmware_def_t *fw_list_next();

firmware_def_t *get_firmware_def(uint32_t id);
uint32_t get_firmware_id(const char *name);

const char *enum_get_value_display(conf_value_def_t *vardef, int32_t val);
int enum_get_value(conf_value_def_t *vardef, const char *str, int32_t *val);

void register_vars();

#endif

