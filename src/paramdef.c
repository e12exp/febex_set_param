#include <stdlib.h>
#include <string.h>

#include "paramdef.h"
#include "data.h"
#include "assert.h"

uint16_t g_num_firmwares;

firmware_list_t *g_fw_list_first;
firmware_list_t *g_fw_list_current;

#define DEF_FIRMWARE(_id, _name, _description, _channels, _fw_recommended, _fw_min, _fw_max) \
  fw_l = (firmware_list_t*)malloc(sizeof(firmware_list_t));		\
  fw_l->fw.id = _id;							\
  fw_l->fw.name = _name;						\
  fw_l->fw.description = _description;					\
  fw_l->fw.fw_min = _fw_min;						\
  fw_l->fw.fw_max = _fw_max;						\
  fw_l->fw.fw_recommended = _fw_recommended;				\
  fw_l->fw.num_channels = _channels;					\
  fw_l->fw.num_global_config_vars = 0;					\
  fw_l->fw.num_channel_config_vars = 0;					\
  fw_l->fw.conf_list_current = NULL;					\
  memset(fw_l->fw.categories, 0, sizeof(fw_l->fw.categories));		\
  fw_list_add(fw_l);							\
  ;

#define BASE_ADDR(addr) c_addr = addr;

#define DEF_VAR(_name, _type, _global, _addr, _offset, _low, _high, _shift, _signed) \
  l = (conf_list_t*)calloc(1, sizeof(conf_list_t));			\
  l->v.name = #_name;							\
  l->v.type = _type;							\
  l->v.global = _global;						\
  l->v.addr = _addr;							\
  l->v.channel_offset = _offset;					\
  l->v.bitmask = (uint32_t)(((uint64_t)1 << ((uint64_t)_high + 1)) - 1); \
  l->v.bitmask &= ~(uint32_t)(((uint64_t)1 << (uint64_t)_low) - 1);	\
  l->v.lowbit = _low;							\
  l->v.channel_shift = _shift;						\
  l->v.vsigned = _signed;						\
  l->v.display_level = fw_default_display_level;			\
  l->v.description = NULL;						\
  l->v.unit = NULL;							\
  conf_list_add(l, &g_fw_list_current->fw);				\
  ;

#define DEF_VAR_INT(_name, _global, _offset, _low, _high, _shift) \
  DEF_VAR(_name, conf_type_int, _global, c_addr, _offset, _low, _high, _shift, 0)

#define DEF_VAR_INT_SIGNED(_name, _global, _offset, _low, _high, _shift) \
  DEF_VAR(_name, conf_type_int, _global, c_addr, _offset, _low, _high, _shift, 1)

#define DEF_VAR_BOOL(_name, _global, _offset, _low, _shift) \
  DEF_VAR_INT(_name, _global, _offset, _low, _low, _shift)

#define DEF_VAR_MASK(_name, _global, _offset, _low, _high, _shift) \
   DEF_VAR(_name, conf_type_mask, _global, c_addr, _offset, _low, _high, _shift, 0)

#define DEF_VAR_ENUM(_name, _global, _offset, _low, _high, _shift) \
  DEF_VAR(_name, conf_type_enum, _global, c_addr, _offset, _low, _high, _shift, 0)

#define ENUM_VAL(_val, _display) enum_value_list_add(_val, #_display, &l->v);

// Define description for parameter
#define DESCR(_descr) l->v.description = _descr;

#define DECLCAT(_category, _section, _name)				\
  category_t* _category=((category_t*) malloc(sizeof(category_t)));	\
  _category->name=_name;							\
  memset(_category->params, 0, sizeof(_category->params));		\
  assert(_section<NUMCATEGORIES);					\
  assert(g_fw_list_current->fw.categories[_section]==0);		\
  g_fw_list_current->fw.categories[_section]=_category;			\
  ;

// Define category for parameter
#define CAT(_category, _subsection) {int i=_subsection; while(i<PARAMSPERCATEGORY && _category->params[i]!=0) i++; assert(i<PARAMSPERCATEGORY); _category->params[i]=&(l->v);}


// Define unit for parameter (reserved for future use)
#define UNIT(_unit) l->v.unit = _unit;

#define DISPLAY_HIDDEN l->v.display_level = hidden;
#define DISPLAY_EXPERT l->v.display_level = expert;
#define DISPLAY_USER l->v.display_level = user;
#define DISPLAY_BEGINNER l->v.display_level = beginner;

#define HOOK_SET(_callback) l->v.hooks.set = _callback;
#define HOOK_GET(_callback) l->v.hooks.get = _callback;

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

  if(l->v.global)
    fw->num_global_config_vars++;
  else
    fw->num_channel_config_vars++;
}

void enum_value_list_add(uint32_t value, const char *display, conf_value_def_t *vardef)
{
  if(vardef->enum_value_list == NULL)
    vardef->enum_value_list = vardef->enum_value_current = (enum_val_t*)calloc(1, sizeof(enum_val_t));
  else
    vardef->enum_value_current = vardef->enum_value_current->next = (enum_val_t*)calloc(1, sizeof(enum_val_t));

  vardef->enum_value_current->value = value;
  vardef->enum_value_current->display = display;
}

void register_vars()
{
  uint32_t c_addr;
  conf_list_t *l;
  firmware_list_t *fw_l;

  g_fw_list_first = g_fw_list_current = NULL;

  display_level_t fw_default_display_level = expert;

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

const char *enum_get_value_display(conf_value_def_t *vardef, int32_t val)
{
  for(vardef->enum_value_current = vardef->enum_value_list; vardef->enum_value_current != NULL; vardef->enum_value_current = vardef->enum_value_current->next)
  {
    if(vardef->enum_value_current->value == val)
      return vardef->enum_value_current->display;
  }

  return "-invalid-";
}

int enum_get_value(conf_value_def_t *vardef, const char *str, int32_t *val)
{
  for(vardef->enum_value_current = vardef->enum_value_list; vardef->enum_value_current != NULL; vardef->enum_value_current = vardef->enum_value_current->next)
  {
    if(strcmp(str, vardef->enum_value_current->display) == 0)
    {
      *val = vardef->enum_value_current->value;
      return 1;
    }
  }

  return 0;
}

