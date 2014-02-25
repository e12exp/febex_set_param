#include "../readfile.h" 

IMPL(save)
{
  fill_regdata_from_module_data();
  write_file();

  printf("Data written.\n");

  return 1;
}

