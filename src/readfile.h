#ifndef _READFILE_H_
#define _READFILE_H_

#include <stdint.h>

#include "data.h"

#define FILEVERSION_RECENT 1

int readfile(file_data_t *file);
void fill_data_from_file(file_data_t *file);

void fill_regdata_from_module_data(file_data_t *file);

int write_file(file_data_t *file);

#endif

