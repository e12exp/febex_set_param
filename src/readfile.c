#include "readfile.h"
#include "filestruct.h"
#include "data.h"
#include "paramdef.h"

char *g_fname;

int readfile(char *fname)
{
  g_fname = fname;

  FILE *fconf;

  if(!(fconf = fopen(fname, "rb")))
  {
    fprintf(stderr, "Could not open file %s for reading.\n", fname);
    return 0;
  }

  uint8_t block_size;
  uint8_t sfp;
  uint8_t module;
  uint32_t base_addr;
  uint32_t *data;

  if(!fread(&sfp, 1, 1, fconf))
  {
    fprintf(stderr, "Warning: Invalid file %s.\n", fname);
    return 0;
  }

  module_data_add_sfp(sfp);

  for(sfp = 0; sfp < g_num_sfp; sfp++)
  {
    fread(&module, 1, 1, fconf);
    module_data_add_module(sfp, module);
  }

  while(!feof(fconf))
  {
    fread(&block_size, 1, 1, fconf);
    fread(&sfp, 1, 1, fconf);
    fread(&module, 1, 1, fconf);
    fread(&base_addr, 4, 1, fconf);

    data = (uint32_t*)malloc(sizeof(uint32_t) * block_size);
    fread(data, 4, block_size, fconf);

    regblock_list_add(block_size, sfp, module, base_addr, data);
  }

  fclose(fconf);

  return 1;
}

int write_file()
{
  FILE *fd;
  regblock_t *block;

  if(!(fd = fopen(g_fname, "wb")))
  {
    fprintf(stderr, "Could not open file %s for writing.\n", g_fname);
    return 0;
  }

  fwrite(&g_num_sfp, 1, 1, fd);
  fwrite(g_num_modules, 1, g_num_sfp, fd);

  block = regblock_list_first();

  while(block)
  {
  //  printf("write(%d, %d, %d, %06x): %08x\n", block->block_size, block->sfp, block->module, block->base_addr, *block->reg_data);

    fwrite(&block->block_size, 1, 1, fd);
    fwrite(&block->sfp, 1, 1, fd);
    fwrite(&block->module, 1, 1, fd);
    fwrite(&block->base_addr, 4, 1, fd);

    fwrite(block->reg_data, 4, block->block_size, fd);
    
    block = regblock_list_next();
  }

  fclose(fd);
  return 1;
}

void fill_data_from_file()
{
  uint8_t sfp, mod;
  uint32_t ngv = 0, ncv = 0, c = 0;
  uint32_t addr, lowbit;
  uint32_t dat;
  uint64_t mask;

  conf_value_def_t *v;

  for(sfp = 0; sfp < g_num_sfp; sfp++)
  {
    for(mod = 0; mod < g_num_modules[sfp]; mod++)
    {
      for(ngv = 0; ngv < g_num_global_config_vars; ngv++)
      {
	v = g_arr_module_data[sfp][mod].arr_global_cfg[ngv].value_def;

        search_register_data(sfp, mod, v->addr, &dat);
        dat = (dat & v->bitmask) >> v->lowbit;
        g_arr_module_data[sfp][mod].arr_global_cfg[ngv].value_data = dat;
      }

      for(ncv = 0; ncv < g_num_channel_config_vars; ncv++)
      {
	v = g_arr_module_data[sfp][mod].arr_channel_cfg[0][ncv].value_def;
	addr = v->addr;
	mask = v->bitmask;
	lowbit = v->lowbit;
	for(c = 0; c < 16; c++)
	{
	  search_register_data(sfp, mod, addr, &dat),
	  dat = (dat & mask) >> lowbit;
	  g_arr_module_data[sfp][mod].arr_channel_cfg[c][ncv].value_data = dat;

	  addr += v->channel_offset;
	  mask <<= v->channel_shift;
	  lowbit += v->channel_shift;
	  if(lowbit >= 32)
	  {
	    lowbit -= 32;
	    mask >>= 32;
	    addr += 4;
	  }
	}
      }
    }
  }
}

void setreg(int sfp, int mod, int channel, conf_value_def_t *def, uint32_t data)
{
//  printf("setreg(%d, %d, %d, %s, 0x%06x, %d)\n", sfp, mod, channel, def->name, def->addr, data);

  uint32_t addr = def->addr;
  uint64_t mask = def->bitmask;
  uint8_t lowbit = def->lowbit;

  uint32_t *regdat;

  addr += channel * def->channel_offset;
  lowbit += channel * def->channel_shift;
  mask <<= (channel * def->channel_shift);

  if(lowbit >= 32)
  {
    lowbit -= 32;
    mask >>= 32;
    addr += 4;
  }

//  printf("addr: 0x%06x, lowbit: %d, mask: %08x\n", addr, lowbit, (uint32_t)mask);

  regdat = get_register(sfp, mod, addr);

  if(regdat == NULL)
  {
//    printf("Add register\n");

    regdat = (uint32_t*)malloc(sizeof(uint32_t));
    *regdat = 0;
    regblock_list_add(1, sfp, mod, addr, regdat);
  }
//  else
//    printf("Register exists. Data: %08x\n", *regdat);

  *regdat |= ((data << lowbit) & (uint32_t)mask);

//  printf("Data: %08x\n", *regdat);
} 

void fill_regdata_from_module_data()
{
  int sfp, mod, c, v;

  regblock_list_clear();

  for(sfp = 0; sfp < g_num_sfp; sfp++)
  {
    for(mod = 0; mod < g_num_modules[sfp]; mod++)
    {
      for(v = 0; v < g_num_global_config_vars; v++)
      {
	setreg(sfp, mod, 0, g_arr_module_data[sfp][mod].arr_global_cfg[v].value_def,
	    g_arr_module_data[sfp][mod].arr_global_cfg[v].value_data);
      }
      for(c = 0; c < 16; c++)
      {
	for(v = 0; v < g_num_channel_config_vars; v++)
	{
	  setreg(sfp, mod, c, g_arr_module_data[sfp][mod].arr_channel_cfg[c][v].value_def,
	      g_arr_module_data[sfp][mod].arr_channel_cfg[c][v].value_data);
	}
      }
    }
  }
}

