/* ui_sdk.h: user interface toolkit --- The goal of this toolkit is just to 
 * make programming the most common things, such as menu and form creation,
 * easier. This toolkit is not complete. You will need to use the original 
 * Motif routines for accomplishing more complicated things. */

/* TODO: inhoud van form entry textfields aanpassen aan huidige waarde van variabele
 * bij iedere expose. */

#ifndef _UIT_H_
#define _UIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <Xm/Xm.h>
#include "motif.h"

/* No label or other resource string that will be converted to a normal C
 * string, will be longer than this */
#define MAX_LABEL_STRING_LENGTH 500

#define DEFAULT_CALLBACK (void (*)(Widget, XtPointer, XtPointer))NULL

/* creates a template dialog */
extern Widget CreateDialog(Widget parent, char *name);

/* sets dialog title */
extern void SetDialogTitle(Widget dialog, char *title);

/* unmanages a message box child (e.g. for a template dialog created with
 * CreateDialog(). */
extern void UnmanageMessageBoxChild(Widget dialog, unsigned char child);

/* manages a message box child */
extern void ManageMessageBoxChild(Widget dialog, unsigned char child);

/* Creates and returns a form widget with given name */
extern Widget CreateForm(Widget parent, char *name);

/* creates a radio box */
extern Widget CreateRadioBox(Widget parent, char *name);

/* creates a rowcolumn widget */
extern Widget CreateRowColumn(Widget parent, char *name);

/* Creates and returns a frame widget with name specified by framename.
 * If frametitlename is not NULL, a label widget is created with that name,
 * which is a child of the frame with type XmFRAME_TITLE_CHILD. */
extern Widget CreateFrame(Widget parent, 
			  char *framename, 
			  char *frametitlename);

/* Creates and returns a toggle button with given name and parent widget. The initial
 * state will be "SET" if initially_set is TRUE or "UNSET" if FALSE. If
 * 'toggle_callback' is not null, it will be called with the given client_data
 * each time the toggle button changes state. If 'toggle_callback' is
 * null, but 'client_data' is not null, a default toggle callback is
 * installed that interprets 'client_data' as a pointer to an int and
 * will fill in 1 or 0 according to the state of the toggle button. */
extern Widget CreateToggleButton(Widget parent,
				 char *name,
				 Boolean initially_set,
				 void (*toggle_callback)(Widget, XtPointer, XtPointer),
				 XtPointer client_data);

/* Creates a push button with given widget name, parent widget, activate callback
 * (no activate callback if NULL is passed), and client data for the activate 
 * callback procedure. */
extern Widget CreatePushButton(Widget parent,
			       char *widgetname, 
			       void (*activate_callback)(Widget, XtPointer, XtPointer),
			       XtPointer client_data);

/* Creates and returns a label widget with given name */
extern Widget CreateLabel(Widget parent, char *name);

/* returns the message string displayed on the label widget. 
 * The string should be no longer than MAX_LABEL_STRING_LENGTH 
 * characters! */
extern char *GetLabelString(Widget label);

/* sets the message displayed on a label widget */
extern void SetLabelString(Widget label, char *message);

/* Creates and returns a new toplevel shell widget with given 
 * window title. Use XtRealizeWidget(the_shell_widget) to bring
 * the separate window to the screen and XtDestroyWidget(the_shell_widget) 
 * to remove it again. */
extern Widget CreateShell(char *title);

/* Creates and returns a menu bar with given name and parent widget */
extern Widget CreateMenuBar(Widget parent, char *menuname);

/* Creates a popup menu (no button involved), position with XmMenuPosition()
 * and pop up with XtManageChild(). */
extern Widget CreatePopupMenu(Widget parent, char *menuname);

/* Creates a cascade button and corresponding pulldown menu with given
 * parent and names. */
extern Widget CreateSubMenu(Widget parent, char *buttonname, char *menuname);

/* Creates a cascade button and corresponding help pulldown menu with given
 * parent and names. (sets also the XmNmenuHelpWidget resource) */
extern Widget CreateHelpSubMenu(Widget parent, char *buttonname, char *menuname);

/* Creates a button with name buttonname and a dialog that will be popped up 
 * when pressing the button. If non-null, the 'create_dialog' function creates 
 * and returns the dialog to be popped up. The button widget is passed as the first 
 * argument to this function. The first argument is the parent widget of the dialog 
 * to be created. The second argument is the dialogname. If 'create_dialog'
 * is not specified (null), a template dialog with name 'dialogname' will be created.
 * If a popup callback is given, it will be responsible for popping
 * up the dialog. If no one is given, a default popup callback,
 * which just popups the dialog, is used. If a popup callback is specified,
 * but no client data, the dialog widget will be passed as client data.
 * This function returns the created dialog widget. */
extern Widget CreateCascadeDialog(Widget parent,
				  char *buttonname,
				  Widget (*create_dialog)(Widget, char *),
				  char *dialogname,
				  void (*popup_callback)(Widget, XtPointer, XtPointer),
				  XtPointer client_data);

/* form entry types for CreateFormEntry() */
typedef enum {FET_SHORT, FET_INTEGER, FET_LONG, 
	      FET_UNSIGNED_SHORT, FET_UNSIGNED, FET_UNSIGNED_LONG,
	      FET_FLOAT, FET_DOUBLE, 
	      FET_STRING/*space delimited*/} FORMENTRYTYPE;

/* Creates a form entry: a label and text field widget pair that can be used
 * to construct forms. The label widget, indicating the meaning of the value that 
 * can be edited in the text field widget, is optional. If no such label
 * widget is wanted, 'labelname' should be a NULL pointer. 'textfname' is the name
 * of the text field widget. 'type' determines the type of the value in the text field.
 * 'pvalue' is a pointer to the value to be displayed in the text field.
 * The value is updated each time after text is deleted or inserted from/into the text
 * field. 'valid' will contain True if the text field has valid contents
 * and False if not. The text field contents are considered to be valid if
 * sscanf(textfieldcontents, format, pvalue) returns 1. 'fieldlength' determines the
 * width (in characters) of the text field. The field length should be large enough to
 * contain the initial contents of the field. 'fieldlength' can be zero, in which case 
 * the default is used (250 characters buffer for holding the initial contents
 * of the field and 20 characters width text field widget). If 'labelname' is not NULL, 
 * the label and text field widget are packed together in a form with
 * name 'formEntry'. The widget returned is the text field widget is 'labelname'
 * is NULL, or the form widget containing both the label and text field widget
 * is 'labelname' is not NULL. */
extern Widget CreateFormEntry(Widget parent, char *labelname, char *textfname, 
		       FORMENTRYTYPE type, XtPointer pvalue, Boolean *valid, 
		       int fieldlength);

/* Creates a prompt dialog with given parent and name, asking for a
 * value of specified type to be filled in in pvalue. valid will contain
 * TRUE if the value that was fille din is valid and FALSE if invalid.
 * If fieldlength is not zero, it defines the length of the editable text
 * field in the prompt dialog. */
extern Widget CreatePromptDialog(Widget parent, char *name,
				 FORMENTRYTYPE type, XtPointer pvalue, 
				 Boolean *valid, int fieldlength);

/* Creates and returns a file selection dialog with given parent and
 * name. If a 'process_file' function is specified, an okCallback
 * function is installed that will try to open the file with specified
 * 'open_mode', call the 'process_file' function passing on
 * the file name, the FILE pointer (possibly NULL if no filename is 
 * entered), a flag indicating whether the file is a pipe or a regular 
 * file and the file selection box Widget. Finally the opened file is
 * closed and the file selection box unmanaged if the 'process_file'
 * function returned nonzero. The file name suffices .gz and .Z
 * are understood, and when the file name starts with a '|', it is
 * interpreted as a command to open a pipe to/from. */
extern Widget CreateFileSelectionDialog(Widget parent, char *name, 
					int (*process_file)(char *fname, FILE *fp, int ispipe, Widget fsbox), char *open_mode);

/* unmanages a selection box child (e.g. for a prompt dialog created with
 * CreatePromptDialog() or a file selection dialog. */
extern void UnmanageSelectionBoxChild(Widget dialog, unsigned char child);

/* manages a selection box child */
extern void ManageSelectionBoxChild(Widget dialog, unsigned char child);

/* Creates an option menu and corresponding submenu with given parent and
 * names. The OptionMenu widget (the button) is returned. Use 
 * CreateOptionButton() to create buttons in the menu. */
extern Widget CreateOptionMenu(Widget parent, char *buttonname, char *submenuname);

/* Creates a push button for an option menu. If 'set' is TRUE, the button 
 * will be the initial choice. The parent widget should be an
 * option menu. */
extern Widget CreateOptionButton(Widget optionmenu, char *buttonname,
				 Boolean set,
				 void (*activate_callback)(Widget, XtPointer, XtPointer),
				 XtPointer client_data);

/* Creates and returns a separator widget with given name */
extern Widget CreateSeparator(Widget parent, char *name);

/* Enable/Disable widget state changes */
extern void EnableWidget(Widget widget);
extern void DisableWidget(Widget widget);

/* Creates a scrolled window widget with given parent widget and name.
 * If 'create_child' is NULL, a XmLabel widget with name 'childname' is created
 * as the work window child. Otherwise, 'create_child' is called
 * in order to create a more complex child. */
extern Widget CreateScrolledWindow(Widget parent, char *name, 
				   Widget (*create_child)(Widget parent, char *childname), char *child_name);



/* CreatFileAndBrowseEntry : creates a formentry with a button
   next to it. The formentry holds a file name that can be chosen
   using the browse button. The browse button spawns a file
   selection dialog.
   !!! labelname currently unused !!!!
*/
Widget CreateFileAndBrowseEntry(Widget parent, 
				char *labelname, char *textfname,
				XtPointer pvalue, 
				Boolean *valid, int fieldlength);

#ifdef __cplusplus
}
#endif

#endif /*_UIT_H_*/
