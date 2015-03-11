#include <stdlib.h>
#include <string.h>

#include "paramdef.h"

uint16_t g_num_firmwares;

firmware_list_t *g_fw_list_first;
firmware_list_t *g_fw_list_current;

#define DEF_FIRMWARE(_id, _name, _description, _fw_recommended, _fw_min, _fw_max) \
  fw_l = (firmware_list_t*)malloc(sizeof(firmware_list_t)); \
  fw_l->fw.id = _id; \
  fw_l->fw.name = _name; \
  fw_l->fw.description = _description; \
  fw_l->fw.fw_min = _fw_min; \
  fw_l->fw.fw_max = _fw_max; \
  fw_l->fw.fw_recommended = _fw_recommended; \
  fw_l->fw.num_global_config_vars = 0; \
  fw_l->fw.num_channel_config_vars = 0; \
  fw_list_add(fw_l);

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
  conf_list_add(l, &g_fw_list_current->fw);

#define DEF_VAR_INT(_name, _global, _offset, _low, _high, _shift) \
  DEF_VAR(_name, conf_type_int, _global, c_addr, _offset, _low, _high, _shift, 0)

#define DEF_VAR_INT_SIGNED(_name, _global, _offset, _low, _high, _shift) \
  DEF_VAR(_name, conf_type_int, _global, c_addr, _offset, _low, _high, _shift, 1)

#define DEF_VAR_BOOL(_name, _global, _offset, _low, _shift) \
  DEF_VAR_INT(_name, _global, _offset, _low, _low, _shift)

#define DEF_VAR_MASK(_name, _global, _offset, _low, _high, _shift) \
   DEF_VAR(_name, conf_type_mask, _global, c_addr, _offset, _low, _high, _shift, 0)

#define NEXT_REG c_addr += 4;

void fw_list_add(firmware_list_t *l)
{
  l->next = NULL;

  if(g_fw_list_current == NULL)
    g_fw_list_first = g_fw_list_current = l;
  else
  {
    g_fw_list_current->next = l;
    g_fw_list_current = l;
  }
}

void conf_list_add(conf_list_t *l, firmware_def_t *fw)
{
  l->next = NULL;

  if(fw->conf_list_current == NULL)
  {
    fw->conf_list_first = fw->conf_list_current = l;
  }
  else
  {
    fw->conf_list_current->next = l;
    fw->conf_list_current = l;
  }

  if(l->v.global == 1)
    fw->num_global_config_vars++;
  else
    fw->num_channel_config_vars++;
}

void register_vars()
{
  uint32_t c_addr;
  conf_list_t *l;
  firmware_list_t *fw_l;

  g_fw_list_first = g_fw_list_current = NULL;

  #include "fw/febex_1.3.def"
  #include "fw/pulser_2.0.def"

}

conf_value_def_t *conf_list_first(firmware_def_t *fw)
{
  if(!fw->conf_list_first)
    return NULL;

  fw->conf_list_current = fw->conf_list_first;

  return &(fw->conf_list_first->v);
}

conf_value_def_t *conf_list_next(firmware_def_t *fw)
{
  if(!fw->conf_list_current)
    return NULL;

  fw->conf_list_current = fw->conf_list_current->next;

  if(!fw->conf_list_current)
    return NULL;

  return &(fw->conf_list_current->v);  
}

firmware_def_t *fw_list_first()
{
  if(!g_fw_list_first)
    return NULL;

  g_fw_list_current = g_fw_list_first;

  return &g_fw_list_first->fw;
}

firmware_def_t *fw_list_next()
{
  if(!g_fw_list_current)
    return NULL;

  if(!(g_fw_list_current = g_fw_list_current->next))
    return NULL;

  return &g_fw_list_current->fw;
}

firmware_def_t *get_firmware_def(uint32_t id)
{
  firmware_def_t *fw;

  for(fw = fw_list_first(); fw != NULL; fw = fw_list_next())
  {
    if(fw->id == id || id == 0) // FW ID = 0 -> Pick default firmware (first one)
      return fw;
  }

  return NULL;
}

uint32_t get_firmware_id(const char *name)
{
  firmware_def_t *fw;

  for(fw = fw_list_first(); fw != NULL; fw = fw_list_next())
  {
    if(strcmp(name, fw->name) == 0 || strcmp(name, "default") == 0)
      return fw->id;
  }

  return 0;
}

