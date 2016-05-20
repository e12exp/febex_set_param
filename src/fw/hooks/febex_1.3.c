#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include "febex_1.3.h"

int hook_febex_13_set_bl_integration_time(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val)
{
  double len = (*val)+1;
  double log2len = log2(len);

  if(floor(log2len) != log2len)
  {
    *val = (int32_t)pow(2., floor(log2len)) - 1;
    fprintf(stderr, "Warning: bl_integration_time+1 is not a power of two. Adjusted to %d\n",
        *val);
  }

  // Adjust bitshift accordingly
  *(module_data_get(file, sfp, module, channel, "bl_shift_right", NULL, NULL, NULL)) = (int32_t)floor(log2len);

  return 0;
}

