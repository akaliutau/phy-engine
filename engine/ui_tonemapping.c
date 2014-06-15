

#include "ui.h"
#include "ui_sdk.h"
#include "tonemapping.h"
#include "cie.h"
#include "radiance.h"
#include "raytracing.h"
#include "render.h"
#include "scene.h"

static void RecomputeDisplayColors(void)
{
  if (Radiance && Radiance->RecomputeDisplayColors)
    Radiance->RecomputeDisplayColors();
  if (RayTracing && RayTracing->RecomputeDisplayColors)
    RayTracing->RecomputeDisplayColors();
  RenderNewDisplayList();
  RenderScene();
}

static void BrightnessOkCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  tmopts.pow_bright_adjust = pow(2., tmopts.brightness_adjust);
  SetToneMap(tmopts.ToneMap);
  RecomputeDisplayColors();
  XtDestroyWidget(w);
}

static void BrightnessCancelCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  XtDestroyWidget(w);
}

static void ReAdaptCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  if (World) {
    tmopts.pow_bright_adjust = pow(2., tmopts.brightness_adjust);
    EstimateViewAdaptation();
    SetToneMap(tmopts.ToneMap);
    RecomputeDisplayColors();
  }

  XtDestroyWidget(XtParent(w));
}

static void
AdaptationMethodCallback(Widget w,
			 XtPointer client_data, XtPointer call_data)
{
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set;
  if (set)
  {
    tmopts.statadapt = (TMA_METHOD)client_data;
    ReEstimateAdaptation();
    SetToneMap(tmopts.ToneMap);
    RecomputeDisplayColors();
  }
}

static Widget CreateAdaptationMethodBox(Widget parent)
{
  Widget tmaMenu = CreateRowColumn(parent, "tmaMethodForm");
  XtVaSetValues(tmaMenu,
		XmNradioBehavior, True,
		XmNorientation, XmHORIZONTAL,
		NULL);

  CreateToggleButton(tmaMenu, "tmaAverageButton",
		     tmopts.statadapt == TMA_AVERAGE,
		     AdaptationMethodCallback, (XtPointer)TMA_AVERAGE);

  CreateToggleButton(tmaMenu, "tmaMedianButton",
		     tmopts.statadapt == TMA_MEDIAN,
		     AdaptationMethodCallback, (XtPointer)TMA_MEDIAN);

  XtManageChild(tmaMenu);
  return tmaMenu;
}

static Widget CreateBrightnessDialog(Widget parent, char *name)
{
  Widget
    box = CreateDialog(parent, name),
    topform = CreateRowColumn(box, "brightnessTopForm"),
    form1, frame;

  frame = CreateFrame(topform, "brightnessFrame", "brightnessTitle");
  form1 = CreateRowColumn(frame, "brightnessForm");
  CreateFormEntry(form1, "brightnessLabel", "brightnessTextf",
		  FET_FLOAT, (XtPointer)&tmopts.brightness_adjust, NULL, 0);
  XtManageChild(form1);

  frame = CreateFrame(topform, "adaptationFrame", "adaptationTitle");
  form1 = CreateRowColumn(frame, "adaptationForm");

  CreateAdaptationMethodBox(form1);
  
  CreateFormEntry(form1, "tmLwaLabel", "tmLwaTextf",
		  FET_FLOAT, (XtPointer)&tmopts.lwa,
		  NULL, 0);

  XtManageChild(form1);
  XtManageChild(topform);

  XtAddCallback(box, XmNokCallback, BrightnessOkCallback, (XtPointer)NULL);
  XtAddCallback(box, XmNcancelCallback, BrightnessCancelCallback, (XtPointer)NULL);

  CreatePushButton(box, "brightnessAdaptButton", ReAdaptCallback, (XtPointer)NULL);

  return box;
}

static void ShowBrightnessDialog(Widget w,
				 XtPointer client_data, XtPointer call_data)
{
  Widget thebox = CreateBrightnessDialog(w, "brightnessDialog");
  XtManageChild(thebox);
}

static Widget CreateCIExyForm(Widget parent, char *name, char *titlename, float *x, float *y)
{
  Widget
    frame = CreateFrame(parent, name, titlename),
    form = CreateRowColumn(frame, "CIExyForm");

  CreateFormEntry(form, "xLabel", "xTextf",
		  FET_FLOAT, (XtPointer)x, NULL, 0);
  CreateFormEntry(form, "yLabel", "yTextf",
		  FET_FLOAT, (XtPointer)y, NULL, 0);

  XtManageChild(form);

  return frame;
}

static void MonitorCalibrationOkCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  ComputeColorConversionTransforms(tmopts.xr, tmopts.yr,
				   tmopts.xg, tmopts.yg,
				   tmopts.xb, tmopts.yb,
				   tmopts.xw, tmopts.yw);
  SetToneMap(tmopts.ToneMap);
  RecomputeGammaTables(tmopts.gamma);
  RecomputeDisplayColors();
}

static void MonitorCalibrationDismissCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild(w);
  RecomputeDisplayColors();
}

Widget CreateRGBForm(Widget parent, char *name, char *text, RGB *rgb)
{
  Widget form = CreateRowColumn(parent, name);
  if (text) CreateLabel(form, text);
  CreateFormEntry(form, "redLabel", "redTextf", FET_FLOAT, (XtPointer)&(rgb->r), NULL, 0);
  CreateFormEntry(form, "greenLabel", "greenTextf", FET_FLOAT, (XtPointer)&(rgb->g), NULL, 0);
  CreateFormEntry(form, "blueLabel", "blueTextf", FET_FLOAT, (XtPointer)&(rgb->b), NULL, 0);
  XtManageChild(form);
  return form;
}

static void GammaTestCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  int testimg = (int)client_data;
  int set = (((XmToggleButtonCallbackStruct *)call_data)->set == XmSET);
  if (!set) return;

  tmopts.display_test_image = (testimg!=0);
  tmopts.testimg = testimg;

  RecomputeGammaTables(tmopts.gamma);
  if (!tmopts.display_test_image)
    RecomputeDisplayColors();
  else
    RenderScene();
}

static Widget CreateGammaBox(Widget parent)
{
  Widget form = CreateRowColumn(parent, "gammaBox");
  Widget rbox;

  Widget frame = CreateFrame(form, "gammaFactorsFrame", "gammaFactorsLabel");
  CreateRGBForm(frame, "gammaForm", NULL, &tmopts.gamma);

  rbox = CreateRadioBox(form, "testImgBox");
  CreateToggleButton(rbox, "noTestImgButton", !tmopts.display_test_image, GammaTestCallback, (XtPointer)0);
  CreateToggleButton(rbox, "blackLevelTestImgButton", tmopts.display_test_image && tmopts.testimg == GAMMA_TEST_IMAGE, GammaTestCallback, (XtPointer)BLACK_LEVEL_IMAGE);
  CreateToggleButton(rbox, "gammaTestImgButton", tmopts.display_test_image && tmopts.testimg == BLACK_LEVEL_IMAGE, GammaTestCallback, (XtPointer)GAMMA_TEST_IMAGE);
  XtManageChild(rbox);

  XtManageChild(form);
  return form;
}

Widget CreateMonitorCalibrationDialog(Widget parent, char *name)
{
  Widget
    box = CreateDialog(parent, name),
    topform = CreateRowColumn(box, "monitorCalibrationForm"),
    form1, frame;

  frame = CreateFrame(topform, "tmParamFrame", "tmParamTitle");
  form1 = CreateRowColumn(frame, "tmParamsForm");
  CreateFormEntry(form1, "tmLdmaxLabel", "tmLdmaxTextf", 
		  FET_FLOAT, (XtPointer)&tmopts.ldm,
		  NULL, 0);
  CreateFormEntry(form1, "tmCmaxLabel", "tmCmaxTextf", 
		  FET_FLOAT, (XtPointer)&tmopts.cmax,
		  NULL, 0);
  XtManageChild(form1);

  frame = CreateFrame(topform, "monitorPrimariesFrame", "monitorPrimariesTitle");
  form1 = CreateRowColumn(frame, "monitorPrimariesForm");
  CreateCIExyForm(form1, "redPrimaryFrame", "redPrimaryTitle",
		  &tmopts.xr, &tmopts.yr);
  CreateCIExyForm(form1, "greenPrimaryFrame", "greenPrimaryTitle",
		  &tmopts.xg, &tmopts.yg);
  CreateCIExyForm(form1, "bluePrimaryFrame", "bluePrimaryTitle",
		  &tmopts.xb, &tmopts.yb);
  CreateCIExyForm(form1, "whitePrimaryFrame", "whitePrimaryTitle",
		  &tmopts.xw, &tmopts.yw);
  XtManageChild(form1);

  frame = CreateFrame(topform, "gammaBoxFrame", "gammaBoxTitle");
  CreateGammaBox(frame);

  XtManageChild(topform);

  XtAddCallback(box, XmNokCallback, MonitorCalibrationOkCallback, (XtPointer)NULL);
  XtAddCallback(box, XmNcancelCallback, MonitorCalibrationDismissCallback, (XtPointer)NULL);

  return box;
}

static void ToneMappingMethodCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set;
  if (set) {
    SetToneMap((TONEMAP *)client_data);
    RecomputeDisplayColors();
  }
}

void CreateToneMappingMethodMenu(Widget parent)
{
  Widget tmoMenu = CreateSubMenu(parent, "tmoMethodButton", "tmoMethodMenu");

  XtVaSetValues(tmoMenu, XmNradioBehavior, True, NULL);

  ForAllAvailableToneMaps(mapping) {
    CreateToggleButton(tmoMenu, mapping->buttonName,
		       mapping == tmopts.ToneMap ? True : False,
		       ToneMappingMethodCallback, (XtPointer)mapping);
  } EndForAll;
}

#ifdef NEVER
static void
CreateAdaptationMethodMenu(Widget parent)
{
  Widget tmaMenu = CreateSubMenu(parent, "tmaMethodButton", "tmaMethodMenu");

  XtVaSetValues(tmaMenu, XmNradioBehavior, True, NULL);

  CreateToggleButton(tmaMenu, "tmaNoneButton",
		     tmopts.statadapt == TMA_NONE,
		     AdaptationMethodCallback, (XtPointer)TMA_NONE);

  CreateToggleButton(tmaMenu, "tmaAverageButton",
		     tmopts.statadapt == TMA_AVERAGE,
		     AdaptationMethodCallback, (XtPointer)TMA_AVERAGE);

  CreateToggleButton(tmaMenu, "tmaMedianButton",
		     tmopts.statadapt == TMA_MEDIAN,
		     AdaptationMethodCallback, (XtPointer)TMA_MEDIAN);
  
}
#endif

void CreateToneMappingMenu(Widget menuBar)
{
  Widget tmMenu = CreateSubMenu(menuBar, "tmoButton", "tmoMenu");

  CreateToneMappingMethodMenu(tmMenu);
  

  CreateCascadeDialog(tmMenu, "monitorCalibrationButton", CreateMonitorCalibrationDialog, "monitorCalibrationDialog", NULL, NULL);

  
  CreatePushButton(tmMenu, "brightnessButton", ShowBrightnessDialog, NULL);
}
