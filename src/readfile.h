#ifndef _READFILE_H_
#define _READFILE_H_

#include <stdint.h>

#define FILEVERSION_RECENT 1

extern char *g_fname;
extern uint8_t g_fileversion;

int readfile();
void fill_data_from_file();

void fill_regdata_from_module_data();

int write_file();

#endif

