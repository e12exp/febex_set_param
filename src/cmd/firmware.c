#include <stdio.h>
#include <stddef.h>

#include "../command.h"
#include "../paramdef.h"

IMPLS(firmware, list)
{
  firmware_def_t *fw;

  printf("Available firmwares:\n\n");
  for(fw = fw_list_first(); fw != NULL; fw = fw_list_next())
  {
    printf("%-16s (0x%08x - 0x%08x, Recommended: 0x%08x)\n", fw->name, fw->fw_min, fw->fw_max, fw->fw_recommended);
    printf("  %s\n\n", fw->description);
  }

  return 1;
}
