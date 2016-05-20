#ifndef _HOOKS_FEBEX_1_3_H
#define _HOOKS_FEBEX_1_3_H

#include <stdint.h>
#include "../../data.h"

int hook_febex_13_set_bl_integration_time(file_data_t *file, uint8_t sfp, uint8_t module, int8_t channel, const char *varname, int32_t *val);

#endif

