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
    char *strlevel;
    switch(g_display_level)
    {
	case beginner:
		strlevel = "beginner";
		break;
	case user:
		strlevel = "user";
		break;
	case expert:
		strlevel = "expert";
		break;
	case hidden:
		strlevel = "debug";
		break;
	default:
		strlevel = "UNKNOWN";
		break;
    }

    printf("Current display level: %s\nAvailable display levels:\n"
        " beginner\n"
        " user\n"
        " expert\n", strlevel);
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
