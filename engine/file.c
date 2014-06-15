

#include <stdio.h>
#include <string.h>

#include "file.h"
#include "pools.h"
#include "error.h"


FILE *OpenFile(char *filename, char *open_mode, int *ispipe)
{
  FILE *fp = (FILE *)NULL;

  if ((*open_mode != 'r' && *open_mode != 'w' && *open_mode != 'a')) {
    Error("OpenFile", "Invalid fopen() mode '%s'\n",
	  open_mode ? open_mode : "(null)");
    return fp;
  }

  if (filename[0] != '\0' && filename[strlen(filename)-1] != '/') {
    char *cmd = Alloc(strlen(filename) + 20);
    char *ext = strrchr(filename, '.');
    if (filename[0] == '|') {
      sprintf(cmd, "%s", filename+1);
      fp = popen(cmd, open_mode);
      *ispipe = TRUE;
    } else if (ext && strcmp(ext, ".gz")==0) {
      if (*open_mode == 'r')
	sprintf(cmd, "gunzip < %s", filename);
      else
	sprintf(cmd, "gzip > %s", filename);
      fp = popen(cmd, open_mode);
      *ispipe = TRUE;
    } else if (ext && strcmp(ext, ".Z")==0) {
      if (*open_mode == 'r')
	sprintf(cmd, "uncompress < %s", filename);
      else
	sprintf(cmd, "compress > %s", filename);
      fp = popen(cmd, open_mode);
      *ispipe = TRUE;
    } else if (ext && strcmp(ext, ".bz")==0) {
      if (*open_mode == 'r')
	sprintf(cmd, "bunzip < %s", filename);
      else
	sprintf(cmd, "bzip > %s", filename);
      fp = popen(cmd, open_mode);
      *ispipe = TRUE;
    } else if (ext && strcmp(ext, ".bz2")==0) {
      if (*open_mode == 'r')
	sprintf(cmd, "bunzip2 < %s", filename);
      else
	sprintf(cmd, "bzip2 > %s", filename);
      fp = popen(cmd, open_mode);
      *ispipe = TRUE;
    } else {
      fp = fopen(filename, open_mode);
      *ispipe = FALSE;
    }
      
    Free(cmd, strlen(filename) + 20);
      
    if (!fp) {
      Error(NULL, "Can't open file '%s' for %s", 
	    filename, *open_mode == 'r' ? "reading" : "writing");
    }
    return fp;
  }

  return (FILE *)NULL;
}


void CloseFile(FILE *fp, int ispipe)
{
  if (fp) {
    
    if (ispipe)
      pclose(fp);
    else
      fclose(fp);
  }
}
