

#include "ui.h"
#include "ui_sdk.h"
#include <Xm/MessageB.h>

#include "error.h"
#include "rmoptions.H"
#include "raymatting.h"
#include "raymatting_priv.h"

static Widget rmControlPanel;


void RM_ShowControlPanel(void)
{
  if (rmControlPanel)
    XtManageChild(rmControlPanel);
  else
    Error("RM_ShowControlPanel", "Control panel not created yet");
}

void RM_HideControlPanel(void)
{
  if (rmControlPanel)
    XtUnmanageChild(rmControlPanel);
}



static void FilterCallback(Widget w, XtPointer client_data,
			    XtPointer call_data)
{
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set;
  if (set == XmSET)
    rms.filter = (RM_FILTER_OPTION)client_data;
}




void CreateRMControlPanel(void *parent_widget)
{
  Widget parent = (Widget)parent_widget;
  Widget rmForm, rmForm2, frame, frame2, togglebox, temp;

  rmControlPanel = CreateDialog(parent, "rmControlPanel");
  rmForm = CreateRowColumn(rmControlPanel, "rmForm");

  frame = CreateFrame(rmForm, "rmFrame", 
		      "rmTitle");

  rmForm2 = CreateRowColumn(frame, "rtsLeftForm");

  CreateFormEntry(rmForm2,"rmPixelSamples", "rmPixelSamplesTextf", 
		  FET_UNSIGNED, (XtPointer)&(rms.samplesPerPixel),
		  NULL,0);

  frame2 = CreateFrame(rmForm2, "rmPixelFilterFrame", 
		      "rmPixelFilterTitle");

  togglebox = CreateRadioBox(frame2,"rmFilterBox");
  CreateToggleButton(togglebox, "rmBox", 
		     (rms.filter == RM_BOX_FILTER), 
		     FilterCallback, (XtPointer)RM_BOX_FILTER);
  CreateToggleButton(togglebox, "rmTent", 
		     (rms.filter == RM_TENT_FILTER), 
		     FilterCallback, (XtPointer)RM_TENT_FILTER);
  CreateToggleButton(togglebox, "rmGauss", 
		     (rms.filter == RM_GAUSS_FILTER), 
		     FilterCallback, (XtPointer)RM_GAUSS_FILTER);
  CreateToggleButton(togglebox, "rmGauss2", 
		     (rms.filter == RM_GAUSS2_FILTER), 
		     FilterCallback, (XtPointer)RM_GAUSS2_FILTER);
  XtManageChild(togglebox);

  XtManageChild(rmForm2);
  XtManageChild(rmForm);

 
  temp = XmMessageBoxGetChild(rmControlPanel, XmDIALOG_CANCEL_BUTTON);
  XtUnmanageChild(temp);
}
