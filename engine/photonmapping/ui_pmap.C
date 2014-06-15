

#include <stdio.h>
#include <Xm/MessageB.h>

#include "pmap.h"
#include "pmapP.h"
#include "pmapoptions.H"
#include "ui.h"
#include "ui_sdk.h"	
#include "select.h"
#include "error.h"

#include <canvas.h>
#include <render.h>



static Widget pmapControlPanel = (Widget)NULL;



static Widget pmapConstantRD;
static Widget pmapImportanceRD;

static void DoBalanceGlobal(Widget w, XtPointer client_data, XtPointer call_data)
{
  PmapBalance(GLOBAL_MAP);
}

static void DoRaycast(Widget w, XtPointer client_data, XtPointer call_data)
{
  PmapRaycast();
}

static void DoRaycastInterrupt(Widget w, XtPointer client_data, XtPointer call_data)
{
  PmapRaycastInterrupt();
}

static void DoRedisplayRaycast(Widget w, XtPointer client_data, 
			       XtPointer call_data)
{
  PmapRedisplayRaycast();
}


static void DoPrintValue(Widget w, XtPointer client_data, 
			       XtPointer call_data)
{
  fprintf(stderr, "Select a pixel ... "); fflush(stderr);
  SelectPatchSetCallback(PmapDoPrintValue);
  CanvasPushMode(CANVASMODE_SELECT_PATCH);
}

static void Reinit(Widget w, XtPointer client_data, XtPointer call_data)
{
  if (CanvasGetMode() != CANVASMODE_WORKING) {
    CanvasPushMode(CANVASMODE_WORKING);
    SetRadianceMethod(&Pmap);
    CanvasPullMode();
    RenderScene();
  } else 
    Error(NULL, "Interrupt computations first before re-initializing");
}


static void ToggleCallback(Widget w, XtPointer client_data,
				       XtPointer call_data)
{
  int *toggle = (int *)client_data;
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set;
  if (set == XmSET)
    *toggle = TRUE;
  else
    *toggle = FALSE;    
}

static void ToggleSamplerCallback(Widget w, XtPointer client_data,
				       XtPointer call_data)
{
  ToggleCallback(w, client_data, call_data);
  PmapChooseSurfaceSampler();
}

static void PrecomputeCallback(Widget w, XtPointer client_data,
				       XtPointer call_data)
{
  Warning("Precompute Irradiance for global map", "REINIT Photon Maps!!");
  ToggleCallback(w, client_data, call_data);
}



static void DCWidgetStatus(DENSITY_CONTROL_OPTION dco)
{
  pmapstate.densityControl = dco;
  
  
  
  switch(dco)
  {
  case NO_DENSITY_CONTROL:
    DisableWidget(pmapConstantRD);
    DisableWidget(pmapImportanceRD);
    break;
  case CONSTANT_RD:
    EnableWidget(pmapConstantRD);
    DisableWidget(pmapImportanceRD);
    break;
  case IMPORTANCE_RD:
    DisableWidget(pmapConstantRD);
    EnableWidget(pmapImportanceRD);
    break;
  }
}

static void DCOptionCallback(Widget w, XtPointer client_data,
			      XtPointer call_data)
{
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set;
  DENSITY_CONTROL_OPTION dco = (DENSITY_CONTROL_OPTION)(int)client_data;
  if (set == XmSET)
  {
    DCWidgetStatus(dco);
  }
}




static void ImpOptionCallback(Widget w, XtPointer client_data,
			      XtPointer call_data)
{
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set;
  IMPORTANCE_OPTION dco = (IMPORTANCE_OPTION)(int)client_data;
  if (set == XmSET)
  {
    pmapstate.importanceOption = dco;
  }    
}


static void RadReturnCallback(Widget w, XtPointer client_data,
			      XtPointer call_data)
{
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set;
  if (set == XmSET)
    pmapstate.radianceReturn = (RADRETURN_OPTION)(int)client_data;
}



void CreatePmapControlPanel(void *parent_widget)
{
  Widget form, frame, columns, temp;
  Widget subform, subsubform, togglebox;

  pmapControlPanel = CreateDialog((Widget)parent_widget, "pmapControlPanel");

  form = CreateRowColumn(pmapControlPanel, "pmapForm");
  CreateLabel(form, "pmapTitle");
  CreateSeparator(form, "pmapSeparator");

  columns = CreateRowColumn(form, "pmapColumns");

  

  frame = CreateFrame(columns, "pmapStandardFrame", "pmapStandardTitle");

  subform = CreateRowColumn(frame, "pmapStandardForm");

  CreateToggleButton(subform, "pmapDoGlobalMap", 
		     (pmapstate.doGlobalMap), 
		     ToggleCallback, (XtPointer)&pmapstate.doGlobalMap);

  CreateFormEntry(subform,"pmapGSamplesPerIteration", 
		  "pmapGSamplesPerIterationTextf", 
		  FET_UNSIGNED, (XtPointer)&(pmapstate.gpaths_per_iteration),
		  NULL,0);  

  CreateToggleButton(subform, "pmapDoCausticMap", 
		     (pmapstate.doCausticMap), 
		     ToggleCallback, (XtPointer)&pmapstate.doCausticMap);

  CreateFormEntry(subform,"pmapCSamplesPerIteration", 
		  "pmapCSamplesPerIterationTextf", 
		  FET_UNSIGNED, (XtPointer)&(pmapstate.cpaths_per_iteration),
		  NULL,0);  

  CreateSeparator(subform, "pmapSeparator");

  CreateToggleButton(subform, "pmapUsePhotonMapSampler", 
		     (pmapstate.usePhotonMapSampler), 
		     ToggleSamplerCallback, (XtPointer)&pmapstate.usePhotonMapSampler);

  CreateFormEntry(subform,"pmapMaxPathLength", 
		  "pmapMaxPathLengthTextf", 
		  FET_UNSIGNED, (XtPointer)&(pmapstate.maximumLightPathDepth),
		  NULL,0);  

  CreateToggleButton(subform, "pmapRenderImage", 
		     (pmapstate.renderImage), 
		     ToggleCallback, (XtPointer)&pmapstate.renderImage);

  CreateSeparator(subform, "pmapSeparator");

  frame = CreateFrame(subform, "pmapReconstructionFrame", 
		      "pmapReconstructionTitle");
  subsubform = CreateRowColumn(frame, "pmapReconstructionForm");

  // CreatePushButton(subsubform, "pmapDoBalanceGTree", DoBalanceGlobal, (XtPointer)NULL);

  CreateLabel(subsubform, "pmapReconLabel");

  CreateFormEntry(subsubform,"pmapReconGPhotons", 
		  "pmapReconGPhotonsTextf", 
		  FET_UNSIGNED, (XtPointer)&(pmapstate.reconGPhotons),
		  NULL,0);  
  CreateFormEntry(subsubform,"pmapReconCPhotons", 
		  "pmapReconCPhotonsTextf", 
		  FET_UNSIGNED, (XtPointer)&(pmapstate.reconCPhotons),
		  NULL,0);  
  CreateFormEntry(subsubform,"pmapReconIPhotons", 
		  "pmapReconIPhotonsTextf", 
		  FET_UNSIGNED, (XtPointer)&(pmapstate.reconIPhotons),
		  NULL,0);  
  CreateFormEntry(subsubform,"pmapDistribPhotons", 
		  "pmapDistribPhotonsTextf", 
		  FET_UNSIGNED, (XtPointer)&(pmapstate.distribPhotons),
		  NULL,0);  

  CreateSeparator(subsubform, "pmapSeparator");

  CreateToggleButton(subsubform, "pmapBalanceKDTree", 
		     (pmapstate.balanceKDTree), 
		     ToggleCallback, (XtPointer)&pmapstate.balanceKDTree);
  CreateToggleButton(subsubform, "pmapPrecomputeGIrradiance", 
		     (pmapstate.precomputeGIrradiance), 
		     PrecomputeCallback, (XtPointer)&pmapstate.precomputeGIrradiance);


  XtManageChild(subsubform);

  XtManageChild(subform);


  

  frame = CreateFrame(columns, "pmapDCFrame", "pmapDCTitle");

  subform = CreateRowColumn(frame, "pmapDCForm");

  togglebox = CreateRadioBox(subform,"pmapDensityControl");
  CreateToggleButton(togglebox, "pmapDensityControlNo", 
		     (pmapstate.densityControl == NO_DENSITY_CONTROL), 
		     DCOptionCallback, (XtPointer)NO_DENSITY_CONTROL);
  CreateToggleButton(togglebox, "pmapDensityControlConstant", 
		     (pmapstate.densityControl == CONSTANT_RD), 
		     DCOptionCallback, (XtPointer)CONSTANT_RD);
  CreateToggleButton(togglebox, "pmapDensityControlImportance", 
		     (pmapstate.densityControl == IMPORTANCE_RD), 
		     DCOptionCallback, (XtPointer)IMPORTANCE_RD);
  XtManageChild(togglebox);

  frame = CreateFrame(subform, "pmapDCConstantFrame", "pmapDCConstantTitle");

  pmapConstantRD = frame;

  subsubform = CreateRowColumn(frame, "pmapDCConstantForm");

  

  CreateFormEntry(subsubform,"pmapConstantRD", 
		  "pmapConstantRDTextf", 
		  FET_FLOAT, (XtPointer)&(pmapstate.constantRD),
		  NULL,0);  

  XtManageChild(subsubform);

  frame = CreateFrame(subform, "pmapDCImportanceFrame", 
		      "pmapDCImportanceTitle");

  pmapImportanceRD = frame;

  subsubform = CreateRowColumn(frame, "pmapDCImportanceForm");
  
  CreateToggleButton(subsubform, "pmapDoImportanceMap", 
		     (pmapstate.doImportanceMap), 
		     ToggleCallback, (XtPointer)&pmapstate.doImportanceMap);

  CreateFormEntry(subsubform,"pmapISamplesPerIteration", 
		  "pmapISamplesPerIterationTextf", 
		  FET_UNSIGNED, (XtPointer)&(pmapstate.ipaths_per_iteration),
		  NULL,0);  



  CreateFormEntry(subsubform,"pmapMinimumImpRD", 
		  "pmapMinimumImpRDTextf", 
		  FET_FLOAT, (XtPointer)&(pmapstate.minimumImpRD),
		  NULL,0);  

  //  CreateFormEntry(subsubform,"pmapPotScale", 
  //		  "pmapPotScaleTextf", 
  //		  FET_FLOAT, (XtPointer)&(pmapstate.potScale),
  //		  NULL,0);  

  CreateFormEntry(subsubform,"pmapCImpScale", 
		  "pmapCImpScaleTextf", 
		  FET_FLOAT, (XtPointer)&(pmapstate.cImpScale),
		  NULL,0);  

  CreateFormEntry(subsubform,"pmapGImpScale", 
		  "pmapGImpScaleTextf", 
		  FET_FLOAT, (XtPointer)&(pmapstate.gImpScale),
		  NULL,0);  


  XtManageChild(subsubform);

  XtManageChild(subform);


  

  frame = CreateFrame(columns, "pmapTestFrame", "pmapTestTitle");

  subform = CreateRowColumn(frame, "pmapTestForm");

  togglebox = CreateRadioBox(subform,"pmapRadReturn");
  CreateToggleButton(togglebox, "pmapRadReturnGlobal", 
		     (pmapstate.radianceReturn == GLOBAL_DENSITY), 
		     RadReturnCallback, (XtPointer)GLOBAL_DENSITY);
  CreateToggleButton(togglebox, "pmapRadReturnCaustic", 
		     (pmapstate.radianceReturn == CAUSTIC_DENSITY), 
		     RadReturnCallback, (XtPointer)CAUSTIC_DENSITY);
  CreateToggleButton(togglebox, "pmapRadReturnCImportance", 
		     (pmapstate.radianceReturn == IMPORTANCE_CDENSITY), 
		     RadReturnCallback, (XtPointer)IMPORTANCE_CDENSITY);
  CreateToggleButton(togglebox, "pmapRadReturnGImportance", 
		     (pmapstate.radianceReturn == IMPORTANCE_GDENSITY), 
		     RadReturnCallback, (XtPointer)IMPORTANCE_GDENSITY);
  CreateToggleButton(togglebox, "pmapRadReturnRecCDensity", 
		     (pmapstate.radianceReturn == REC_CDENSITY), 
		     RadReturnCallback, (XtPointer)REC_CDENSITY);
  CreateToggleButton(togglebox, "pmapRadReturnRecGDensity", 
		     (pmapstate.radianceReturn == REC_GDENSITY), 
		     RadReturnCallback, (XtPointer)REC_GDENSITY);

  CreateToggleButton(togglebox, "pmapRadReturnGlobalRadiance",
		     (pmapstate.radianceReturn == GLOBAL_RADIANCE), 
		     RadReturnCallback, (XtPointer)GLOBAL_RADIANCE);
  CreateToggleButton(togglebox, "pmapRadReturnCausticRadiance",
		     (pmapstate.radianceReturn == CAUSTIC_RADIANCE), 
		     RadReturnCallback, (XtPointer)CAUSTIC_RADIANCE);
  XtManageChild(togglebox);



 
  
  CreateSeparator(subform, "pmapSeparator");

  CreateToggleButton(subform, "pmapFalseColLog", 
		     (pmapstate.falseColLog), 
		     ToggleCallback, (XtPointer)&pmapstate.falseColLog);

  CreateToggleButton(subform, "pmapFalseColMono", 
		     (pmapstate.falseColMono), 
		     ToggleCallback, (XtPointer)&pmapstate.falseColMono);

  CreateFormEntry(subform,"pmapFalseColMax", 
		  "pmapFalseColMaxTextf", 
		  FET_FLOAT, (XtPointer)&(pmapstate.falseColMax),
		  NULL,0);  

  

  CreateSeparator(subform, "pmapSeparator");
  CreatePushButton(subform, "pmapRaycastButton", DoRaycast, (XtPointer)NULL);
  CreatePushButton(subform, "pmapRaycastInterruptButton", DoRaycastInterrupt, 
		   (XtPointer)NULL);
  CreatePushButton(subform, "pmapRedisplayRaycastButton", DoRedisplayRaycast, 
		   (XtPointer)NULL);
  //CreatePushButton(subform, "pmapPrintValueButton", DoPrintValue, 
  //	   (XtPointer)NULL);


  XtManageChild(subform);

  XtManageChild(columns);
  XtManageChild(form);

  
  temp = XmMessageBoxGetChild(pmapControlPanel, XmDIALOG_CANCEL_BUTTON);
  XtUnmanageChild(temp);

  CreatePushButton(pmapControlPanel, "pmapReinitButton", Reinit, (XtPointer)NULL);

  
  DCWidgetStatus(pmapstate.densityControl);
}

void ShowPmapControlPanel(void)
{
  if (pmapControlPanel)
    XtManageChild(pmapControlPanel);
  else
    Error("ShowPmapControlPanel", "Control panel not created yet");
}

void HidePmapControlPanel(void)
{
  if (pmapControlPanel)
    XtUnmanageChild(pmapControlPanel);
  else
    Error("HidePmapControlPanel", "Control panel not created yet");
}
