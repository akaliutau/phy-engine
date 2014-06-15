

#include "ui.h"
#include "ui_sdk.h"
#include <Xm/RowColumn.h>

#include "canvas.h"
#include "patch_type.h"
#include "ui_material.h"
#include "error.h"

static Widget canvasPopupMenu = (Widget)NULL;
static PATCH *the_patch;
static POINT the_point;

static void EditMaterialCallback(Widget button, XtPointer client_data, XtPointer call_data)
{
  if (!the_patch) 
    return;
#ifdef EDITMATERIAL
  EditMaterial(the_patch->surface->material);
#endif
}

void CreateCanvasPopupMenu(void)
{
#ifdef EDITMATERIAL
  Widget materialEditorDialog;
#endif

  canvasPopupMenu = CreatePopupMenu(canvas, "canvasPopupMenu");

#ifdef EDITMATERIAL
  materialEditorDialog = CreateCascadeDialog(canvasPopupMenu, "editMaterialButton", 
					     CreateMaterialEditor, "editMaterialDialog", EditMaterialCallback, (XtPointer)NULL);
#else
  Warning(NULL, "the material editor is out of order");
#endif
}

void PopUpCanvasMenu(XEvent *event, PATCH *patch, POINT *point)
{
  if (!canvasPopupMenu)
    return;

  the_patch = patch;
  the_point = *point;
  XmMenuPosition(canvasPopupMenu, (XButtonPressedEvent *)event);
  XtManageChild(canvasPopupMenu);
}
