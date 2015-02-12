#include <stdio.h>
#include <stdlib.h>

#include "binaryval.h"

void print_binary_val(uint32_t val, uint32_t mask)
{
   int32_t i;

   for(i = 31; i >= 0; i--)
   {
      if((mask & (1 << i)) == 0)
         continue;

      printf("%d", (val & (1 << i)) >> i);
   }
   printf("\n");
}
