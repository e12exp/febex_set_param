
#undef NDEBUG //always execute argument of assert macros
#include "assert.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>


#include "../command.h"
#include "../data.h"
#include "../consoleinterface.h"
#include "../paramdef.h"
#include "../binaryval.h"
#include "batchfile.h"
#include <limits>
#include <algorithm>
#include <type_traits>
#include <cctype> //tolower
#include <string.h>
#include <sys/wait.h>


#include "../data.h"
#include "../readfile.h"

#define PREPROCESS


IMPL(batchfile)
{
  ARGS_INIT
  ARG_STR(filename);

  FILE* cmdfile;
  int pid=0;

#ifndef PREPROCESS // no preprocessing
  if (strcmp(filename, "-"))
    {
      cmdfile=fopen(filename, "r");
      assert(cmdfile);
    }
  else
    {
      cmdfile=stdin;
    }
  fprintf(stderr, "Reading commands from %s.\n", (cmdfile==stdin)?"stdin":filename);
#else // PREPROCESS true

  int pipefd[2]; //0: read, 1: write
  assert(!pipe(pipefd)); // note: we force assert statements to be executed.
  
  assert((pid=fork())!=-1);
  if (!pid) //child
    {
      close(pipefd[0]);
      assert(dup2(pipefd[1], STDOUT_FILENO)!=-1);
      char* cmd[]={"/usr/bin/env", "gcc", "-E", "-x", "c-header", filename, NULL};
      assert(execv(cmd[0], cmd)!=-1);
      exit(-1);
    }
  assert(cmdfile=fdopen(pipefd[0], "r"));
  close(pipefd[1]);
  fprintf(stderr, "Reading commands from gcc -E pipe output.\n");

#endif // preprocessing
  
  int res=eval_print_loop(cmdfile);
  if (pid)
    {
      int status;
      assert(waitpid(pid, &status, 0)!=-1);
      if (!WIFEXITED(status) || WEXITSTATUS(status))
	{
	  fprintf(stderr, "Looks like preprocessing the command file failed. I will not write anything.\n");
	  exit(2);
	}
    }

  if (!res)
    {
      assert(g_active_file>=0);
      file_data_t *file = g_file_data[g_active_file];
      fill_regdata_from_module_data(file);
      int r=write_file(file);
      if (!r)
	{
	  fprintf(stderr, "FATAL: Could not save file.\n");
	}
      else
	{
	  fprintf(stderr, "File written.\n"); 
	}
      exit(!r);
    }
  else
    {
      fprintf(stderr, "A command failed. Changes will be discarded.\n"); 
      exit(1);
    }
  return -1; //unreachable.
}


IMPL_HELP(batchfile)
{
  printf("Runs commands from filename. On success, saves file and exits setpar with 0.\n"
	 "If any command fails, execution is stopped, nothing is written and setpar returns 1.\n"
	 "Calling the batchfile command from within a batchfile probably won't work\n");
  return 1;
}

