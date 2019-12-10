#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "consoleinterface.h"
#include "data.h"
#include "command.h"

display_level_t g_display_level;
bool g_is_batch;
void print_num_modules()
{
  uint8_t sfp;

  printf("Active file: %d (%s)\n", g_active_file, g_file_data[g_active_file]->filename);
  printf("# SFPs: %d\n", g_file_data[g_active_file]->num_sfp);
  for(sfp = 0; sfp < g_file_data[g_active_file]->num_sfp; sfp++)
  {
    printf("# modules on SFP %d: %d\n", sfp, g_file_data[g_active_file]->num_modules[sfp]);
  }
}

void load_history()
{
  static uint8_t hist_loaded = 0;

  FILE *fdhist;
  char buf[512];
  int l;

  if(hist_loaded == 1)
    return;

  if((fdhist = fopen(".setpar.hist", "r")))
  {
    while(!feof(fdhist))
    {
      fgets(buf, 512, fdhist);
      l = strlen(buf)-1;
      if(buf[l] == '\n')
	buf[l] = 0;

      add_history(buf);
    }
    fclose(fdhist);
  }

  hist_loaded = 1;
}

void f_add_history(char *str)
{
  if (g_is_batch)
    return;
  FILE *fdhist;
  if(!(fdhist = fopen(".setpar.hist", "a")))
    return;
  fprintf(fdhist, "%s\n", str);
  fclose(fdhist);
}

void get_command(char *cmd, int *argc, char **argv, FILE* fd, char cmdline[512])
{
  // really, string processing in plain C?
  // ok whatever floats your boat. --pklenze
  cmdline[0]=0;

  static char *buf = NULL;
  int l;
  unsigned int n;
  char *tok;
  int ret=0;
  if(buf != NULL)
    free(buf);

  if (!fd)
    load_history();

  if (!fd)
    buf = readline("> ");
  else
    {
      buf=NULL;
      size_t buflen=0;

      ret=getline(&buf, &buflen, fd);
      if (ret!=-1)
	  buf[strlen(buf)-1]=0;
      //fprintf(stderr, "read %d bytes: %s, is_eof:%d\n", ret, cmdline, feof(fd));
    }
  snprintf(cmdline, 512, "%s", buf);

  if (!buf || ret<0) // eof
    {
      buf=(char*)malloc(50);
      strncpy(buf,"exit", 50);
    }
  
  l = strlen(buf) - 1;
  //buf[l] = 0;

  if(strlen(buf) != 0 && !fd)
  {
    add_history(buf);
    f_add_history(buf);
  }

  n = 0;
  tok = strtok(buf, " ");
  if(tok == NULL)
  {
    *argc = -1;
    return;
  }

  l = strlen(tok);
  strncpy(cmd, tok, 64);
  
  while(1)
  {
    tok = strtok(NULL, " ");
    if(!tok)
      break;
    l = strlen(tok);
    argv[n] = (char*)malloc(sizeof(char) * l + 1);
    strncpy(argv[n++], tok, l + 1);
  }

  *argc = n;
}

void free_command(char *cmd, int argc, char **argv)
{
  unsigned int i;

  //free(cmd);
  for(i = 0; i < (unsigned int) argc; i++)
    free(argv[i]);
  //free(argv);
}


int eval_print_loop(FILE* fd)
{
  // fd is either a file* to read from or 
  // null (read from stdin, interactive mode
  char cmdline[512];
  uint8_t stat=1;
  char cmd[64];
  int cmd_argc;
  char *cmd_argv[32];

  do
  {
    get_command(cmd, &cmd_argc, cmd_argv, fd,cmdline);
    if(cmd_argc == -1)
    {
      continue;
    }
    if (cmd[0]=='#' //preprocessor output
	|| (cmd[0]=='/' && cmd[1]=='/') //comment
	)
      continue;
    stat = interpret_command(cmd, cmd_argc, cmd_argv);
    if (stat==2)
      {
	fprintf(stderr, "Command \"%s\" failed \n", cmdline);
	if (fd)
	  return -1;
      }
    free_command(cmd, cmd_argc, cmd_argv);
    
  }
  while(stat);
  return 0;
}
