

#include "mcrad.h"
#include "mcradP.h"
#include "ui_mcrad.h"
#include "ui_hierarchy.h"
#include "ui.h"
#include "ui_sdk.h"
#include "error.h"
#include "render.h"
#include "canvas.h"

static Widget srrControlPanel = (Widget)NULL;

static void ShowIt(void)
{
  CanvasPushMode(CANVASMODE_WORKING);
  Radiance->RecomputeDisplayColors();
  CanvasPullMode();
  RenderNewDisplayList();
  RenderScene();
}

static void ShowWhatCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  int set = (((XmToggleButtonCallbackStruct *)call_data)->set == XmSET);
  if (set) {
    SHOW_WHAT flag = (SHOW_WHAT)client_data;
    mcr.show = flag;
    ShowIt();
  }
}

static void CreateVariaMenu(Widget parent)
{
  Widget subform = CreateRowColumn(parent, "mcrVariaForm");
  CreateFormEntry(subform, "mcrRayUnits", "mcrRayUnitsTextf",
		  FET_INTEGER, (XtPointer)&mcr.ray_units_per_it,
		  NULL, 0);

  CreateToggleButton(subform, "srrBidirectionalButton",
		     mcr.bidirectional_transfers ? True : False,
		     NULL, (XtPointer)&mcr.bidirectional_transfers);

  CreateToggleButton(subform, "mcrConstControlVButton",
		     mcr.constant_control_variate ? True : False,
		     NULL, (XtPointer)&mcr.constant_control_variate);

#ifdef IDMCR
  CreateToggleButton(subform, "srrImportanceDrivenButton",
		     mcr.importance_driven ? True : False,
		     NULL, (XtPointer)&mcr.importance_driven);
#endif

  CreateToggleButton(subform, "mcrIndirectOnlyButton",
		     mcr.indirect_only ? True : False,
		     NULL, (XtPointer)&mcr.indirect_only);

  CreateToggleButton(subform, "mcrNonDiffuseFirstShotButton",
		     mcr.do_nondiffuse_first_shot ? True : False,
		     NULL, (XtPointer)&mcr.do_nondiffuse_first_shot);

  CreateFormEntry(subform, "mcrInitLSSamples", "mcrInitLSSamplesTextf",
		  FET_INTEGER, (XtPointer)&mcr.initial_ls_samples,
		  NULL, 0);  

#ifdef FAKE_GLOBAL_LINES
  CreateToggleButton(subform, "mcrFakeGlobalLinesButton", 
		     mcr.fake_global_lines ? True : False,
		     NULL, (XtPointer)&mcr.fake_global_lines);
#endif

  XtManageChild(subform);
}

static void CreateDisplayMenu(Widget parent)
{
  Widget form, subform;

  form = CreateRowColumn(parent, "mcrDisplayForm");
  subform = CreateRadioBox(form, "mcrDisplayForm");

  CreateToggleButton(subform, "srrShowTotalButton",
		     mcr.show == SHOW_TOTAL_RADIANCE ? True : False,
		     ShowWhatCallback, (XtPointer)SHOW_TOTAL_RADIANCE);

  CreateToggleButton(subform, "srrShowIndirectButton",
		     mcr.show == SHOW_INDIRECT_RADIANCE ? True : False,
		     ShowWhatCallback, (XtPointer)SHOW_INDIRECT_RADIANCE);

#ifdef IDMCR
  CreateToggleButton(subform, "srrShowImportanceButton",
		     mcr.show == SHOW_IMPORTANCE ? True : False,
		     ShowWhatCallback, (XtPointer)SHOW_IMPORTANCE);
#endif
  XtManageChild(subform);
  XtManageChild(form);
}

void SrrCreateControlPanel(void *parent_widget)
{
  Widget form, form2, frame, left, right;

  srrControlPanel = CreateDialog((Widget)parent_widget, "srrControlPanel");

  form = CreateRowColumn(srrControlPanel, "srrForm");

  CreateLabel(form, "srrTitle");

  form2 = CreateRowColumn(form, "srrForm");
  XtVaSetValues(form2, XmNorientation, XmHORIZONTAL, NULL);
  left = CreateRowColumn(form2, "srrForm");
  right = CreateRowColumn(form2, "srrForm");

  frame = CreateFrame(left, "mcrVariaFrame", "mcrVariaTitle");
  CreateVariaMenu(frame);

  frame = CreateFrame(left, "mcrDisplayFrame", "mcrDisplayTitle");
  CreateDisplayMenu(frame);

  
  CreateHMeshingMenu(right);

#ifdef HOMCR
  frame = CreateFrame(right, "basisFrame", "basisTitle");
  McrCreateBasisMenu(frame);
#endif 

  XtManageChild(left);
  XtManageChild(right);
  XtManageChild(form2);
  XtManageChild(form);

  
  UnmanageMessageBoxChild(srrControlPanel, XmDIALOG_CANCEL_BUTTON);
}

void SrrShowControlPanel(void)
{
  if (srrControlPanel)
    XtManageChild(srrControlPanel);
  else
    Error("SrrShowControlPanel", "Control panel not created yet");
}

void SrrHideControlPanel(void)
{
  if (srrControlPanel)
    XtUnmanageChild(srrControlPanel);
  else
    Error("SrrHideControlPanel", "Control panel not created yet");
}

void SrrUpdateControlPanel(void *parent_widget)
{
  if (srrControlPanel) {
    XtUnmanageChild(srrControlPanel);
    XtDestroyWidget(srrControlPanel);
    SrrCreateControlPanel(parent_widget);
  }
}
