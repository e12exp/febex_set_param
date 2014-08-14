#include "../readfile.h" 

IMPL(save)
{
  fill_regdata_from_module_data();
  int r=write_file();
  if(r)
    printf("Data written.\n");

  return r;
}

