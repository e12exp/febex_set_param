#include <stdlib.h>
#include <stdio.h>

#include "../command.h"
#include "file.h"

IMPL(save)
{
  return cmd_impl_file_save(0, NULL);
}

IMPL_HELP(save)
{
  printf("Alias for 'file save'.\n"
      "Save the current set of configuration variables to the currently active parameter file.\n\n"
      "Note: If you exit the program without saving, all changes will be lost.\n");

  return 1;
}

