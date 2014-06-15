

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "ui.h"
#include "ui_sdk.h"
#include "raytracing.h"
#include "canvas.h"
#include "camera.h"
#include "error.h"
#include "statistics.h"
#include "render.h"



static int busy_raytracing = FALSE;




static int DoRayTracing(void )
{
  
  CanvasPushMode(CANVASMODE_RENDER);
  
  

  Camera.changed = FALSE;
  renderopts.render_raytraced_image = TRUE;

  busy_raytracing = TRUE;
  RayTrace(NULL, NULL, FALSE);
  busy_raytracing = FALSE;

  CanvasPullMode();

  return 0;	
}




static void RayTracingRunStopCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
#ifdef ASK_FOR_FILENAME
  Widget filenamePromptBox = (Widget)client_data;
#endif

  if(RayTracing == NULL)
  {
    Error(0, "Specify raytracing method first");
  }
  else
  {
    if (!busy_raytracing)
    {
#ifdef ASK_FOR_FILENAME
      XtManageChild(filenamePromptBox);
#endif
      DoRayTracing();
    } 
    else 
    {
      RayTracing->InterruptRayTracing();
    }
  }
}


static void RayTracingControlCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  if (RayTracing)
    RayTracing->ShowControlPanel();
  else
    Error(NULL, "Specify a raytracing method first");
}





static char *raytracingStatsFormatString = (char *)NULL;



static Widget raytracingStatsMessage = (Widget)NULL;



void UpdateRayTracingStats(void)
{
  char buf[MAX_LABEL_STRING_LENGTH];

  if (!raytracingStatsFormatString || !raytracingStatsMessage)
    return;

  sprintf(buf, raytracingStatsFormatString,
	  rt_total_time/(float)CLOCKS_PER_SEC, 
	  rt_pixcount/(rt_total_time ? (rt_total_time/(float)CLOCKS_PER_SEC) : 1.),
	  rt_raycount/(rt_total_time ? (rt_total_time/(float)CLOCKS_PER_SEC) : 1.));

  SetLabelString(raytracingStatsMessage, buf);
}





static void UpdateRayTracingStatsCallback(Widget statsBox, XtPointer client_data, XtPointer call_data)
{
  UpdateRayTracingStats();
}





static void DismissStatsCallback(Widget statsBox, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild(statsBox);
}





static Widget CreateRayTracingStatsBox(Widget parent, char *name)
{
  Widget statsBox = CreateDialog(parent, name);

  
  raytracingStatsMessage = CreateLabel(statsBox, "raytracingStatsMessage");

  
  raytracingStatsFormatString = GetLabelString(raytracingStatsMessage);

  
  UpdateRayTracingStats();

  XtAddCallback(statsBox, XmNokCallback, UpdateRayTracingStatsCallback, (XtPointer)NULL);
  XtAddCallback(statsBox, XmNcancelCallback, DismissStatsCallback, (XtPointer)NULL);

  return statsBox;
}





static void ShowRayTracingStats(Widget statsButton, XtPointer client_data, XtPointer call_data)
{
  Widget statsBox = (Widget)client_data;

  UpdateRayTracingStats();
  XtManageChild(statsBox);
}




static void SetRayTracingMethodCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set;
  if (set == XmSET) {
    CanvasPushMode(CANVASMODE_WORKING);
    
    SetRayTracing((RAYTRACINGMETHOD *)client_data);
    CanvasPullMode();
  }
}

static void CreateRayTracingMethodMenu(Widget parent)
{
  Widget methodMenu = CreateSubMenu(parent, "raytracingMethodButton", "raytracingMethodMenu");

  XtVaSetValues(methodMenu,
		XmNradioBehavior, True,
		NULL);

  CreateToggleButton(methodMenu, "NoRayTracingButton",
		     (!RayTracing ? TRUE : FALSE),
		     SetRayTracingMethodCallback, (XtPointer)NULL);

  ForAllRayTracingMethods(method) {
    CreateToggleButton(methodMenu, method->buttonName, 
		       (method == RayTracing ? TRUE : FALSE),
		       SetRayTracingMethodCallback, (XtPointer)method);
  } EndForAll;
}



static void CreateRayTracingControlPanels(void *parent_widget)
{
  ForAllRayTracingMethods(method) {
    method->CreateControlPanel(parent_widget);
  } EndForAll;
}



static void RayTracingRedisplayCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    
  CanvasPushMode(CANVASMODE_RENDER);
  

  if (!RayTracing)
    Warning(NULL, "No ray tracing method active");
  else if (!RayTracing->Redisplay || !RayTracing->Redisplay())
    Warning(NULL, "No previous %s image available", RayTracing->fullName);
  else {	
    Camera.changed = FALSE;
    renderopts.render_raytraced_image = TRUE;
  }

  CanvasPullMode();
}

static void RayTracingReprojectCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
    
  CanvasPushMode(CANVASMODE_RENDER);
  

  if (!RayTracing)
    Warning(NULL, "No ray tracing method active");
  else if (!RayTracing->Reproject || !RayTracing->Reproject())
    Warning(NULL, "No previous %s image available", RayTracing->fullName);
  else {	
    Camera.changed = FALSE;
    renderopts.render_raytraced_image = TRUE;
  }

  CanvasPullMode();
}

static int RayTracingSaveImage(char *fname, FILE *fp, int ispipe, Widget fsbox)
{
  ImageOutputHandle *img = NULL;

  if (fp) {
    img = CreateRadianceImageOutputHandle(fname, fp, ispipe, 
					  Camera.hres, Camera.vres, reference_luminance/179.);
    if (!img) return 0;
  }

  if (!RayTracing)
    Warning(NULL, "No ray tracing method active");
  else if (!RayTracing->SaveImage || !RayTracing->SaveImage(img))
    Warning(NULL, "No previous %s image available", RayTracing->fullName);

//  if (img)
//    DeleteImageOutputHandle(img);

  return 1;	
}

static Widget CreateSaveImageDialog(Widget parent, char *name)
{
  return CreateFileSelectionDialog(parent, name, RayTracingSaveImage, "w");
}



void CreateRayTracingMenu(Widget menuBar)
{
  Widget raytracingMenu, raytracingControlButton;
#ifdef ASK_FOR_FILENAME
  Widget filenamePromptBox;
#endif

  raytracingMenu = CreateSubMenu(menuBar, "raytracingButton", "raytracingMenu");

  CreateRayTracingMethodMenu(raytracingMenu);

  raytracingControlButton = CreatePushButton(raytracingMenu, "raytracingControlButton",
					     RayTracingControlCallback, (XtPointer)NULL);
  CreateRayTracingControlPanels(raytracingControlButton);
  CreateSeparator(raytracingMenu, "raytracingSeparator");

#ifdef ASK_FOR_FILENAME
  
  filenamePromptBox = CreateFileSelectionDialog(raytracingMenu, "filenamePromptBox", 
						DoRayTracing, "w");
#endif

  CreatePushButton(raytracingMenu, "raytracingRunStopButton", 
		   RayTracingRunStopCallback, (XtPointer)NULL );

  busy_raytracing = FALSE;
  CreateSeparator(raytracingMenu, "raytracingSeparator");

  
  CreatePushButton(raytracingMenu, "raytracingRedisplayButton",
		   RayTracingRedisplayCallback, NULL);
#ifdef RT_REPROJECT_BUTTON
  CreatePushButton(raytracingMenu, "raytracingReprojectButton",
		   RayTracingReprojectCallback, NULL);
#endif
  
  CreateCascadeDialog(raytracingMenu, "raytracingSaveImageButton", 
		      CreateSaveImageDialog, "filenamePromptBox",
		      NULL, NULL);

  CreateSeparator(raytracingMenu, "raytracingSeparator");
  CreateCascadeDialog(raytracingMenu, "statsButton", CreateRayTracingStatsBox, "statsBox", 
		      ShowRayTracingStats, NULL);
}




