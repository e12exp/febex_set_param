#include <stdio.h>
#include <stdlib.h>

#include "binaryval.h"

void print_binary_val(uint32_t val0, uint32_t val1, uint32_t mask)
{
   int32_t i;

   for(i = 31; i >= 0; i--)
   {
      if((mask & (1 << i)) == 0)
	  continue;

      if ( ((val0 & (1 << i)) >> i) == ((val1 & (1 << i)) >> i) )
	printf("%d", (val0 & (1 << i)) >> i);
      else
	printf("?");
   }
}
