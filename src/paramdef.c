#include <stdlib.h>

#include "paramdef.h"

uint8_t g_num_global_config_vars;
uint8_t g_num_channel_config_vars;

conf_list_t *g_conf_list_first;
conf_list_t *g_conf_list_current;

#define BASE_ADDR(addr) c_addr = addr;

#define DEF_VAR(_name, _type, _global, _addr, _offset, _low, _high, _shift, _signed) \
  l = (conf_list_t*)malloc(sizeof(conf_list_t)); \
  l->v.name = #_name; \
  l->v.type = _type; \
  l->v.global = _global; \
  l->v.addr = _addr; \
  l->v.channel_offset = _offset; \
  l->v.bitmask = (uint32_t)(((uint64_t)1 << ((uint64_t)_high + 1)) - 1); \
  l->v.bitmask &= ~(uint32_t)(((uint64_t)1 << (uint64_t)_low) - 1); \
  l->v.lowbit = _low; \
  l->v.channel_shift = _shift; \
  l->v.vsigned = _signed; \
  conf_list_add(l);

#define DEF_VAR_INT(_name, _global, _offset, _low, _high, _shift) \
  DEF_VAR(_name, conf_type_int, _global, c_addr, _offset, _low, _high, _shift, 0)

#define DEF_VAR_INT_SIGNED(_name, _global, _offset, _low, _high, _shift) \
  DEF_VAR(_name, conf_type_int, _global, c_addr, _offset, _low, _high, _shift, 1)

#define DEF_VAR_BOOL(_name, _global, _offset, _low, _shift) \
  DEF_VAR_INT(_name, _global, _offset, _low, _low, _shift)

#define DEF_VAR_MASK(_name, _global, _offset, _low, _high, _shift) \
   DEF_VAR(_name, conf_type_mask, _global, c_addr, _offset, _low, _high, _shift, 0)

#define NEXT_REG c_addr += 4;

void conf_list_add(conf_list_t *l)
{
  l->next = NULL;

  if(g_conf_list_current == NULL)
  {
    g_conf_list_first = g_conf_list_current = l;
  }
  else
  {
    g_conf_list_current->next = l;
    g_conf_list_current = l;
  }

  if(l->v.global == 1)
    g_num_global_config_vars++;
  else
    g_num_channel_config_vars++;
}

void register_vars()
{
  uint32_t c_addr;
  conf_list_t *l;

  g_conf_list_first = g_conf_list_current = NULL;

  g_num_global_config_vars = 0;
  g_num_channel_config_vars = 0;

  #include "param.def"

}

conf_value_def_t *conf_list_first()
{
  if(!g_conf_list_first)
    return NULL;

  g_conf_list_current = g_conf_list_first;

  return &g_conf_list_first->v;
}

conf_value_def_t *conf_list_next()
{
  if(!g_conf_list_current)
    return NULL;

  g_conf_list_current = g_conf_list_current->next;

  if(!g_conf_list_current)
    return NULL;

  return &g_conf_list_current->v;  
}

