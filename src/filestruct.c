#include "filestruct.h"

regblock_list_t *g_regblock_first;
regblock_list_t *g_regblock_last;
regblock_list_t *g_regblock_current;

void regblock_list_init()
{
  g_regblock_first = NULL;
  g_regblock_last = NULL;
  g_regblock_current = NULL;
}

void regblock_list_add(uint8_t block_size, uint8_t sfp, uint8_t module, uint32_t base_addr, uint32_t *data)
{
//  printf("add(%d, %d, %d, 0x%06x)\n", block_size, sfp, module, base_addr);

  regblock_list_t *elem = (regblock_list_t*)malloc(sizeof(regblock_list_t));

  elem->data.block_size = block_size;
  elem->data.sfp = sfp;
  elem->data.module = module;
  elem->data.base_addr = base_addr;
  elem->data.reg_data = data;

  elem->next = NULL;

  if(g_regblock_first == NULL)
  {
    g_regblock_first = g_regblock_current = g_regblock_last = elem;
  }
  else
  {
    g_regblock_last->next = elem;
    g_regblock_current = g_regblock_last = elem;
  }
}

regblock_t *regblock_list_first()
{
  if(!g_regblock_first)
    return NULL;
 
  g_regblock_current = g_regblock_first; 
  return &g_regblock_first->data;
}

regblock_t *regblock_list_next()
{
  if(!g_regblock_current)
    return NULL;

  g_regblock_current = g_regblock_current->next;

  if(!g_regblock_current)
    return NULL;

  return &g_regblock_current->data;
}

int search_register_data(uint8_t sfp, uint8_t module, uint32_t addr, uint32_t *val)
{
  *val = 0;

  uint32_t *v = get_register(sfp, module, addr);

  if(v == NULL)
    return 0;

  *val = *v;
   return 1; 
}

uint32_t *get_register(uint8_t sfp, uint8_t module, uint32_t addr)
{
  uint8_t offset;
  regblock_t *data = regblock_list_first();

  while(data != NULL)
  {
    if(data->sfp == sfp && data->module == module && addr >= data->base_addr && addr < data->base_addr + 4*data->block_size)
    {
      offset = (addr - data->base_addr) / 4;
      return &data->reg_data[offset];
    } 

    data = regblock_list_next();
  }

  return NULL;
}

void regblock_list_clear()
{
  regblock_list_t *e;
  g_regblock_current = g_regblock_first;
  
  while(g_regblock_current)
  {
    e = g_regblock_current;
    free(e->data.reg_data);
    e = e->next;
    free(g_regblock_current);
    g_regblock_current = e;
  }

  g_regblock_current = g_regblock_first = g_regblock_last = NULL;
}

