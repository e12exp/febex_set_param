#ifndef _HOOKS_PULSER_2_0_H_
#define _HOOKS_PULSER_2_0_H_

#include <stdint.h>

#include "../../data.h"

int hook_pulser_20_set_decay_const(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);
int hook_pulser_20_get_decay_const(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);

int hook_pulser_20_set_duty(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);
int hook_pulser_20_get_duty(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);

int hook_pulser_20_set_common_duty(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);
int hook_pulser_20_get_common_duty(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);

int hook_pulser_20_set_delay_const(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);
int hook_pulser_20_get_delay_const(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);

int hook_pulser_20_set_delay_min(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);
int hook_pulser_20_get_delay_min(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);

int hook_pulser_20_set_delay_max(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);
int hook_pulser_20_get_delay_max(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);

int hook_pulser_20_set_square_height(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);
int hook_pulser_20_get_square_height(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);

#endif
