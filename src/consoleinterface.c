#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "consoleinterface.h"
#include "data.h"

display_level_t g_display_level;

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
  FILE *fdhist;
  if(!(fdhist = fopen(".setpar.hist", "a")))
    return;
  fprintf(fdhist, "%s\n", str);
  fclose(fdhist);
}

void get_command(char *cmd, int *argc, char **argv)
{
  //char buf[256];
  static char *buf = NULL;
  int l;
  unsigned int n;
  char *tok;

  if(buf != NULL)
    free(buf);

  load_history();

  //printf("> ");
  //fgets(buf, 256, stdin);
  //
  buf = readline("> ");

  l = strlen(buf) - 1;
  //buf[l] = 0;

  if(strlen(buf) != 0)
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
  for(i = 0; i < argc; i++)
    free(argv[i]);
  //free(argv);
}

