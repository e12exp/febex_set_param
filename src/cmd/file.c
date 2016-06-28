#include <stdlib.h>
#include <stdio.h>

#include "file.h"

#include "../command.h"
#include "../data.h"
#include "../readfile.h"

IMPLS(file, list)
{
  uint8_t fi;

  printf("Open files:\n\n");
  printf("Number | Active | Modules         | Filename\n");
  printf("-------+--------+-----------------+---------\n");

  for(fi = 0; fi < g_num_files; fi++)
  {
    printf("%6d |      %s | %3d,%3d,%3d,%3d | %s\n", fi, fi == g_active_file ? "*" : " ",
        g_file_data[fi]->num_sfp >= 1 ? g_file_data[fi]->num_modules[0] : 0,
        g_file_data[fi]->num_sfp >= 2 ? g_file_data[fi]->num_modules[1] : 0,
        g_file_data[fi]->num_sfp >= 3 ? g_file_data[fi]->num_modules[2] : 0,
        g_file_data[fi]->num_sfp == 4 ? g_file_data[fi]->num_modules[3] : 0,
        g_file_data[fi]->filename);
  }

  return 1;
}

IMPLS_HELP(file, list)
{
  printf("List currently open files.\n");
  return 1;
}

IMPLS(file, open)
{
  ARGS_INIT
  ARG_STR(filename)

  file_data_t *file = file_data_add(filename);
  if(readfile(file))
  {
    fill_data_from_file(file);
  }
  else
  {
    printf("Opened empty database file.\n");
  }

  return cmd_impl_file_list(0, NULL);
}

IMPLS_HELP(file, open)
{
  printf("Open a database file. The file will be created if it doesn't exist.\n\n"
      "Note: You may open up to 256 files at once and switch the active file using the\n"
      "      'file use' command.\n\n"
      "See: file use\n");
  return 1;
}

IMPLS(file, use)
{
  ARGS_INIT
  ARG_INT(fileno)

  if(fileno < 0 || fileno >= g_num_files)
  {
    fprintf(stderr, "Invalid file number\n");
    return 0;
  }

  g_active_file = fileno;

  printf("Active file: %d (%s)\n", g_active_file, g_file_data[g_active_file]->filename);

  return 1;
}

IMPLS_HELP(file, use)
{
  printf("Set the currently active file on which all commands are applied.\n"
      "  file-number: Number of the opened file to use.\n\n"
      "See 'file list' to list all currently open files.\n");
  return 1;
}

IMPLS(file, close)
{
  ARGS_INIT
  ARG_INT(fileno)

  if(fileno < 0)
  {
    fileno = g_active_file;
  }

  if(fileno >= g_num_files)
  {
    fprintf(stderr, "Invalid file number.\n");
    return 0;
  }

  if(g_num_files == 1)
  {
    fprintf(stderr, "You have to keep one open file.\n");
    return 0;
  }

  file_data_close(fileno);

  if(fileno == g_active_file)
  {
    g_active_file = 0;
  }

  printf("File closed. Currently active file: %d (%s)\n", g_active_file, g_file_data[g_active_file]->filename);

  return 1;
}

IMPLS_HELP(file, close)
{
  printf("Close a file.\n"
      "  [file-number]: Number of file to close. If omitted, the active file will be closed.\n\n"
      "Note: The file will NOT automatically be saved!\n");
  return 1;
}

IMPLS(file, save)
{
  int8_t fileno;

  if(argc < 1)
    fileno = -1;
  else
    fileno = (int8_t)atoi(argv[0]);

  char c;
  file_data_t *file;

  if(fileno < 0)
  {
    fileno = g_active_file;
  }

  if(fileno >= g_num_files)
  {
    fprintf(stderr, "Invalid file number.\n");
    return 0;
  }

  file = g_file_data[fileno];

  if(file->version < FILEVERSION_RECENT)
  {
    printf("Do you want to convert database file %s from version %d to version %d?\n",
        file->filename, file->version, FILEVERSION_RECENT);
    
    do
    {
      printf(" Yes (y) | No (n) ?\n");
      c = (char)getchar();
    }
    while(c != 'y' && c != 'n');

    if(c == 'y')
      file->version = FILEVERSION_RECENT;
  }

  fill_regdata_from_module_data(file);
  int r=write_file(file);
  if(r)
  {
    printf("Data written.\n");
  }

  return r;
}

IMPLS_HELP(file, save)
{
  printf("Save a file.\n"
      "  [file-number]: Number of file to save. If omitted, the currently active file will be saved.\n\n");
  return 1;
}

