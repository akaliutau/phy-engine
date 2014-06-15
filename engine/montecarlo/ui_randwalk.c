

#include "mcrad.h"
#include "mcradP.h"
#include "ui_mcrad.h"
#include "ui.h"
#include "ui_sdk.h"
#include "error.h"
#include "render.h"

static Widget rwrControlPanel = (Widget)NULL;

static void CreateVariaMenu(Widget parent)
{
  Widget subform = CreateRowColumn(parent, "mcrVariaForm");
  CreateFormEntry(subform, "mcrRayUnits", "mcrRayUnitsTextf",
		  FET_INTEGER, (XtPointer)&mcr.ray_units_per_it,
		  NULL, 0);

  CreateToggleButton(subform, "rwrContinuousButton",
		     mcr.continuous_random_walk ? True : False,
		     NULL, (XtPointer)&mcr.continuous_random_walk);

  CreateToggleButton(subform, "mcrConstControlVButton",
		     mcr.constant_control_variate ? True : False,
		     NULL, (XtPointer)&mcr.constant_control_variate);

  CreateToggleButton(subform, "mcrIndirectOnlyButton",
		     mcr.indirect_only ? True : False,
		     NULL, (XtPointer)&mcr.indirect_only);
#ifdef HACK
  CreateToggleButton(subform, "hack",
		     mcr.hack ? True : False,
		     NULL, (XtPointer)&mcr.hack);
#endif 
  XtManageChild(subform);
}

static void EstimatorTypeCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  mcr.rw_estimator_type = (RW_ESTIMATOR_TYPE)client_data;
}

static void CreateEstimatorTypeMenu(Widget parent)
{
  Widget subform = CreateRadioBox(parent, "estimatorTypeBox");

  CreateToggleButton(subform, "rwrShootingButton",
		     mcr.rw_estimator_type == RW_SHOOTING ? True : False,
		     EstimatorTypeCallback, (XtPointer)RW_SHOOTING);

  CreateToggleButton(subform, "rwrGatheringButton",
		     mcr.rw_estimator_type == RW_GATHERING ? True : False,
		     EstimatorTypeCallback, (XtPointer)RW_GATHERING);

  XtManageChild(subform);
}

static void EstimatorKindCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  mcr.rw_estimator_kind = (RW_ESTIMATOR_KIND)client_data;
}

static void CreateEstimatorKindMenu(Widget parent)
{
  Widget form = CreateRowColumn(parent, "estimatorKindGroupBox");
  Widget subform = CreateRadioBox(form, "estimatorKindBox");

  CreateToggleButton(subform, "rwrCollisionButton",
		     mcr.rw_estimator_kind == RW_COLLISION ? True : False,
		     EstimatorKindCallback, (XtPointer)RW_COLLISION);

  CreateToggleButton(subform, "rwrAbsorptionButton",
		     mcr.rw_estimator_kind == RW_ABSORPTION ? True : False,
		     EstimatorKindCallback, (XtPointer)RW_ABSORPTION);

  CreateToggleButton(subform, "rwrSurvivalButton",
		     mcr.rw_estimator_kind == RW_SURVIVAL ? True : False,
		     EstimatorKindCallback, (XtPointer)RW_SURVIVAL);

  CreateToggleButton(subform, "rwrLastButNthButton",
		     mcr.rw_estimator_kind == RW_LAST_BUT_NTH ? True : False,
		     EstimatorKindCallback, (XtPointer)RW_LAST_BUT_NTH);

  CreateToggleButton(subform, "rwrNLastButton",
		     mcr.rw_estimator_kind == RW_NLAST ? True : False,
		     EstimatorKindCallback, (XtPointer)RW_NLAST);

  XtManageChild(subform);

  CreateFormEntry(form, "rwrN", "rwrNTextf",
		  FET_INTEGER, (XtPointer)&mcr.rw_numlast,
		  NULL, 0);

  XtManageChild(form);
}

void RwrCreateControlPanel(void *parent_widget)
{
  Widget form, left, right, frame;

  rwrControlPanel = CreateDialog((Widget)parent_widget, "rwrControlPanel");
  form = CreateRowColumn(rwrControlPanel, "rwrForm");
  left = CreateRowColumn(form, "rwrLeft");
  right = CreateRowColumn(form, "rwrRight");

  CreateLabel(left, "rwrTitle");

  frame = CreateFrame(left, "mcrVariaFrame", "mcrVariaTitle");
  CreateVariaMenu(frame);

  frame = CreateFrame(left, "samplingFrame", "samplingTitle");
  McrCreateSamplingMenu(frame);

#ifdef HOMCR
  frame = CreateFrame(left, "basisFrame", "basisTitle");
  McrCreateBasisMenu(frame);
#endif 

  frame = CreateFrame(right, "rwrEstimatorTypeFrame", "rwrEstimatorTypeTitle");
  CreateEstimatorTypeMenu(frame);

  frame = CreateFrame(right, "rwrEstimatorKindFrame", "rwrEstimatorKindTitle");
  CreateEstimatorKindMenu(frame);

  XtManageChild(left);
  XtManageChild(right);
  XtManageChild(form);

  
  UnmanageMessageBoxChild(rwrControlPanel, XmDIALOG_CANCEL_BUTTON);
}

void RwrShowControlPanel(void)
{
  if (rwrControlPanel)
    XtManageChild(rwrControlPanel);
  else
    Error("RwrShowControlPanel", "Control panel not created yet");
}

void RwrHideControlPanel(void)
{
  if (rwrControlPanel)
    XtUnmanageChild(rwrControlPanel);
  else
    Error("RwrHideControlPanel", "Control panel not created yet");
}

void RwrUpdateControlPanel(void *parent_widget)
{
  if (rwrControlPanel) {
    XtUnmanageChild(rwrControlPanel);
    XtDestroyWidget(rwrControlPanel);
    RwrCreateControlPanel(parent_widget);
  }
}
