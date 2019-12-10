#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //isatty
#include <assert.h>

#include "paramdef.h"
#include "filestruct.h"
#include "readfile.h"
#include "data.h"
#include "consoleinterface.h"
#include "command.h"

int main(int argc, char **argv)
{
  int empty=0;
  char *progname=argv[0];
  if (argc>1 && !strcmp(argv[1], "--empty" ))
    {
      // for batch mode:
      // unlink the file, so the user can start with a clean slate
      empty=1;
      argc--;
      argv++;
    }

  if(argc < 2)
  {
    fprintf(stderr, "Usage: %s [--empty] file [cmd [cmd args]]\n", progname);
    return -1;
  }
  if (empty)
    {
      fprintf(stderr, "Removing %s, if existing. ", argv[1]);
      unlink(argv[1]);
    }


  file_data_init();
  register_vars();
  register_commands();
  g_display_level = expert; // If someone only wanted to change trigger threshold, xe would use DUCK. 
  g_is_batch=!isatty(fileno(stdin)) || argc > 2;

  file_data_t *file = file_data_add(argv[1]);

  if(readfile(file))
  {
    fill_data_from_file(file);
  }
  else
    fprintf(stderr, "Creating new database.\n");

  if (argc > 2) // batch mode
  {
    // I (MW) think, in batch mode, the "expert" display level should be fine
    g_display_level = expert;

    //interpret command returns 1 on success, 2 on error
    //and 0 on quit
    int res=interpret_command(argv[2], argc-3, &(argv[3]));
    if (res == 1
        && strcmp("get", argv[2]) 
        && strcmp("list", argv[2])
        && strcmp("help", argv[2])
       )
    {
      fill_regdata_from_module_data(file);
      res=write_file(file);
    }
    return res != 1;
  }

  print_num_modules();

  eval_print_loop(NULL);

  printf("Bye!\n");

  return 0;  
}

