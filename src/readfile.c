#include "readfile.h"
#include "filestruct.h"
#include "data.h"
#include "paramdef.h"
#include "consoleinterface.h"
int readfile(file_data_t *file)
{
  FILE *fconf;

  if(!(fconf = fopen(file->filename, "rb")))
  {
    fprintf(stderr, "Could not open file %s for reading.\n", file->filename);
    file->version = FILEVERSION_RECENT;
    return 0;
  }

  uint8_t block_size;
  uint8_t sfp;
  uint8_t module, m;
  uint32_t base_addr;
  uint32_t fw;
  uint32_t *data;

  if(!fread(&sfp, 1, 1, fconf))
  {
    fprintf(stderr, "Warning: Invalid/empty file %s.\n", file->filename);
    return 0;
  }

  if(sfp == 0xff)
  {
    // New file structure -> Read file version
    if(!fread(&file->version, 1, 1, fconf))
    {
      fprintf(stderr, "Warning: Invalid file %s: Could not read file version.\n", file->filename);
      return 0;
    }

    switch(file->version)
    {
      case 1:
        // Real number of SFPs
        if(!fread(&sfp, 1, 1, fconf))
        {
          fprintf(stderr, "Warning: Invalid file %s: Could not read number of SFPs.\n", file->filename);
          return 0;
        }
        break;

      default:
        fprintf(stderr, "Invalid file version for file %s: %d\n", file->filename, file->version);
        return 0;
    }
  }
  else
  {
    // Old file structure
    file->version = 0;
  }

  if(sfp == 0 || sfp > 4)
  {
    fprintf(stderr, "Invalid number of SFPs: %d\n", sfp);
    return 0;
  }

  module_data_add_sfp(file, sfp);

  switch(file->version)
  {
    case 0:
      for(sfp = 0; sfp < file->num_sfp; sfp++)
      {
        fread(&module, 1, 1, fconf);
        module_data_add_module(file, sfp, module, 0);
      }
      break;

    case 1:
      for(sfp = 0; sfp < file->num_sfp; sfp++)
      {
        fread(&module, 1, 1, fconf);
        for(m = 0; m < module; m++)
        {
          fread(&fw, 4, 1, fconf);
          // Read (and ignore) further firmware definition for MBS
          fread(&base_addr, 4, 1, fconf);  // FW Min
          fread(&base_addr, 4, 1, fconf);  // FW Max
          fread(&base_addr, 4, 1, fconf);  // FW Recommended
          if(module_data_add_module(file, sfp, 1, fw) == -1)
            fprintf(stderr, "Could not add module %d to SFP %d\n", m, sfp);
        }
      }
      break;

      // Invalid file version have already been checked before 
  }

  regblock_list_init(&file->regdata);

  while(!feof(fconf))
  {
    fread(&block_size, 1, 1, fconf);
    fread(&sfp, 1, 1, fconf);
    fread(&module, 1, 1, fconf);
    fread(&base_addr, 4, 1, fconf);

    data = (uint32_t*)malloc(sizeof(uint32_t) * block_size);
    fread(data, 4, block_size, fconf);

    regblock_list_add(&file->regdata, block_size, sfp, module, base_addr, data);
  }

  fclose(fconf);

  if (!g_is_batch)
    fprintf(stderr, "Read configuration from file %s. File version: %d. File number: %d\n", file->filename, file->version, g_num_files-1);

  return 1;
}

int write_file(file_data_t *file)
{
  FILE *fd;
  regblock_t *block;
  uint8_t i, j;

  if(!(fd = fopen(file->filename, "wb")))
  {
    fprintf(stderr, "Could not open file %s for writing.\n", file->filename);
    return 0;
  }

  switch(file->version)
  {
    case 0:
      fwrite(&file->num_sfp, 1, 1, fd);
      fwrite(file->num_modules, 1, file->num_sfp, fd);

      break;

    case 1:
      // File header
      i = 0xff;
      fwrite(&i, 1, 1, fd);
      fwrite(&file->version, 1, 1, fd);

      // Data header, firmware information
      fwrite(&file->num_sfp, 1, 1, fd);
      for(i = 0; i < file->num_sfp; i++)
      {
        fwrite(&file->num_modules[i], 1, 1, fd);
        for(j = 0; j < file->num_modules[i]; j++)
        {
          fwrite(&file->module_data[i][j].firmware->id, 4, 1, fd);
          fwrite(&file->module_data[i][j].firmware->fw_min, 4, 1, fd);
          fwrite(&file->module_data[i][j].firmware->fw_max, 4, 1, fd);
          fwrite(&file->module_data[i][j].firmware->fw_recommended, 4, 1, fd);
        }
      }

      break;

    default:
      fprintf(stderr, "Invalid file version.\n");
      return 0;
  }


  block = regblock_list_first(&file->regdata);

  while(block)
  {
    //  printf("write(%d, %d, %d, %06x): %08x\n", block->block_size, block->sfp, block->module, block->base_addr, *block->reg_data);

    fwrite(&block->block_size, 1, 1, fd);
    fwrite(&block->sfp, 1, 1, fd);
    fwrite(&block->module, 1, 1, fd);
    fwrite(&block->base_addr, 4, 1, fd);

    fwrite(block->reg_data, 4, block->block_size, fd);

    block = regblock_list_next(&file->regdata);
  }

  fclose(fd);
  return 1;
}

void fill_data_from_file(file_data_t *file)
{
  uint8_t sfp, mod;
  uint32_t ngv = 0, ncv = 0, c = 0;
  uint32_t addr, lowbit;
  uint32_t dat;
  uint64_t mask;

  conf_value_def_t *v;

  for(sfp = 0; sfp < file->num_sfp; sfp++)
  {
    for(mod = 0; mod < file->num_modules[sfp]; mod++)
    {
      for(ngv = 0; ngv < file->module_data[sfp][mod].firmware->num_global_config_vars; ngv++)
      {
        v = file->module_data[sfp][mod].arr_global_cfg[ngv].value_def;

        search_register_data(&file->regdata, sfp, mod, v->addr, &dat);
        dat = (dat & v->bitmask) >> v->lowbit;
        // Check sign bit
        if(v->vsigned && (dat & ((v->bitmask >> (v->lowbit + 1)) + 1)) != 0)
        {
          // Signed, negative number => Fill upper bits with 1 instead of 0
          dat |= (0xFFFFFFFF & ~(v->bitmask >> v->lowbit));
        }
        file->module_data[sfp][mod].arr_global_cfg[ngv].value_data = dat;
      }

      for(ncv = 0; ncv < file->module_data[sfp][mod].firmware->num_channel_config_vars; ncv++)
      {
        v = file->module_data[sfp][mod].arr_channel_cfg[0][ncv].value_def;
        addr = v->addr;
        mask = v->bitmask;
        lowbit = v->lowbit;
        for(c = 0; c < file->module_data[sfp][mod].firmware->num_channels; c++)
        {
          search_register_data(&file->regdata, sfp, mod, addr, &dat);
          dat = (dat & mask) >> lowbit;
          // Check sign bit
          if(v->vsigned && (dat & ((v->bitmask >> (v->lowbit + 1)) + 1)) != 0)
          {
            // Signed, negative number => Fill upper bits with 1 instead of 0
            dat |= (0xFFFFFFFF & ~(v->bitmask >> v->lowbit));
          }
          file->module_data[sfp][mod].arr_channel_cfg[c][ncv].value_data = dat;

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

void setreg(regblock_list_ptr_t *list, int sfp, int mod, int channel, conf_value_def_t *def, uint32_t data)
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

  regdat = get_register(list, sfp, mod, addr);

  if(regdat == NULL)
  {
    //    printf("Add register\n");

    regdat = (uint32_t*)malloc(sizeof(uint32_t));
    *regdat = 0;
    regblock_list_add(list, 1, sfp, mod, addr, regdat);
  }
  //  else
  //    printf("Register exists. Data: %08x\n", *regdat);

  *regdat |= ((data << lowbit) & (uint32_t)mask);

  //  printf("Data: %08x\n", *regdat);
} 

void fill_regdata_from_module_data(file_data_t *file)
{
  int sfp, mod, c, v;

  regblock_list_clear(&file->regdata);

  for(sfp = 0; sfp < file->num_sfp; sfp++)
  {
    for(mod = 0; mod < file->num_modules[sfp]; mod++)
    {
      for(v = 0; v < file->module_data[sfp][mod].firmware->num_global_config_vars; v++)
      {
        setreg(&file->regdata, sfp, mod, 0, file->module_data[sfp][mod].arr_global_cfg[v].value_def,
            file->module_data[sfp][mod].arr_global_cfg[v].value_data);
      }
      for(c = 0; c < file->module_data[sfp][mod].firmware->num_channels; c++)
      {
        for(v = 0; v < file->module_data[sfp][mod].firmware->num_channel_config_vars; v++)
        {
          setreg(&file->regdata, sfp, mod, c, file->module_data[sfp][mod].arr_channel_cfg[c][v].value_def,
              file->module_data[sfp][mod].arr_channel_cfg[c][v].value_data);
        }
      }
    }
  }
}

