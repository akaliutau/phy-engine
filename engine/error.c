

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "error.h"

static int errorOccurred = FALSE;


void ErrorReset(void)
{
  errorOccurred = FALSE;
}


int ErrorOccurred(void)
{
  int errocc = errorOccurred;
  
  errorOccurred = FALSE;
  return errocc;
}


void Error(char *routine, char *text, ...)
{
  va_list pvar;
  
  fprintf(stderr, "Error: ");
  if (routine) fprintf(stderr, "%s(): ", routine);
  
  va_start(pvar, text);
  vfprintf(stderr, text, pvar);
  va_end(pvar);
  
  fprintf(stderr, ".\n");
  fflush(stderr);
  
  errorOccurred = TRUE;
}

 
void Fatal(int errcode, char *routine, char *text, ...)
{
  va_list pvar;
  
  fprintf(stderr, "Fatal error: ");
  if (routine) fprintf(stderr, "%s(): ", routine);
  
  va_start(pvar, text);
  vfprintf(stderr, text, pvar);
  va_end(pvar);
  
  fprintf(stderr, ".\n");
  fflush(stderr);

  abort();
  exit(errcode);
  
  errorOccurred = TRUE;
}

void Warning(char *routine, char *text, ...)
{
  va_list pvar;
  
  fprintf(stderr, "Warning: ");
  if (routine) fprintf(stderr, "%s(): ", routine);
  
  va_start(pvar, text);
  vfprintf(stderr, text, pvar);
  va_end(pvar);
  
  fprintf(stderr, ".\n"); 
  fflush(stderr);
}

void Info(char *routine, char *text, ...)
{
#ifdef INFO
  va_list pvar;

  if (routine) fprintf(stderr, "%s(): ", routine);
  
  va_start(pvar, text);
  vfprintf(stderr, text, pvar);
  va_end(pvar);
  
  fprintf(stderr, ".\n"); 
  fflush(stderr);
#endif 
}
