#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../command.h"
#include "../consoleinterface.h"

IMPL(display)
{
  ARGS_INIT;
  ARG_STR(level);

  if(strlen(level) == 0)
  {
    printf("Available display levels:\n"
        " beginner\n"
        " user\n"
        " expert\n");
    return 1;
  }

  if(strcmp(level, "debug") == 0)
    g_display_level = hidden;

  else if(strcmp(level, "expert") == 0)
    g_display_level = expert;

  else if(strcmp(level, "user") == 0)
    g_display_level = user;

  else if(strcmp(level, "beginner") == 0)
    g_display_level = beginner;

  else
  {
    fprintf(stderr, "Invalid display level\n");
    return 0;
  }

  return 1;
}
