#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "pulser_2.0.h"
#include "../../data.h"

int hook_pulser_20_set_decay_const(uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val)
{

  if(strcmp(varname, "decay_const_linear") == 0)
  {
    *(module_data_get(sfp, module, channel, "decay_const_2_l", NULL, NULL, NULL)) = *val & 0xff;
    *(module_data_get(sfp, module, channel, "decay_const_2_h", NULL, NULL, NULL)) = (*val >> 8) & 0xff;
  }
  else if(strcmp(varname, "decay_const_drop") == 0)
  {

    *(module_data_get(sfp, module, channel, "decay_const_1_l", NULL, NULL, NULL)) = *val & 0xff;
    *(module_data_get(sfp, module, channel, "decay_const_1_h", NULL, NULL, NULL)) = (*val >> 8) & 0xff;
  }

  return 1;
}

int hook_pulser_20_get_decay_const(uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val)
{

  if(strcmp(varname, "decay_const_linear") == 0)
  {
    *val = *(module_data_get(sfp, module, channel, "decay_const_2_l", NULL, NULL, NULL));
    *val |= *(module_data_get(sfp, module, channel, "decay_const_2_h", NULL, NULL, NULL)) << 8;
  }
  else if(strcmp(varname, "decay_const_drop") == 0)
  {
    *val = *(module_data_get(sfp, module, channel, "decay_const_1_l", NULL, NULL, NULL));
    *val |= *(module_data_get(sfp, module, channel, "decay_const_1_h", NULL, NULL, NULL)) << 8;
  }

  return 1;
}

int hook_pulser_20_set_duty(uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val)
{
  *(module_data_get(sfp, module, channel, "duty_0", NULL, NULL, NULL)) = *val & 0xff;
  *(module_data_get(sfp, module, channel, "duty_1", NULL, NULL, NULL)) = (*val >> 8) & 0xff;
  *(module_data_get(sfp, module, channel, "duty_2", NULL, NULL, NULL)) = (*val >> 16) & 0xff;
  *(module_data_get(sfp, module, channel, "duty_3", NULL, NULL, NULL)) = (*val >> 24) & 0xff;

  return 1;
}

int hook_pulser_20_get_duty(uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val)
{
  *val = *module_data_get(sfp, module, channel, "duty_0", NULL, NULL, NULL);
  *val |= *module_data_get(sfp, module, channel, "duty_1", NULL, NULL, NULL) << 8;
  *val |= *module_data_get(sfp, module, channel, "duty_2", NULL, NULL, NULL) << 16;
  *val |= *module_data_get(sfp, module, channel, "duty_3", NULL, NULL, NULL) << 24;

  return 1;
}

int hook_pulser_20_set_common_duty(uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val)
{
  *(module_data_get(sfp, module, channel, "common_duty_0", NULL, NULL, NULL)) = *val & 0xff;
  *(module_data_get(sfp, module, channel, "common_duty_1", NULL, NULL, NULL)) = (*val >> 8) & 0xff;
  *(module_data_get(sfp, module, channel, "common_duty_2", NULL, NULL, NULL)) = (*val >> 16) & 0xff;
  *(module_data_get(sfp, module, channel, "common_duty_3", NULL, NULL, NULL)) = (*val >> 24) & 0xff;

  return 1;
}

int hook_pulser_20_get_common_duty(uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val)
{
  *val = *module_data_get(sfp, module, channel, "common_duty_0", NULL, NULL, NULL);
  *val |= *module_data_get(sfp, module, channel, "common_duty_1", NULL, NULL, NULL) << 8;
  *val |= *module_data_get(sfp, module, channel, "common_duty_2", NULL, NULL, NULL) << 16;
  *val |= *module_data_get(sfp, module, channel, "common_duty_3", NULL, NULL, NULL) << 24;

  return 1;
}

int hook_pulser_20_set_delay_const(uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val)
{
  *(module_data_get(sfp, module, channel, "delay_const_1", NULL, NULL, NULL)) = *val & 0xff;
  *(module_data_get(sfp, module, channel, "delay_const_2", NULL, NULL, NULL)) = (*val >> 8) & 0xff;

  return 1;
}

int hook_pulser_20_get_delay_const(uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val)
{
  *val = *module_data_get(sfp, module, channel, "delay_const_1", NULL, NULL, NULL);
  *val |= *module_data_get(sfp, module, channel, "delay_const_2", NULL, NULL, NULL) << 8;

  return 1;
}

int hook_pulser_20_set_delay_min(uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val)
{
  *(module_data_get(sfp, module, channel, "delay_min_1", NULL, NULL, NULL)) = *val & 0xff;
  *(module_data_get(sfp, module, channel, "delay_min_2", NULL, NULL, NULL)) = (*val >> 8) & 0xff;

  return 1;
}

int hook_pulser_20_get_delay_min(uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val)
{
  *val = *module_data_get(sfp, module, channel, "delay_min_1", NULL, NULL, NULL);
  *val |= *module_data_get(sfp, module, channel, "delay_min_2", NULL, NULL, NULL) << 8;

  return 1;
}

int hook_pulser_20_set_delay_max(uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val)
{
  *(module_data_get(sfp, module, channel, "delay_max_1", NULL, NULL, NULL)) = *val & 0xff;
  *(module_data_get(sfp, module, channel, "delay_max_2", NULL, NULL, NULL)) = (*val >> 8) & 0xff;

  return 1;
}

int hook_pulser_20_get_delay_max(uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val)
{
  *val = *module_data_get(sfp, module, channel, "delay_max_1", NULL, NULL, NULL);
  *val |= *module_data_get(sfp, module, channel, "delay_max_2", NULL, NULL, NULL) << 8;

  return 1;
}
