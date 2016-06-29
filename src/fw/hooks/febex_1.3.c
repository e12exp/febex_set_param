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

// Check if in multi event mode the requested number of events fits to the data buffer
int hook_febex_13_check_buffer_size(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val)
{
  int32_t num_events_readout, trace_length, tot_enabled, opmode_enable_trace;
  int c;
  int32_t size_per_event, max_events;

  if(strcmp(varname, "num_events_readout") == 0)
    num_events_readout = *val;
  else
    num_events_readout = *module_data_get(file, sfp, module, -1, "num_events_readout", NULL, NULL, NULL);

  if(num_events_readout == 255)
  {
    // Assuming multi event readout -> No problem!
    return 0;
  }

  if(strcmp(varname, "trace_length") == 0)
    trace_length = *val;
  else
    trace_length = *module_data_get(file, sfp, module, -1, "trace_length", NULL, NULL, NULL);

  if(strcmp(varname, "tot_enabled") == 0)
    tot_enabled = *val;
  else
    tot_enabled = *module_data_get(file, sfp, module, -1, "tot_enabled", NULL, NULL, NULL);

  opmode_enable_trace = 0;

  for(c = 0; c < 16; c++)
  {
    if(channel == c && strcmp(varname, "opmode_enable_trace") == 0)
      opmode_enable_trace |= *val;
    else
      opmode_enable_trace |= *module_data_get(file, sfp, module, c, "opmode_enable_trace", NULL, NULL, NULL);
  }

  size_per_event = 40 + (tot_enabled ? 12 : 0)
    + (opmode_enable_trace ? 8 + 2 * trace_length : 0);
  max_events = 8*1024 / size_per_event;

  if(num_events_readout >= max_events)
  {
    fprintf(stderr, "Warning: A maximum of %d events will fit to the data buffer. Setting num_events_readout = %d\n", max_events, max_events - 1);

    if(strcmp(varname, "num_events_readout") == 0)
      *val = max_events - 1;
    else
      *module_data_get(file, sfp, module, -1, "num_events_readout", NULL, NULL, NULL) = max_events - 1;
  }

  return 0;
}
