#ifndef _HOOKS_PULSER_2_0_H_
#define _HOOKS_PULSER_2_0_H_

#include <stdint.h>

int hook_pulser_20_set_decay_const(uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);
int hook_pulser_20_get_decay_const(uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);

int hook_pulser_20_set_duty(uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);
int hook_pulser_20_get_duty(uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);

#endif
