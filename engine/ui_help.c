

#include <stdlib.h>

#include "ui.h"
#include "ui_sdk.h"
#include "defaults.h"


static Widget CreateHelpDialog(Widget parent, char *name)
{
  Widget helpBox = CreateDialog(parent, name);
  UnmanageMessageBoxChild(helpBox, XmDIALOG_CANCEL_BUTTON);

  
  CreateScrolledWindow(helpBox, "helpScrollW", NULL, "helpLabel");
  return helpBox;
}


void CreateHelpMenu(Widget parent)
{
  Widget about = CreateHelpSubMenu(parent, "About", "about");
  CreateCascadeDialog(about, "Info", CreateHelpDialog, "aboutBox", DEFAULT_CALLBACK, NULL);
}



