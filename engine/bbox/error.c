

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "bbox.h"



static void brep_info(void *client_data, char *message)
{
#ifndef DEBUG
  
#else 
  fprintf(stderr, "%s\n", message);
#endif 
}


static void brep_warning(void *client_data, char *message)
{
  fprintf(stderr, "warning: %s\n", message);
}


static void brep_error(void *client_data, char *message)
{
  fprintf(stderr, "error: %s\n", message);
}


static void brep_fatal(void *client_data, char *message)
{
  fprintf(stderr, "fatal error: %s\n", message);
  exit(-1);
}

static BBOX_MSG_CALLBACK_FUNC brep_info_callback_func = brep_info,
                              brep_warning_callback_func = brep_warning,
                              brep_error_callback_func = brep_error,
                              brep_fatal_callback_func = brep_fatal;


BBOX_MSG_CALLBACK_FUNC BBoxSetInfoCallback(BBOX_MSG_CALLBACK_FUNC func)
{
  BBOX_MSG_CALLBACK_FUNC old_func = brep_info_callback_func;
  brep_info_callback_func = func;
  return old_func;
}

BBOX_MSG_CALLBACK_FUNC BBoxSetWarningCallback(BBOX_MSG_CALLBACK_FUNC func)
{
  BBOX_MSG_CALLBACK_FUNC old_func = brep_warning_callback_func;
  brep_warning_callback_func = func;
  return old_func;
}

BBOX_MSG_CALLBACK_FUNC BBoxSetErrorCallback(BBOX_MSG_CALLBACK_FUNC func)
{
  BBOX_MSG_CALLBACK_FUNC old_func = brep_error_callback_func;
  brep_error_callback_func = func;
  return old_func;
}

BBOX_MSG_CALLBACK_FUNC BBoxSetFatalCallback(BBOX_MSG_CALLBACK_FUNC func)
{
  BBOX_MSG_CALLBACK_FUNC old_func = brep_fatal_callback_func;
  brep_fatal_callback_func = func;
  return old_func;
}


void BBoxInfo(void *client_data, char *routine, char *text, ...)
{
	va_list pvar;
	char buf[BBOX_MAX_MESSAGE_LENGTH];
	
	va_start(pvar, text);
	vsprintf(buf, text, pvar);
	va_end(pvar);

#ifdef DEBUG
	if (routine) {
	  char *p;
	  
	  for (p=buf; *p; p++) {}
	  sprintf(p, " (%s)", routine);
	}
#endif
	
	brep_info_callback_func(client_data, buf);
}


void BBoxWarning(void *client_data, char *routine, char *text, ...)
{
	va_list pvar;
	char buf[BBOX_MAX_MESSAGE_LENGTH];

	va_start(pvar, text);
	vsprintf(buf, text, pvar);
	va_end(pvar);

#ifdef DEBUG
	if (routine) {
	  char *p;
	  
	  for (p=buf; *p; p++) {}
	  sprintf(p, " (%s)", routine);
	}
#endif
	
	brep_warning_callback_func(client_data, buf);
}


void BBoxError(void *client_data, char *routine, char *text, ...)
{
	va_list pvar;
	char buf[BBOX_MAX_MESSAGE_LENGTH];
	
	va_start(pvar, text);
	vsprintf(buf, text, pvar);
	va_end(pvar);

#ifdef DEBUG
	if (routine) {
	  char *p;
	  
	  for (p=buf; *p; p++) {}
	  sprintf(p, " (%s)", routine);
	}
#endif
	
	brep_error_callback_func(client_data, buf);
}


void BBoxFatal(void *client_data, char *routine, char *text, ...)
{
	va_list pvar;
	char buf[BBOX_MAX_MESSAGE_LENGTH];
	
	va_start(pvar, text);
	vsprintf(buf, text, pvar);
	va_end(pvar);

#ifdef DEBUG
	if (routine) {
	  char *p;
	  
	  for (p=buf; *p; p++) {}
	  sprintf(p, " (%s)", routine);
	}
#endif
	
	brep_fatal_callback_func(client_data, buf);
}

