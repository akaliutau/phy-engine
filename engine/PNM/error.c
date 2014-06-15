

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "pnm.h"
#include "pnm_private.h"





static void pnm_warning(char *message)
{
  fprintf(stderr, "warning: %s\n", message);
}


static void pnm_error(char *message)
{
  fprintf(stderr, "error: %s\n", message);
}


static void pnm_fatal(char *message)
{
  fprintf(stderr, "fatal error: %s\n", message);
  exit(-1);
}

PNM_MSG_CALLBACK_FUNC pnm_info_callback_func = NULL,
                      pnm_warning_callback_func = pnm_warning,
                      pnm_error_callback_func = pnm_error,
                      pnm_fatal_callback_func = pnm_fatal;


PNM_MSG_CALLBACK_FUNC PnmSetInfoCallback(PNM_MSG_CALLBACK_FUNC func)
{
  PNM_MSG_CALLBACK_FUNC old_func = pnm_info_callback_func;
  pnm_info_callback_func = func;
  return old_func;
}

PNM_MSG_CALLBACK_FUNC PnmSetWarningCallback(PNM_MSG_CALLBACK_FUNC func)
{
  PNM_MSG_CALLBACK_FUNC old_func = pnm_warning_callback_func;
  pnm_warning_callback_func = func;
  return old_func;
}

PNM_MSG_CALLBACK_FUNC PnmSetErrorCallback(PNM_MSG_CALLBACK_FUNC func)
{
  PNM_MSG_CALLBACK_FUNC old_func = pnm_error_callback_func;
  pnm_error_callback_func = func;
  return old_func;
}

PNM_MSG_CALLBACK_FUNC PnmSetFatalCallback(PNM_MSG_CALLBACK_FUNC func)
{
  PNM_MSG_CALLBACK_FUNC old_func = pnm_fatal_callback_func;
  pnm_fatal_callback_func = func;
  return old_func;
}


void PnmInfo(char *routine, char *text, ...)
{
  va_list pvar;
  char buf[PNM_MAX_MESSAGE_LENGTH], *p;
	
  va_start(pvar, text);
  vsprintf(buf, text, pvar);
  va_end(pvar);
  
  if (routine) {
    
    for (p=buf; *p; p++) {}
    sprintf(p, " (in subroutine %s)", routine);
  }
	
  pnm_info_callback_func(buf);
}


void PnmWarning(char *routine, char *text, ...)
{
  va_list pvar;
  char buf[PNM_MAX_MESSAGE_LENGTH], *p;
  
  va_start(pvar, text);
  vsprintf(buf, text, pvar);
  va_end(pvar);
  
  if (routine) {
    
    for (p=buf; *p; p++) {}
    sprintf(p, " (in subroutine %s)", routine);
  }
  
  pnm_warning_callback_func(buf);
}


void PnmError(char *routine, char *text, ...)
{
  va_list pvar;
  char buf[PNM_MAX_MESSAGE_LENGTH], *p;
  
  va_start(pvar, text);
  vsprintf(buf, text, pvar);
  va_end(pvar);
  
  if (routine) {
    
    for (p=buf; *p; p++) {}
    sprintf(p, " (in subroutine %s)", routine);
  }
	
  pnm_error_callback_func(buf);
}


void PnmFatal(char *routine, char *text, ...)
{
  va_list pvar;
  char buf[PNM_MAX_MESSAGE_LENGTH], *p;
  
  va_start(pvar, text);
  vsprintf(buf, text, pvar);
  va_end(pvar);
  
  if (routine) {
    
    for (p=buf; *p; p++) {}
    sprintf(p, " (in subroutine %s)", routine);
  }
  
  pnm_fatal_callback_func(buf);
}

