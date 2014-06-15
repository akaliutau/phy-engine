

#include "ui.h"
#include "ui_sdk.h"
#include "camera.h"
#include "defaults.h"
#include "render.h"

extern CAMERA AlternateCamera;


static void EditCameraOKCallback(Widget w, XtPointer client_data, XtPointer calldata)
{
  CAMERA *cam = (CAMERA *)client_data;

  CameraSet(&Camera, &cam->eyep, &cam->lookp, &cam->updir, 
	    cam->fov, Camera.hres, Camera.vres, &Camera.background);
  RenderScene();
}

static void EditCameraCancelCallback(Widget w, XtPointer client_data, XtPointer calldata)
{
  Widget editCameraDialog = XtParent(w);
  CAMERA *cam = (CAMERA *)client_data;

  XtDestroyWidget(editCameraDialog);
  CameraDestroy(cam);
}

static void ShowEditCameraBox(Widget w, XtPointer client_data, XtPointer call_data)
{
  Widget editCameraBox, editCameraForm, frame, subform;
  CAMERA *cam;

  editCameraBox = CreateDialog(w, "editCameraBox");
  editCameraForm = CreateRowColumn(editCameraBox, "editCameraForm");

  cam = CameraCreate();	
  *cam = Camera;

  frame = CreateFrame(editCameraForm, "eyepFrame", "eyepTitle");
  subform = CreateRowColumn(frame, "eyepForm");
  CreateFormEntry(subform, "xLabel", "xTextf", 
		  FET_FLOAT, (XtPointer)&cam->eyep.x, NULL, 0);
  CreateFormEntry(subform, "yLabel", "yTextf", 
		  FET_FLOAT, (XtPointer)&cam->eyep.y, NULL, 0);
  CreateFormEntry(subform, "zLabel", "zTextf", 
		  FET_FLOAT, (XtPointer)&cam->eyep.z, NULL, 0);
  XtManageChild(subform);

  frame = CreateFrame(editCameraForm, "lookpFrame", "lookpTitle");
  subform = CreateRowColumn(frame, "lookpForm");
  CreateFormEntry(subform, "xLabel", "xTextf", 
		  FET_FLOAT, (XtPointer)&cam->lookp.x, NULL, 0);
  CreateFormEntry(subform, "yLabel", "yTextf", 
		  FET_FLOAT, (XtPointer)&cam->lookp.y, NULL, 0);
  CreateFormEntry(subform, "zLabel", "zTextf", 
		  FET_FLOAT, (XtPointer)&cam->lookp.z, NULL, 0);
  XtManageChild(subform);

  frame = CreateFrame(editCameraForm, "updirFrame", "updirTitle");
  subform = CreateRowColumn(frame, "updirForm");
  CreateFormEntry(subform, "xLabel", "xTextf", 
		  FET_FLOAT, (XtPointer)&cam->updir.x, NULL, 0);
  CreateFormEntry(subform, "yLabel", "yTextf", 
		  FET_FLOAT, (XtPointer)&cam->updir.y, NULL, 0);
  CreateFormEntry(subform, "zLabel", "zTextf", 
		  FET_FLOAT, (XtPointer)&cam->updir.z, NULL, 0);
  XtManageChild(subform);

  frame = CreateFrame(editCameraForm, "fovFrame", "fovTitle");
  subform = CreateRowColumn(frame, "fovForm");
  CreateFormEntry(subform, NULL, "fovTextf", 
		  FET_FLOAT, (XtPointer)&cam->fov, NULL, 0);
  XtManageChild(subform);

  XtManageChild(editCameraForm);

  
  XtAddCallback(editCameraBox, XmNokCallback, EditCameraOKCallback, (XtPointer)cam);
  XtAddCallback(editCameraBox, XmNcancelCallback, EditCameraCancelCallback, (XtPointer)cam);

  
  XtManageChild(editCameraBox);
}

static void SaveCameraCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  CameraPush(&Camera);
  RenderScene();
}

static void RestoreCameraCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  CameraPop(&Camera);
  RenderScene();
}

static void SetAlternateCameraCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  AlternateCamera = Camera;
}

static void ToggleAlternateCameraCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  CAMERA tmp = Camera;
  
  AlternateCamera.hres = Camera.hres;
  AlternateCamera.vres = Camera.vres;
  Camera = AlternateCamera;
  Camera.changed = 0;
  AlternateCamera = tmp;
  renderopts.render_raytraced_image = FALSE;
  RenderScene();
}

static void ResetCameraCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  VECTOR eyep = DEFAULT_EYEP, 
         lookp = DEFAULT_LOOKP,
         updir = DEFAULT_UPDIR;
  RGB    backgroundcolor = DEFAULT_BACKGROUND_COLOR;

  CameraSet(&Camera, &eyep, &lookp, &updir, DEFAULT_FOV, 
	    Camera.hres, Camera.vres, &backgroundcolor);
  RenderScene();
}

static void PrintCameraCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  CameraPrint(stderr, &Camera);
  
}

void CreateCameraMenu(Widget menuBar)
{
  Widget cameraMenu;

  AlternateCamera = Camera;

  cameraMenu = CreateSubMenu(menuBar, "cameraButton", "cameraMenu");

  CreatePushButton(cameraMenu, "editCameraButton", ShowEditCameraBox, (XtPointer)NULL);
  CreatePushButton(cameraMenu, "resetCameraButton", ResetCameraCallback, (XtPointer)NULL);
  CreateSeparator(cameraMenu, "cameraSeparator");

  CreatePushButton(cameraMenu, "saveCameraButton", SaveCameraCallback, (XtPointer)NULL);
  CreatePushButton(cameraMenu, "restoreCameraButton", RestoreCameraCallback, (XtPointer)NULL);
  CreateSeparator(cameraMenu, "cameraSeparator");

  CreatePushButton(cameraMenu, "setAlternateCameraButton", SetAlternateCameraCallback, (XtPointer)NULL);
  CreatePushButton(cameraMenu, "toggleAlternateCameraButton", ToggleAlternateCameraCallback, (XtPointer)NULL);
  CreateSeparator(cameraMenu, "cameraSeparator");

  CreatePushButton(cameraMenu, "printCameraButton", PrintCameraCallback, (XtPointer)NULL);
}

