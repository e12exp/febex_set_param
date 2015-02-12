#ifndef _PARAMDEF_H_
#define _PARAMDEF_H_

#include <stdint.h>

typedef enum
{
  conf_type_int = 1,
  conf_type_macro_qpid = 2,
  conf_type_mask = 3
}
conf_type_t;

typedef struct
{
  char *name;

  conf_type_t type;

  uint8_t global;
  uint32_t addr;
  uint32_t channel_offset;

  uint32_t bitmask;
  uint8_t lowbit;
  uint32_t channel_shift;

  uint8_t vsigned;
}
conf_value_def_t;

typedef struct s_conf_list
{
  conf_value_def_t v;

  struct s_conf_list *next;
}
conf_list_t;

extern uint8_t g_num_global_config_vars;
extern uint8_t g_num_channel_config_vars;

extern conf_list_t *g_conf_list_first;
extern conf_list_t *g_conf_list_current;

conf_value_def_t *conf_list_first();
conf_value_def_t *conf_list_next();

void register_vars();

#endif

