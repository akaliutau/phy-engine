

#include "ui.h"
#include "ui_sdk.h"
#include <Xm/MessageB.h>

#include "error.h"

#include "rtstochastic.h"
#include "rtstochastic_priv.h"
#include "rtsoptions.H"

static Widget rtsControlPanel;


void RTStochastic_ShowControlPanel(void)
{
  if (rtsControlPanel)
    XtManageChild(rtsControlPanel);
  else
    Error("RTStochastic_ShowControlPanel", "Control panel not created yet");
}

void RTStochastic_HideControlPanel(void)
{
  if (rtsControlPanel)
    XtUnmanageChild(rtsControlPanel);
  
}



static void RadianceContributionCallback(Widget w, XtPointer client_data, 
					 XtPointer call_data)
{
  int set = (((XmToggleButtonCallbackStruct *)call_data)->set == XmSET);

  switch((RTSRADIANCE_OPTION)client_data)
  {
  case NEXTEVENT_RADIANCE:
    rts.nextEvent = set;
    break;
  default:
    break;
  }
}


static void BackgroundCallback(Widget w, XtPointer client_data,
			       XtPointer call_data)
{
  int set = (((XmToggleButtonCallbackStruct *)call_data)->set == XmSET);

  switch((RTSBACKGROUND_OPTION)client_data)
    {
    case BACKGROUND_SAMPLE:
      rts.backgroundSampling = set;
      break;
    case BACKGROUND_DIRECT:
      rts.backgroundDirect = set;
      break;
    case BACKGROUND_INDIRECT:
      rts.backgroundIndirect = set;
      break;
    default:
      break;
    }
}



static void ReflectionSamplingCallback(Widget w, XtPointer client_data,
				       XtPointer call_data)
{
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set;
  if (set == XmSET)
    rts.reflectionSampling = (RTSSAMPLING_MODE)client_data;
}

static void RadModeCallback(Widget w, XtPointer client_data,
			    XtPointer call_data)
{
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set;
  if (set == XmSET)
    rts.radMode = (RTSRAD_MODE)client_data;
}

static void LightModeCallback(Widget w, XtPointer client_data,
				       XtPointer call_data)
{
  //int set = ((XmToggleButtonCallbackStruct *)call_data)->set;

  rts.lightMode = (RTSLIGHT_MODE)client_data;
}


static void ProgressiveTracingCallback(Widget w, XtPointer client_data,
				       XtPointer call_data)
{
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set;
  if (set == XmSET)
    rts.progressiveTracing = TRUE;
  else
    rts.progressiveTracing = FALSE;    
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




void CreateRTStochasticControlPanel(void *parent_widget)
{
  Widget parent = (Widget)parent_widget;
  Widget rtsForm, options, frame, subform, subsubform;
  Widget rtsForm2;
  Widget rtsFormHorizontal;
  Widget temp, togglebox;

  rtsControlPanel = CreateDialog(parent, "rtsControlPanel");
  rtsForm = CreateRowColumn(rtsControlPanel, "rtsForm");

  

  CreateLabel(rtsForm, "rtsTitle");

  rtsFormHorizontal = CreateRowColumn(rtsForm, "rtsHorizontalForm");

  

  rtsForm2 = CreateRowColumn(rtsFormHorizontal, "rtsLeftForm");

  frame = CreateFrame(rtsForm2, "rtsPixelHandlingFrame", 
		      "rtsPixelHandlingTitle");

  subform = CreateRowColumn(frame, "rtsPixelHandlingForm");
  
  CreateFormEntry(subform,"rtsPixelSamples", "rtsPixelSamplesTextf", 
		  FET_UNSIGNED, (XtPointer)&(rts.samplesPerPixel),
		  NULL,0);

  CreateToggleButton(subform, "rtsProgressiveToggle",
		     rts.progressiveTracing, 
		     ProgressiveTracingCallback,
		     (XtPointer)0);
  CreateToggleButton(subform, "rtsDoFrameCoherentToggle",
		     rts.doFrameCoherent, 
		     ToggleCallback,
		     (XtPointer)&rts.doFrameCoherent);
  CreateToggleButton(subform, "rtsDoCorellatedSamplingToggle",
		     rts.doCorrelatedSampling, 
		     ToggleCallback,
		     (XtPointer)&rts.doCorrelatedSampling);
  CreateFormEntry(subform,"rtsBaseSeed", "rtsBaseSeedTextf", 
		  FET_UNSIGNED_LONG, (XtPointer)&(rts.baseSeed),
		  NULL,0);


  XtManageChild(subform);

  

  frame = CreateFrame(rtsForm2, "rtsRadianceContributionFrame", 
		      "rtsRadianceContributionTitle");


  subform = CreateRowColumn(frame, "rtsRadianceContributionForm");

  togglebox = CreateRadioBox(subform,"rtsRadModeBox");
  CreateToggleButton(togglebox, "rtsStoredNone", 
		     (rts.radMode == STORED_NONE), 
		     RadModeCallback, (XtPointer)STORED_NONE);
  CreateToggleButton(togglebox, "rtsStoredDirect", 
		     (rts.radMode == STORED_DIRECT), 
		     RadModeCallback, (XtPointer)STORED_DIRECT);
  CreateToggleButton(togglebox, "rtsStoredIndirect", 
		     (rts.radMode == STORED_INDIRECT), 
		     RadModeCallback, (XtPointer)STORED_INDIRECT);
  CreateToggleButton(togglebox, "rtsStoredPhotonmap", 
		     (rts.radMode == STORED_PHOTONMAP), 
		     RadModeCallback, (XtPointer)STORED_PHOTONMAP);
  XtManageChild(togglebox);

  CreateSeparator(subform, "rtsSeparator");

  

  temp = CreateToggleButton(subform, "rtsRadianceNextEventToggle",
			    rts.nextEvent, RadianceContributionCallback,
			    (XtPointer)NEXTEVENT_RADIANCE);

  temp = CreateToggleButton(subform, "rtsBackgroundSamplingToggle",
			    rts.backgroundSampling, BackgroundCallback,
			    (XtPointer)BACKGROUND_SAMPLE);

  frame = CreateFrame(subform, "rtsNextEventFrame", 
		      NULL);

  subsubform = CreateRowColumn(frame, "rtsNextEventForm");


  CreateFormEntry(subsubform,"rtsNextEventSamples", "rtsNextEventSamplesTextf", 
		  FET_UNSIGNED, (XtPointer)&(rts.nextEventSamples),
		  NULL,0);

  options = CreateOptionMenu(subsubform, "rtsLightModeMenu", 
			     "rtsLightModeOptions");
  CreateOptionButton(options, "rtsPowerLights", 
		     (rts.lightMode == POWER_LIGHTS),
		     LightModeCallback, (XtPointer)POWER_LIGHTS);
  CreateOptionButton(options, "rtsImportantLights", 
		     (rts.lightMode == IMPORTANT_LIGHTS),
		     LightModeCallback, (XtPointer)IMPORTANT_LIGHTS);
  CreateOptionButton(options, "rtsAllLights", 
		     (rts.lightMode == ALL_LIGHTS),
		     LightModeCallback, (XtPointer)ALL_LIGHTS);

  XtManageChild(options);
  XtManageChild(subsubform);
  XtManageChild(subform);

  XtManageChild(rtsForm2);

  rtsForm2 = CreateRowColumn(rtsFormHorizontal, "rtsRightForm");


  
  frame = CreateFrame(rtsForm2, "rtsReflectionFrame", "rtsReflectionTitle");
  subform = CreateRowColumn(frame,"rtsReflectionForm");
  CreateFormEntry(subform,"rtsReflectionSamples", "rtsReflectionSamplesTextf", 
		  FET_UNSIGNED, (XtPointer)&(rts.scatterSamples),
		  NULL,0);
  CreateToggleButton(subform, "rtsDifferentFirstDG", 
		     (rts.differentFirstDG), 
		     ToggleCallback, (XtPointer)&rts.differentFirstDG);
  CreateFormEntry(subform,"rtsFirstDGSamples", "rtsFirstDGSamplesTextf", 
		  FET_UNSIGNED, (XtPointer)&(rts.firstDGSamples),
		  NULL,0);

  CreateToggleButton(subform, "rtsSeparateSpecular", 
		     (rts.separateSpecular), 
		     ToggleCallback, (XtPointer)&rts.separateSpecular);

  CreateSeparator(subform, "rtsSeparator");

 
  togglebox = CreateRadioBox(subform,"rtsReflectionSamplingForm");

  CreateToggleButton(togglebox, "rtsReflectionBrdfButton", 
		     (rts.reflectionSampling == BRDFSAMPLING), 
		    ReflectionSamplingCallback, (XtPointer)BRDFSAMPLING);
  CreateToggleButton(togglebox, "rtsReflectionClassicalButton",
		     rts.reflectionSampling == CLASSICALSAMPLING, 
		     ReflectionSamplingCallback,
		     (XtPointer)CLASSICALSAMPLING);
  CreateToggleButton(togglebox, "rtsReflectionPhotonMapButton",
		     rts.reflectionSampling == PHOTONMAPSAMPLING, 
		     ReflectionSamplingCallback,
		     (XtPointer)PHOTONMAPSAMPLING);
  XtManageChild(togglebox);

  CreateSeparator(subform, "rtsSeparator");

  CreateFormEntry(subform,"rtsMinimumPathDepth", "rtsMinimumPathDepthTextf", 
		  FET_UNSIGNED, (XtPointer)&(rts.minPathDepth),
		  NULL,0);
  CreateFormEntry(subform,"rtsMaximumPathDepth", "rtsMaximumPathDepthTextf", 
		  FET_UNSIGNED, (XtPointer)&(rts.maxPathDepth),
		  NULL,0);

  XtManageChild(subform);

  
  frame = CreateFrame(rtsForm2, "rtsBackgroundOptionsFrame", 
		      "rtsBackgroundOptionsTitle");

  subform = CreateRowColumn(frame,"rtsBackgroundOPtionsForm");  

  temp = CreateToggleButton(subform, "rtsBackgroundDirectToggle",
			    rts.backgroundDirect, BackgroundCallback,
			    (XtPointer)BACKGROUND_DIRECT);

  temp = CreateToggleButton(subform, "rtsBackgroundIndirectToggle",
			    rts.backgroundIndirect, BackgroundCallback,
			    (XtPointer)BACKGROUND_INDIRECT);

  XtManageChild(subform);

  XtManageChild(rtsForm2);

  XtManageChild(rtsFormHorizontal);

  XtManageChild(rtsForm);

 
  temp = XmMessageBoxGetChild(rtsControlPanel, XmDIALOG_CANCEL_BUTTON);
  XtUnmanageChild(temp);

}

