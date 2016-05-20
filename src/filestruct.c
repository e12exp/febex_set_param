#include "filestruct.h"

void regblock_list_init(regblock_list_ptr_t *list)
{
  list->first = NULL;
  list->last = NULL;
  list->current = NULL;
}

void regblock_list_add(regblock_list_ptr_t *list, uint8_t block_size, uint8_t sfp, uint8_t module, uint32_t base_addr, uint32_t *data)
{
//  printf("add(%d, %d, %d, 0x%06x)\n", block_size, sfp, module, base_addr);

  regblock_list_t *elem = (regblock_list_t*)malloc(sizeof(regblock_list_t));

  elem->data.block_size = block_size;
  elem->data.sfp = sfp;
  elem->data.module = module;
  elem->data.base_addr = base_addr;
  elem->data.reg_data = data;

  elem->next = NULL;

  if(list->first == NULL)
  {
    list->first = list->current = list->last = elem;
  }
  else
  {
    list->last->next = elem;
    list->current = list->last = elem;
  }
}

regblock_t *regblock_list_first(regblock_list_ptr_t *list)
{
  if(!list->first)
    return NULL;
 
  list->current = list->first; 
  return &list->first->data;
}

regblock_t *regblock_list_next(regblock_list_ptr_t *list)
{
  if(!list->current)
    return NULL;

  list->current = list->current->next;

  if(!list->current)
    return NULL;

  return &list->current->data;
}

int search_register_data(regblock_list_ptr_t *list, uint8_t sfp, uint8_t module, uint32_t addr, uint32_t *val)
{
  *val = 0;

  uint32_t *v = get_register(list, sfp, module, addr);

  if(v == NULL)
    return 0;

  *val = *v;
   return 1; 
}

uint32_t *get_register(regblock_list_ptr_t *list, uint8_t sfp, uint8_t module, uint32_t addr)
{
  uint8_t offset;
  regblock_t *data = regblock_list_first(list);

  while(data != NULL)
  {
    if(data->sfp == sfp && data->module == module && addr >= data->base_addr && addr < data->base_addr + 4*data->block_size)
    {
      offset = (addr - data->base_addr) / 4;
      return &data->reg_data[offset];
    } 

    data = regblock_list_next(list);
  }

  return NULL;
}

void regblock_list_clear(regblock_list_ptr_t *list)
{
  regblock_list_t *e;
  list->current = list->first;
  
  while(list->current)
  {
    e = list->current;
    free(e->data.reg_data);
    e = e->next;
    free(list->current);
    list->current = e;
  }

  list->current = list->first = list->last = NULL;
}

