#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../command.h"
#include "../consoleinterface.h"

IMPL(display)
{
//  ARGS_INIT;
//  ARG_STR(level);

  char *level = NULL;
  if(argc != 0)
    level = argv[0];

  if(level == NULL || strlen(level) == 0)
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

IMPL_HELP(display)
{
  printf("Get or set the current display level.\n"
      "Depending on the display level, the \"list\" command will omit possibly confusing "
      "parameters and only show, what the programmer thinks is reasonable for you to change.\n\n"
      "  level: Set the display level to this value.\n\n"
      "For your convenience, here is the output of the bare \"display\" command:\n\n");
  return cmd_impl_display(0, NULL);
}

