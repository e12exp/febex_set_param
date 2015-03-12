#include <stdlib.h>
#include <stdio.h>

#include "../command.h"
#include "../readfile.h" 

IMPL(save)
{
  char c;

  if(g_fileversion < FILEVERSION_RECENT)
  {
    printf("Do you want to convert database file %s from version %d to version %d?\n",
        g_fname, g_fileversion, FILEVERSION_RECENT);
    
    do
    {
      printf(" Yes (y) | No (n) ?\n");
      c = (char)getchar();
    }
    while(c != 'y' && c != 'n');

    if(c == 'y')
      g_fileversion = FILEVERSION_RECENT;
  }

  fill_regdata_from_module_data();
  int r=write_file();
  if(r)
    printf("Data written.\n");

  return r;
}

