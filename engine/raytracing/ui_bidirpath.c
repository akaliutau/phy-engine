

#include "ui.h"
#include "ui_sdk.h"
#include <Xm/MessageB.h>

#include "error.h"

#include "bidirpath.h"
#include "bidirpath_priv.h"

#include "bidiroptions.H"
#include "ui_bidirpresets.h"

static Widget bidirControlPanel;
static Widget bidirControlParent;
static Widget bidirSparForm;
static Widget bidirWeightedForm;

void BidirPathShowControlPanel(void)
{
  if (bidirControlPanel)
    XtManageChild(bidirControlPanel);
  else
    Error("BIDIRPATH_ShowControlPanel", "Control panel not created yet");
}

void BidirPathHideControlPanel(void)
{
  if (bidirControlPanel)
    XtUnmanageChild(bidirControlPanel);
  
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

void SparToggleCallback(Widget w, XtPointer client_data,
				       XtPointer call_data)
{
  int oldUseSpars = bidir.basecfg.useSpars;

  ToggleCallback(w, client_data, call_data);

  if(bidir.basecfg.useSpars != oldUseSpars)
  {
    if(bidir.basecfg.useSpars)
    {
      EnableWidget(bidirSparForm);
    }
    else
    {
      DisableWidget(bidirSparForm);
    }
  }
}
void WeightedToggleCallback(Widget w, XtPointer client_data,
				       XtPointer call_data)
{
  int oldWeighted = bidir.basecfg.doWeighted;

  ToggleCallback(w, client_data, call_data);

  if(bidir.basecfg.doWeighted != oldWeighted)
  {
    if(bidir.basecfg.doWeighted)
    {
      EnableWidget(bidirWeightedForm);
    }
    else
    {
      DisableWidget(bidirWeightedForm);
    }
  }
}



static void CopyPresetConfig(BP_BASECONFIG *dest, BP_BASECONFIG *preset)
{
  

  

  if(preset->maximumPathDepth != PRESET_UNCHANGED)
    dest->maximumPathDepth = preset->maximumPathDepth;

  if (preset->maximumEyePathDepth == PRESET_DERIVE)
    dest->maximumEyePathDepth = dest->maximumPathDepth;
  else if(preset->maximumEyePathDepth != PRESET_UNCHANGED)
    dest->maximumEyePathDepth = preset->maximumEyePathDepth;

  if (preset->maximumLightPathDepth == PRESET_DERIVE)
    dest->maximumLightPathDepth = dest->maximumPathDepth;
  else if(preset->maximumLightPathDepth != PRESET_UNCHANGED)
    dest->maximumLightPathDepth = preset->maximumLightPathDepth;

  if(preset->minimumPathDepth != PRESET_UNCHANGED)
    dest->minimumPathDepth = preset->minimumPathDepth;

  
  if(preset->samplesPerPixel != PRESET_UNCHANGED)
    dest->samplesPerPixel = preset->samplesPerPixel;

  if(preset->sampleImportantLights != PRESET_UNCHANGED)
    dest->sampleImportantLights = preset->sampleImportantLights;

  if(preset->progressiveTracing != PRESET_UNCHANGED)
    dest->progressiveTracing = preset->progressiveTracing;

  if(preset->eliminateSpikes != PRESET_UNCHANGED)
    dest->eliminateSpikes = preset->eliminateSpikes;

  

  if(preset->useSpars != PRESET_UNCHANGED)
    dest->useSpars = preset->useSpars;
  if(preset->doLe != PRESET_UNCHANGED)
    dest->doLe = preset->doLe;
  if(preset->doLD != PRESET_UNCHANGED)
    dest->doLD = preset->doLD;
  if(preset->doLI != PRESET_UNCHANGED)
    dest->doLI = preset->doLI;

  if(strcmp(preset->leRegExp, STRING_UNCHANGED))
    strcpy(dest->leRegExp, preset->leRegExp);
  if(strcmp(preset->ldRegExp, STRING_UNCHANGED))
    strcpy(dest->ldRegExp, preset->ldRegExp);
  if(strcmp(preset->liRegExp, STRING_UNCHANGED))
    strcpy(dest->liRegExp, preset->liRegExp);

  if(preset->doWeighted != PRESET_UNCHANGED)
    dest->doWeighted = preset->doWeighted;
  if(preset->doWLe != PRESET_UNCHANGED)
    dest->doWLe = preset->doWLe;
  if(preset->doWLD != PRESET_UNCHANGED)
    dest->doWLD = preset->doWLD;
  if(strcmp(preset->wleRegExp, STRING_UNCHANGED))
    strcpy(dest->wleRegExp, preset->wleRegExp);
  if(strcmp(preset->wldRegExp, STRING_UNCHANGED))
    strcpy(dest->wldRegExp, preset->wldRegExp);
}

static void SetPresetCallback(Widget w, XtPointer client_data,
		       XtPointer call_data)
{
  int presetNr = (int)client_data;
  

  if(presetNr >= 0)
  {
    CopyPresetConfig(&bidir.basecfg, &bidirPresets[presetNr]);
    
    XtUnmanageChild(bidirControlPanel);
    XtDestroyWidget(bidirControlPanel);
    CreateBidirPathControlPanel(bidirControlParent);
    XtManageChild(bidirControlPanel);  
  }
}



void CreateBidirPathControlPanel(void *parent_widget)
{
  Widget parent = (Widget)parent_widget;
  Widget bidirForm, frame, subform;
  Widget columnForm, temp, options;
  int i;
  char name[] = "bidirPresetxxxx";  

  bidirControlParent = parent;
  bidirControlPanel = CreateDialog(parent, "bidirControlPanel");

  

  

  columnForm = CreateRowColumn(bidirControlPanel, "bidirColumn");

  bidirForm = CreateRowColumn(columnForm, "bidirLeftForm");

  

  CreateLabel(bidirForm, "bidirTitle");

  frame = CreateFrame(bidirForm, "bidirPixelHandlingFrame", 
		      "bidirPixelHandlingTitle");

  subform = CreateRowColumn(frame, "bidirPixelHandlingForm");
  

  CreateFormEntry(subform,"bidirSamplesPerPixel", 
		  "bidirSamplesPerPixelTextf", 
		  FET_UNSIGNED, (XtPointer)&(bidir.basecfg.samplesPerPixel),
		  NULL,0);

  temp = CreateToggleButton(subform, "bidirProgressiveToggle",
			    bidir.basecfg.progressiveTracing, 
			    ToggleCallback,
			    (XtPointer)&bidir.basecfg.progressiveTracing);

  XtManageChild(subform);

  frame = CreateFrame(bidirForm, "bidirPathLengthFrame", "bidirPathLengthFrameTitle");

  subform = CreateRowColumn(frame, "bidirPLForm");

  CreateFormEntry(subform,"bidirMaximumEyePathDepth", 
		  "bidirMaximumEyePathDepthTextf", 
		  FET_UNSIGNED, (XtPointer)&(bidir.basecfg.maximumEyePathDepth),
		  NULL,0);
  CreateFormEntry(subform,"bidirMaximumLightPathDepth", 
		  "bidirMaximumLightPathDepthTextf", 
		  FET_UNSIGNED, (XtPointer)&(bidir.basecfg.maximumLightPathDepth),
		  NULL,0);
  CreateFormEntry(subform,"bidirMaximumPathDepth", 
		  "bidirMaximumPathDepthTextf", 
		  FET_UNSIGNED, (XtPointer)&(bidir.basecfg.maximumPathDepth),
		  NULL,0);
  CreateFormEntry(subform,"bidirMinimumPathDepth", 
		  "bidirMinimumPathDepthTextf", 
		  FET_UNSIGNED, (XtPointer)&(bidir.basecfg.minimumPathDepth),
		  NULL,0);

  XtManageChild(subform);

  temp = CreateToggleButton(bidirForm, "bidirImportantLightToggle",
			    bidir.basecfg.sampleImportantLights, 
			    ToggleCallback,
			    (XtPointer)&bidir.basecfg.sampleImportantLights);

  

  

  CreateSeparator(bidirForm, "bidirPresetSeparator");
  options = CreateOptionMenu(bidirForm, "bidirPresetsMenu", "bidirPresets");
  CreateOptionButton(options, "bidirPresetName", TRUE, 
		     SetPresetCallback, (XtPointer)-1);

  

  for(i = 0; i < NUM_BIDIR_PRESETS; i++)
  {
    sprintf(name, "bidirPreset%i", i);
    CreateOptionButton(options, name, FALSE, 
		       SetPresetCallback, (XtPointer)i);
  }


  XtManageChild(bidirForm);

  bidirForm = CreateRowColumn(columnForm, "bidirRightForm");

  temp = CreateToggleButton(bidirForm, "bidirUseSpars",
			    bidir.basecfg.useSpars, 
			    SparToggleCallback,
			    (XtPointer)&bidir.basecfg.useSpars);

  frame = CreateFrame(bidirForm, "bidirSparFrame", "bidirSparFrameTitle");

  subform = CreateRowColumn(frame, "bidirSparForm");

  temp = CreateToggleButton(subform, "bidirDoLe",
			    bidir.basecfg.doLe, 
			    ToggleCallback,
			    (XtPointer)&bidir.basecfg.doLe);
  CreateFormEntry(subform,"bidirLeRegExp", 
		  "bidirLeRegExpTextf", 
		  FET_STRING, (XtPointer)&(bidir.basecfg.leRegExp),
		  NULL, MAX_REGEXP_SIZE-1);

#ifdef WMP_WEIGHTS
  temp = CreateToggleButton(subform, "bidirDoLD",
			    bidir.basecfg.doLD, 
			    ToggleCallback,
			    (XtPointer)&bidir.basecfg.doLD);
  CreateFormEntry(subform,"bidirLDRegExp", 
		  "bidirLDRegExpTextf", 
		  FET_STRING, (XtPointer)&(bidir.basecfg.ldRegExp),
		  NULL, MAX_REGEXP_SIZE-1);

  temp = CreateToggleButton(subform, "bidirDoLI",
			    bidir.basecfg.doLI, 
			    ToggleCallback,
			    (XtPointer)&bidir.basecfg.doLI);
  CreateFormEntry(subform,"bidirLIRegExp", 
		  "bidirLIRegExpTextf", 
		  FET_STRING, (XtPointer)&(bidir.basecfg.liRegExp),
		  NULL, MAX_REGEXP_SIZE-1);
#else
  // Without weighted multipass define: just do full radiance readout
  temp = CreateToggleButton(subform, "bidirDoLDI",
			    bidir.basecfg.doLD, 
			    ToggleCallback,
			    (XtPointer)&bidir.basecfg.doLD);
  CreateFormEntry(subform,"bidirLDIRegExp", 
		  "bidirLDIRegExpTextf", 
		  FET_STRING, (XtPointer)&(bidir.basecfg.ldRegExp),
		  NULL, MAX_REGEXP_SIZE-1);
#endif 


  bidirSparForm = subform;
  if(!bidir.basecfg.useSpars)
    DisableWidget(bidirSparForm);

  XtManageChild(subform);

  XtManageChild(bidirForm);

  XtManageChild(columnForm);

 
  temp = XmMessageBoxGetChild(bidirControlPanel, XmDIALOG_CANCEL_BUTTON);
  XtUnmanageChild(temp);
}

