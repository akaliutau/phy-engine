

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "gdt.h"
#include "private.h"



static void gdt_info(char *message)
{
#ifndef DEBUG
  
#else 
  fprintf(stderr, "%s\n", message);
#endif 
}


static void gdt_warning(char *message)
{
  fprintf(stderr, "warning: %s\n", message);
}


static void gdt_error(char *message)
{
  fprintf(stderr, "error: %s\n", message);
}


static void gdt_fatal(char *message)
{
  fprintf(stderr, "fatal error: %s\n", message);
  exit(-1);
}

static GDT_MSG_CALLBACK_FUNC gdt_info_callback_func = gdt_info,
                             gdt_warning_callback_func = gdt_warning,
                             gdt_error_callback_func = gdt_error,
                             gdt_fatal_callback_func = gdt_fatal;


GDT_MSG_CALLBACK_FUNC GdtSetInfoCallback(GDT_MSG_CALLBACK_FUNC func)
{
  GDT_MSG_CALLBACK_FUNC old_func = gdt_info_callback_func;
  gdt_info_callback_func = func;
  return old_func;
}

GDT_MSG_CALLBACK_FUNC GdtSetWarningCallback(GDT_MSG_CALLBACK_FUNC func)
{
  GDT_MSG_CALLBACK_FUNC old_func = gdt_warning_callback_func;
  gdt_warning_callback_func = func;
  return old_func;
}

GDT_MSG_CALLBACK_FUNC GdtSetErrorCallback(GDT_MSG_CALLBACK_FUNC func)
{
  GDT_MSG_CALLBACK_FUNC old_func = gdt_error_callback_func;
  gdt_error_callback_func = func;
  return old_func;
}

GDT_MSG_CALLBACK_FUNC GdtSetFatalCallback(GDT_MSG_CALLBACK_FUNC func)
{
  GDT_MSG_CALLBACK_FUNC old_func = gdt_fatal_callback_func;
  gdt_fatal_callback_func = func;
  return old_func;
}


void GdtInfo(char *routine, char *text, ...)
{
	va_list pvar;
	char buf[GDT_MAX_MESSAGE_LENGTH], *p;
	
	va_start(pvar, text);
	vsprintf(buf, text, pvar);
	va_end(pvar);

	if (routine) {
	  
	  for (p=buf; *p; p++) {}
	  sprintf(p, " (in subroutine %s)", routine);
	}
	
	gdt_info_callback_func(buf);
}


void GdtWarning(char *routine, char *text, ...)
{
	va_list pvar;
	char buf[GDT_MAX_MESSAGE_LENGTH], *p;

	va_start(pvar, text);
	vsprintf(buf, text, pvar);
	va_end(pvar);

	if (routine) {
	  
	  for (p=buf; *p; p++) {}
	  sprintf(p, " (in subroutine %s)", routine);
	}
	
	gdt_warning_callback_func(buf);
}


void GdtError(char *routine, char *text, ...)
{
	va_list pvar;
	char buf[GDT_MAX_MESSAGE_LENGTH], *p;
	
	va_start(pvar, text);
	vsprintf(buf, text, pvar);
	va_end(pvar);

	if (routine) {
	  
	  for (p=buf; *p; p++) {}
	  sprintf(p, " (in subroutine %s)", routine);
	}
	
	gdt_error_callback_func(buf);
}


void GdtFatal(char *routine, char *text, ...)
{
	va_list pvar;
	char buf[GDT_MAX_MESSAGE_LENGTH], *p;
	
	va_start(pvar, text);
	vsprintf(buf, text, pvar);
	va_end(pvar);

	if (routine) {
	  
	  for (p=buf; *p; p++) {}
	  sprintf(p, " (in subroutine %s)", routine);
	}
	
	gdt_fatal_callback_func(buf);
}

