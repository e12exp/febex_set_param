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

void regblock_list_init();
void regblock_list_add(uint8_t block_size, uint8_t sfp, uint8_t module, uint32_t base_addr, uint32_t *data);

regblock_t *regblock_list_first();
regblock_t *regblock_list_next();

void regblock_list_clear();

int search_register_data(uint8_t sfp, uint8_t module, uint32_t addr, uint32_t *val);

uint32_t *get_register(uint8_t sfp, uint8_t module, uint32_t addr);

#endif

