

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "pools.h"
#include "error.h"





static void pools_warning(char *message)
{
  fprintf(stderr, "warning: %s\n", message);
}


static void pools_error(char *message)
{
  fprintf(stderr, "error: %s\n", message);
}


static void pools_fatal(char *message)
{
  fprintf(stderr, "fatal error: %s\n", message);
  abort();
  exit(-1);
}

POOLS_MSG_CALLBACK_FUNC pools_info_callback_func = NULL,
                        pools_warning_callback_func = pools_warning,
                        pools_error_callback_func = pools_error,
                        pools_fatal_callback_func = pools_fatal;


POOLS_MSG_CALLBACK_FUNC PoolsSetInfoCallback(POOLS_MSG_CALLBACK_FUNC func)
{
  POOLS_MSG_CALLBACK_FUNC old_func = pools_info_callback_func;
  pools_info_callback_func = func;
  return old_func;
}

POOLS_MSG_CALLBACK_FUNC PoolsSetWarningCallback(POOLS_MSG_CALLBACK_FUNC func)
{
  POOLS_MSG_CALLBACK_FUNC old_func = pools_warning_callback_func;
  pools_warning_callback_func = func;
  return old_func;
}

POOLS_MSG_CALLBACK_FUNC PoolsSetErrorCallback(POOLS_MSG_CALLBACK_FUNC func)
{
  POOLS_MSG_CALLBACK_FUNC old_func = pools_error_callback_func;
  pools_error_callback_func = func;
  return old_func;
}

POOLS_MSG_CALLBACK_FUNC PoolsSetFatalCallback(POOLS_MSG_CALLBACK_FUNC func)
{
  POOLS_MSG_CALLBACK_FUNC old_func = pools_fatal_callback_func;
  pools_fatal_callback_func = func;
  return old_func;
}


void PoolsInfo(char *routine, char *text, ...)
{
	va_list pvar;
	char buf[POOLS_MAX_MESSAGE_LENGTH], *p;
	
	va_start(pvar, text);
	vsprintf(buf, text, pvar);
	va_end(pvar);

	if (routine) {
	  
	  for (p=buf; *p; p++) {}
	  sprintf(p, " (in subroutine %s)", routine);
	}
	
	pools_info_callback_func(buf);
}


void PoolsWarning(char *routine, char *text, ...)
{
	va_list pvar;
	char buf[POOLS_MAX_MESSAGE_LENGTH], *p;

	va_start(pvar, text);
	vsprintf(buf, text, pvar);
	va_end(pvar);

	if (routine) {
	  
	  for (p=buf; *p; p++) {}
	  sprintf(p, " (in subroutine %s)", routine);
	}
	
	pools_warning_callback_func(buf);
}


void PoolsError(char *routine, char *text, ...)
{
	va_list pvar;
	char buf[POOLS_MAX_MESSAGE_LENGTH], *p;
	
	va_start(pvar, text);
	vsprintf(buf, text, pvar);
	va_end(pvar);

	if (routine) {
	  
	  for (p=buf; *p; p++) {}
	  sprintf(p, " (in subroutine %s)", routine);
	}
	
	pools_error_callback_func(buf);
}


void PoolsFatal(char *routine, char *text, ...)
{
	va_list pvar;
	char buf[POOLS_MAX_MESSAGE_LENGTH], *p;
	
	va_start(pvar, text);
	vsprintf(buf, text, pvar);
	va_end(pvar);

	if (routine) {
	  
	  for (p=buf; *p; p++) {}
	  sprintf(p, " (in subroutine %s)", routine);
	}
	
	pools_fatal_callback_func(buf);
}

