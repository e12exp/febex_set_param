#ifndef _FILESTRUCT_H_
#define _FILESTRUCT_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

typedef struct
{
  uint8_t block_size;
  uint8_t sfp;
  uint8_t module;
  uint32_t base_addr;

  uint32_t *reg_data;  
}
regblock_t;

typedef struct s_regblock_list
{
  regblock_t data;

  struct s_regblock_list *next;
}
regblock_list_t;

typedef struct
{
  regblock_list_t *first;
  regblock_list_t *current;
  regblock_list_t *last;
}
regblock_list_ptr_t;

void regblock_list_init(regblock_list_ptr_t *list);
void regblock_list_add(regblock_list_ptr_t *list, uint8_t block_size, uint8_t sfp, uint8_t module, uint32_t base_addr, uint32_t *data);

regblock_t *regblock_list_first(regblock_list_ptr_t *list);
regblock_t *regblock_list_next(regblock_list_ptr_t *list);

void regblock_list_clear(regblock_list_ptr_t *list);

int search_register_data(regblock_list_ptr_t *list, uint8_t sfp, uint8_t module, uint32_t addr, uint32_t *val);

uint32_t *get_register(regblock_list_ptr_t *list, uint8_t sfp, uint8_t module, uint32_t addr);

#endif

