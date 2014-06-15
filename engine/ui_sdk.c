

#include "ui.h"
#include "ui_sdk.h"
#include "file.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Xm/Xm.h>
#include <Xm/PushB.h>
#include <Xm/Form.h>
#include <Xm/MessageB.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/CascadeB.h>
#include <Xm/TextF.h>
#include <Xm/Separator.h>
#include <Xm/SelectioB.h>
#include <Xm/FileSB.h>
#include <Xm/ScrolledW.h>

#include "error.h"
#include "pools.h"


void EnableWidget(Widget widget)
{
  XtSetSensitive(widget, True);
}


void DisableWidget(Widget widget)
{
  XtSetSensitive(widget, False);
}


Widget CreatePushButton(Widget parent,
			char *widgetname, 
			void (*activate_callback)(Widget, XtPointer, XtPointer),
			XtPointer client_data)
{
  Widget pushb;

  pushb = XtVaCreateManagedWidget(widgetname,
				  xmPushButtonWidgetClass,
				  parent,
				  NULL);

  if (activate_callback)
    XtAddCallback(pushb, XmNactivateCallback, activate_callback, client_data);

  return pushb;
}


Widget CreateDialog(Widget parent, char *name)
{
  return XmCreateTemplateDialog(parent, name, visargs, nrvisargs);
}

void SetDialogTitle(Widget dialog, char *title)
{
  XtVaSetValues(dialog,
		XtVaTypedArg, XmNtitle, XmRString,
		title, strlen(title)+1, NULL);
}


void UnmanageMessageBoxChild(Widget dialog, unsigned char child)
{
  Widget childw = XmMessageBoxGetChild(dialog, child);
  XtUnmanageChild(childw);
}


void ManageMessageBoxChild(Widget dialog, unsigned char child)
{
  Widget childw = XmMessageBoxGetChild(dialog, child);
  XtManageChild(childw);
}


Widget CreateRadioBox(Widget parent, char *name)
{
  return XmCreateRadioBox(parent, name, visargs, nrvisargs);
}


Widget CreateRowColumn(Widget parent, char *name)
{
  return XmCreateRowColumn(parent, name, visargs, nrvisargs);
}


Widget CreateLabel(Widget parent, char *name)
{
  return XtVaCreateManagedWidget(name,
				 xmLabelWidgetClass,
				 parent,
				 NULL);
}



char *extract_normal_string(XmString cs)
{
  XmStringContext context;
  XmStringCharSet charset;
  XmStringDirection direction;
  Boolean separator;
  char *primitive_string, *p;
  int n;
  static char buf[MAX_LABEL_STRING_LENGTH+1];

  XmStringInitContext (&context,cs);

  p = buf; n = 0;
  while (XmStringGetNextSegment (context,&primitive_string,
				 &charset,&direction,&separator)) {
    int l;
    if (!primitive_string) {
      Warning("extract_normal_string", "no primitive string");
      break;
    }
    l = strlen(primitive_string);
    if (n + 1 + l > MAX_LABEL_STRING_LENGTH) {
      Warning("extract_normal_string", "Buffer overflow. Recompile ui_sdk.c with a larger value for MAX_LABEL_STRING_LENGTH (currently %d)",
	      MAX_LABEL_STRING_LENGTH);
      break;
    }
    if (p != buf) { *p++ = '\n'; n++; }	
    strcpy(p, primitive_string);
    p += l; n += l;
  }

  XmStringFreeContext (context);

  return buf;
}


char *GetLabelString(Widget label)
{
  XmString xmstr;
  XtVaGetValues(label, XmNlabelString, &xmstr, NULL);
  return strdup(extract_normal_string(xmstr));
}


void SetLabelString(Widget label, char *message)
{
  XtVaSetValues(label,
		XtVaTypedArg,
		  XmNlabelString, XmRString, message, strlen(message)+1,
		NULL);
}


Widget CreateForm(Widget parent,
		  char *name)
{
  return XtVaCreateWidget(name,
			  xmFormWidgetClass,
			  parent,
			  NULL);
}


Widget CreateFrame(Widget parent, 
		   char *framename, 
		   char *frametitlename)
{
  Widget frame;

  frame = XtVaCreateManagedWidget(framename,
				  xmFrameWidgetClass,
				  parent,
				  NULL);

  if (frametitlename) {
    XtVaCreateManagedWidget(frametitlename,
			    xmLabelWidgetClass,
			    frame,
			    XmNchildType, XmFRAME_TITLE_CHILD,
			    NULL);
  }

  return frame;
}


static void default_toggle_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
  int *state = (int *)client_data;
  int set = (((XmToggleButtonCallbackStruct *)call_data)->set == XmSET);
  *state = set;
}


Widget CreateToggleButton(Widget parent,
			  char *name,
			  Boolean initially_set,
			  void (*toggle_callback)(Widget, XtPointer, XtPointer),
			  XtPointer client_data)
{
  Widget toggleB;

  toggleB = XtVaCreateManagedWidget(name,
				    xmToggleButtonWidgetClass,
				    parent,
				    XmNset, initially_set,
				    NULL);

  if (toggle_callback) 
    XtAddCallback(toggleB, XmNvalueChangedCallback, toggle_callback, client_data);
  else if (client_data)
    XtAddCallback(toggleB, XmNvalueChangedCallback, default_toggle_callback, client_data);

  return toggleB;
}


Widget CreateShell(char *title)
{
  return XtAppCreateShell(title,
			  APP_CLASS_NAME,
			  topLevelShellWidgetClass,
			  display,
			  visargs, nrvisargs);
}


Widget CreateMenuBar(Widget parent, char *menuname)
{
  return XmCreateMenuBar(parent, menuname, visargs, nrvisargs);
}


Widget CreatePopupMenu(Widget parent, char *menuname)
{
  return XmCreatePopupMenu(parent, menuname, visargs, nrvisargs);
}


Widget CreateSubMenu(Widget parent, char *buttonname, char *menuname)
{
  Widget menu, button;

  button = XtVaCreateManagedWidget(buttonname,
				   xmCascadeButtonWidgetClass,
				   parent,
				   NULL);

  menu = XmCreatePulldownMenu(parent, menuname, visargs, nrvisargs);

  XtVaSetValues(button,
		XmNsubMenuId, menu,
		NULL);

  return menu;
}


Widget CreateHelpSubMenu(Widget parent, char *buttonname, char *menuname)
{
  Widget menu, button;

  button = XtVaCreateManagedWidget(buttonname,
				   xmCascadeButtonWidgetClass,
				   parent,
				   NULL);

  menu = XmCreatePulldownMenu(parent, menuname, visargs, nrvisargs);

  XtVaSetValues(button,
		XmNsubMenuId, menu,
		NULL);

  XtVaSetValues(parent,
		XmNmenuHelpWidget, button,
		NULL);

  return menu;
}


Widget CreateOptionMenu(Widget parent, char *buttonname, char *submenuname)
{
  Widget menu, button;
  Arg args[8];
  int n = 0;

  menu = XmCreatePulldownMenu(parent, submenuname, visargs, nrvisargs);
  
  for (n=0; n<nrvisargs; n++)
    args[n] = visargs[n];
  XtSetArg(args[n], XmNsubMenuId, menu); n++;
  button = XmCreateOptionMenu(parent, buttonname, args, n);
  XtManageChild(button);

  return button;
}


Widget CreateOptionButton(Widget optionmenu, char *buttonname,
			  Boolean set, 
			  void (*activate_callback)(Widget, XtPointer, XtPointer),
			  XtPointer client_data)
{
  Widget submenu, button;

  XtVaGetValues(optionmenu,
		XmNsubMenuId, &submenu,
		NULL);

  button = CreatePushButton(submenu, buttonname, activate_callback, client_data);
  if (set) 
    XtVaSetValues(submenu,
		  XmNmenuHistory, button,
		  NULL);

  return button;
}


void DoPopup(Widget w, XtPointer client_data, XtPointer call_data)
{
  XtManageChild((Widget)client_data);
}


Widget CreateCascadeDialog(Widget parent,
			   char *buttonname,
			   Widget (*create_dialog)(Widget, char *),
			   char *dialogname,
			   void (*popup_callback)(Widget, XtPointer, XtPointer),
			   XtPointer client_data)
{
  Widget button, dialog;

  button = XtVaCreateManagedWidget(buttonname,
				   xmPushButtonWidgetClass,
				   parent,
				   NULL);

  if (!create_dialog)
    dialog = XmCreateTemplateDialog(button, dialogname, visargs, nrvisargs);
  else
    dialog = create_dialog(button, dialogname);

  if (popup_callback) 
    XtAddCallback(button, XmNactivateCallback, popup_callback,
		  client_data ? client_data : (XtPointer)dialog);
  else
    XtAddCallback(button, XmNactivateCallback, DoPopup, (XtPointer)dialog);

  return dialog;
}


static char *handle_fet_type(FORMENTRYTYPE type, XtPointer pvalue, char *buf)
{
  char *format = (char *)NULL;

  
  switch (type) {
  case FET_SHORT:
    format = "%hd";
    sprintf(buf, format, *(short *)pvalue);
    break;
  case FET_INTEGER:
    format = "%d";
    sprintf(buf, format, *(int *)pvalue);
    break;
  case FET_LONG:
    format = "%ld";
    sprintf(buf, format, *(long *)pvalue);
    break;
  case FET_UNSIGNED_SHORT:
    format = "%hu";
    sprintf(buf, format, *(unsigned short *)pvalue);
    break;
  case FET_UNSIGNED:
    format = "%u";
    sprintf(buf, format, *(unsigned int *)pvalue);
    break;
  case FET_UNSIGNED_LONG:
    format = "%lu";
    sprintf(buf, format, *(unsigned long *)pvalue);
    break;
  case FET_FLOAT:
    format = "%g";
    sprintf(buf, format, *(float *)pvalue);
    break;
  case FET_DOUBLE:
    format = "%lg";
    sprintf(buf, format, *(double *)pvalue);
    break;
  case FET_STRING:
    format = "%s";
    sprintf(buf, format, (char *)pvalue);
    break;
  default:
    Fatal(2, "handle_fet_type", "Invalid form entry type %d", type);    
  }

  return format;
}


typedef struct TEXTFIELDCONTENTS {
  FORMENTRYTYPE type;
  char *format;
  XtPointer pvalue;
  Boolean *valid;
  char *buf;
  int buflen;
  Widget textf;
} TEXTFIELDCONTENTS;

static void TextFieldGetValue(Widget textf, XtPointer client_data, XtPointer call_data)
{
  TEXTFIELDCONTENTS *contents = (TEXTFIELDCONTENTS *)client_data;
  char *text = XmTextFieldGetString(textf);
  *(contents->valid) = (sscanf(text, contents->format, contents->pvalue)==1);
  free(text);
}

static void TextFieldSetValue(Widget dialog, XtPointer client_data, XtPointer call_data)
{
  TEXTFIELDCONTENTS *contents = (TEXTFIELDCONTENTS *)client_data;
  handle_fet_type(contents->type, contents->pvalue, contents->buf);
  XmTextFieldSetString(contents->textf, contents->buf);
}

static void TextFieldDestroy(Widget textf, XtPointer client_data, XtPointer call_data)
{
  TEXTFIELDCONTENTS *contents = (TEXTFIELDCONTENTS *)client_data;
  if (contents->buflen >= 250)	
    Free((char *)contents->buf, contents->buflen+1);
  Free((char *)contents, sizeof(TEXTFIELDCONTENTS));
}


Widget CreateFormEntry(Widget parent, char *labelname, char *textfname,
		       FORMENTRYTYPE type, XtPointer pvalue, Boolean *valid,
		       int fieldlength)
{
  Widget entry=(Widget)NULL, value;
  char *buf;
  static char defbuf[250];	
  TEXTFIELDCONTENTS *contents;
  char *format = NULL;

  static Boolean bla;
  if (!valid) valid=&bla;

  if (fieldlength >= 250)
    buf = (char *)Alloc(fieldlength+1);
  else
    buf = defbuf;

  if (labelname) {
    
    entry = XtVaCreateWidget("formEntry",
			     xmFormWidgetClass,
			     parent,
			     NULL);
  }

  
  value = XtVaCreateManagedWidget(textfname,
				  xmTextFieldWidgetClass,
				  labelname ? entry : parent,
				  XmNrightAttachment,	XmATTACH_FORM,
				  NULL);

  if (fieldlength>0)
    XtVaSetValues(value,
		  XmNmaxLength, fieldlength,
		  NULL);

  format = handle_fet_type(type, pvalue, buf);
  *valid = True;
  XmTextFieldSetString(value, buf);

  
  contents = (TEXTFIELDCONTENTS *)Alloc(sizeof(TEXTFIELDCONTENTS));
  contents->type = type;
  contents->format = format;
  contents->pvalue = pvalue;
  contents->valid = valid;
  contents->buf = buf;
  contents->buflen = fieldlength;
  contents->textf = value;

  XtAddCallback(value, XmNvalueChangedCallback, TextFieldGetValue, (XtPointer)contents);
  XtAddCallback(value, XmNdestroyCallback, TextFieldDestroy, (XtPointer)contents);
  

  if (labelname) {
    
    XtVaCreateManagedWidget(labelname, 
				    xmLabelWidgetClass,
				    entry,
				    XmNrightAttachment, 	XmATTACH_WIDGET,
				    XmNrightWidget,		value,
				    XmNtopAttachment,		XmATTACH_FORM,
				    XmNbottomAttachment,	XmATTACH_FORM,
				    NULL);

    XtManageChild(entry);
    return entry;
  } else
    return value;
}


Widget CreatePromptDialog(Widget parent, char *name,
			  FORMENTRYTYPE type, XtPointer pvalue, 
			  Boolean *valid, int fieldlength)
{
  Widget dialog, value;
  char *buf;
  static char defbuf[250];	
  TEXTFIELDCONTENTS *contents;
  char *format = NULL;

  static Boolean bla;
  if (!valid) valid=&bla;

  if (fieldlength >= 250)
    buf = (char *)Alloc(fieldlength+1);
  else
    buf = defbuf;

  dialog = XmCreatePromptDialog(parent, name, visargs, nrvisargs);
  value = XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT);

  if (fieldlength>0)
    XtVaSetValues(value,
		  XmNmaxLength, fieldlength,
		  NULL);

  format = handle_fet_type(type, pvalue, buf);
  *valid = True;
  XmTextFieldSetString(value, buf);

  
  contents = (TEXTFIELDCONTENTS *)Alloc(sizeof(TEXTFIELDCONTENTS));
  contents->type = type;
  contents->format = format;
  contents->pvalue = pvalue;
  contents->valid = valid;
  contents->buf = buf;
  contents->buflen = fieldlength;
  contents->textf = value;
  XtAddCallback(value, XmNvalueChangedCallback, TextFieldGetValue, (XtPointer)contents);
  XtAddCallback(value, XmNdestroyCallback, TextFieldDestroy, (XtPointer)contents);
  XtAddCallback(dialog, XmNmapCallback, TextFieldSetValue, (XtPointer)contents);

  UnmanageSelectionBoxChild(dialog, XmDIALOG_HELP_BUTTON);

  return dialog;
}			  


void UnmanageSelectionBoxChild(Widget dialog, unsigned char child)
{
  Widget childw = XmSelectionBoxGetChild(dialog, child);
  XtUnmanageChild(childw);
}


void ManageSelectionBoxChild(Widget dialog, unsigned char child)
{
  Widget childw = XmSelectionBoxGetChild(dialog, child);
  XtManageChild(childw);
}

static void unmanage_callback(Widget dialog, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild(dialog);
}

typedef struct FSDATA {
  char *open_mode;
  int (*process_file)(char *fname, FILE *fp, int ispipe, Widget fsbox);
} FSDATA;

static void process_file_callback(Widget dialog, XtPointer client_data, XtPointer call_data)
{
  FSDATA *fsdata = (FSDATA *)client_data;
  char *filename = extract_normal_string (((XmSelectionBoxCallbackStruct *)call_data)->value);
  char *open_mode = fsdata->open_mode;
  int ispipe = FALSE, succes;
  FILE *fp = (FILE *)NULL;

  
  if(!open_mode)
  {
    

    
    succes = fsdata->process_file(filename, fp, ispipe, dialog);
  }
  else 
  {
    fp = OpenFile(filename, open_mode, &ispipe);

    
    succes = fsdata->process_file(filename, fp, ispipe, dialog);

    CloseFile(fp, ispipe);
  }

  
  if (succes)
    XtUnmanageChild(dialog);
}


Widget CreateFileSelectionDialog(Widget parent, char *name, 
				 int (*process_file)(char *fname, FILE *fp, int ispipe, Widget fsbox), char *open_mode)
{
  Widget dialog = XmCreateFileSelectionDialog(parent, name, visargs, nrvisargs);

  XtVaSetValues(dialog, XmNautoUnmanage, False, NULL);

  XtAddCallback(dialog, XmNcancelCallback, unmanage_callback, NULL);
  if (process_file) {
    FSDATA *fsdata = (FSDATA *)Alloc(sizeof(FSDATA));
    fsdata->open_mode = open_mode;
    fsdata->process_file = process_file;
    XtAddCallback(dialog, XmNokCallback, process_file_callback, (XtPointer)fsdata);
  }

  UnmanageSelectionBoxChild(dialog, XmDIALOG_HELP_BUTTON);

  return dialog;
}


Widget CreateSeparator(Widget parent, char *name)
{
  return XtVaCreateManagedWidget(name,
				 xmSeparatorWidgetClass,
				 parent,
				 NULL);
}


Widget CreateScrolledWindow(Widget parent, char *name, 
			    Widget (*create_child)(Widget parent, char *childname), char *child_name)
{
  Widget scrolledW, child;

  scrolledW = XtVaCreateManagedWidget(name, 
				      xmScrolledWindowWidgetClass,
				      parent,
				      NULL);

  if (create_child)
    child = create_child(scrolledW, child_name);
  else
    child = CreateLabel(scrolledW, child_name);

  XtVaSetValues(scrolledW,
		XmNworkWindow, child,
		NULL);

  return scrolledW;
}



static void browse_filename_okcallback(Widget dialog, XtPointer client_data, XtPointer call_data)
{
  Widget formEntry = (Widget)client_data;
  char *filename = extract_normal_string (((XmSelectionBoxCallbackStruct *)call_data)->value);

  XmTextFieldSetString(formEntry, filename);

  XtUnmanageChild(dialog);
}


Widget CreateFileAndBrowseEntry(Widget parent, 
				char *labelname, char *textfname,
				XtPointer pvalue, 
				Boolean *valid, int fieldlength)
{
  Widget rowcol=(Widget)NULL;
  Widget formEntry, button, dialog;

  rowcol = CreateRowColumn(parent, "fileAndBrowseRowCol");

  formEntry = CreateFormEntry(rowcol, NULL, textfname,
			      FET_STRING, pvalue, valid, fieldlength);

  button = XtVaCreateManagedWidget("fileAndBrowseButton",
				   xmPushButtonWidgetClass,
				   rowcol,
				   NULL);

  dialog = XmCreateFileSelectionDialog(button, "fileAndBrowseDialog", 
				       visargs, nrvisargs);

  XtVaSetValues(dialog, XmNautoUnmanage, False, NULL);
  XtAddCallback(dialog, XmNcancelCallback, unmanage_callback, NULL);

  XtAddCallback(dialog, XmNokCallback, browse_filename_okcallback, 
		(XtPointer)formEntry);


  UnmanageSelectionBoxChild(dialog, XmDIALOG_HELP_BUTTON);

  XtAddCallback(button, XmNactivateCallback, DoPopup, (XtPointer)dialog);

  XtManageChild(rowcol);

  return rowcol;
}
