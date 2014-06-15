

#include <stdio.h>

#include "ui.h"
#include "ui_sdk.h"
#include "radiance.h"
#include "statistics.h"
#include "scene.h"
#include "canvas.h"
#include "render.h"
#include "error.h"





static void SetRadianceMethodCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set;
  if (set == XmSET) {
    CanvasPushMode(CANVASMODE_WORKING);
    
    SetRadianceMethod((RADIANCEMETHOD *)client_data);
    CanvasPullMode();
    RenderNewDisplayList();
    RenderScene();
  }
}

static void CreateRadianceMethodMenu(Widget parent)
{
  Widget methodMenu = CreateSubMenu(parent, "radianceMethodButton", "radianceMethodMenu");

  XtVaSetValues(methodMenu,
		XmNradioBehavior, True,
		NULL);

  CreateToggleButton(methodMenu, "NoRadianceButton",
		     (!Radiance ? TRUE : FALSE),
		     SetRadianceMethodCallback, (XtPointer)NULL);

  ForAllRadianceMethods(method) {
    CreateToggleButton(methodMenu, method->buttonName, 
		       (method == Radiance ? TRUE : FALSE),
		       SetRadianceMethodCallback, (XtPointer)method);
  } EndForAll;
}


static Widget controlPanelParent;	

static void CreateRadianceControlPanels(Widget parent)
{
  controlPanelParent = parent;

  ForAllRadianceMethods(method) {
    method->CreateControlPanel((void *)parent);
  } EndForAll;
}


static int radiance_running;
int interrupt_radiance;
void UpdateRadianceStats(void);


int DoRadianceOneStep(void)
{
  int done;

  renderopts.render_raytraced_image = FALSE;
  if (!World || !Radiance)
    return TRUE;

  CanvasPushMode(CANVASMODE_WORKING);
  done = Radiance->DoStep();
  CanvasPullMode();

  UpdateRadianceStats();
  RenderNewDisplayList();
  RenderScene();

  return done;
}


static void RadianceRunStopCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  if (radiance_running)
    
    interrupt_radiance = TRUE;

  else {
    int done;

    if (!Radiance)
      Error(NULL, "Specify a radiance method first");

    radiance_running = TRUE;
    interrupt_radiance = FALSE;

    ProcessWaitingEvents();

    done = FALSE;
    while (!interrupt_radiance && !done) {
      done = DoRadianceOneStep();
      ProcessWaitingEvents();
    }

    radiance_running = FALSE;
    interrupt_radiance = FALSE;
  }
}

static void RadianceOneIterationCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  if (Radiance) {
    interrupt_radiance = FALSE;
    radiance_running = TRUE;

    DoRadianceOneStep();

    interrupt_radiance = FALSE;
    radiance_running = FALSE;
  } else
    Error(NULL, "Specify a radiance method first");
}


static void RadianceControlCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  if (Radiance) {
    if (Radiance->UpdateControlPanel) 
      Radiance->UpdateControlPanel(controlPanelParent);
    Radiance->ShowControlPanel();
  } else
    Error(NULL, "Specify a radiance method first");
}



static Widget radianceStatsMessage = (Widget)NULL;


void UpdateRadianceStats(void)
{
  char *message;

  if (!Radiance || !(message=Radiance->GetStats())) 
    message = "No statistics information available";

  SetLabelString(radianceStatsMessage, message);
}


static void UpdateRadianceStatsCallback(Widget statsBox, XtPointer client_data, XtPointer call_data)
{
  UpdateRadianceStats();
}


static void DismissStatsCallback(Widget statsBox, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild(statsBox);
}


static Widget CreateRadianceStatsDialog(Widget parent, char *name)
{
  Widget statsBox = CreateDialog(parent, name);

  
  radianceStatsMessage = CreateLabel(statsBox, "radianceStatsMessage");

  
  UpdateRadianceStats();

  
  XtAddCallback(statsBox, XmNokCallback, UpdateRadianceStatsCallback, (XtPointer)NULL);
  XtAddCallback(statsBox, XmNcancelCallback, DismissStatsCallback, (XtPointer)NULL);

  return statsBox;
}


static void ShowRadianceStats(Widget statsButton, XtPointer client_data, XtPointer call_data)
{
  Widget statsBox = (Widget)client_data;

  UpdateRadianceStats();
  XtManageChild(statsBox);
}


void CreateRadianceMenu(Widget menuBar)
{
  Widget radianceMenu, radianceControlButton;

  radianceMenu = CreateSubMenu(menuBar, "radianceButton", "radianceMenu");

  CreateRadianceMethodMenu(radianceMenu);  
  radianceControlButton = CreatePushButton(radianceMenu, "radianceControlButton", RadianceControlCallback, (XtPointer)NULL);
  CreateRadianceControlPanels(radianceControlButton);
  CreateSeparator(radianceMenu, "radianceSeparator");

  CreatePushButton(radianceMenu, "radianceOneIterationButton", RadianceOneIterationCallback, (XtPointer)NULL);
  CreatePushButton(radianceMenu, "radianceRunStopButton", RadianceRunStopCallback, (XtPointer)NULL);
  radiance_running = FALSE;
  CreateSeparator(radianceMenu, "radianceSeparator");

  CreateCascadeDialog(radianceMenu, "statsButton", CreateRadianceStatsDialog, "statsBox", ShowRadianceStats, NULL);
}
