

#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "ui.h"
#include "ui_sdk.h"
#include "defaults.h"
#include "canvas.h"
#include "render.h"
#include "statistics.h"
#include "pools.h"
#include "error.h"
#include "readmgf.h"
#include "vertexutils.h"
#include "appdata.h"
#include "camera.h"
#include "scene.h"
#include "radiance.h"
#include "fileopts.h"

int force_onesided_surfaces, monochrome, nqcdivs, disable_textures;



static void PushRecentFile(char *filename);
static void UpdateCurrentFileCamera(void);



static Widget CreateNrQuartCircDivDialog(Widget parent, char *name)
{
  return CreatePromptDialog(parent, name, FET_INTEGER, (XtPointer)&nqcdivs, NULL, 0);
}

static Widget CreateFileOptionsMenu(Widget parent)
{
  Widget menu = CreateSubMenu(parent, "fileOptionsButton", "fileOptionsMenu");

  CreateToggleButton(menu, "forceOneSidednessButton",
		     force_onesided_surfaces,
		     NULL, (XtPointer)&force_onesided_surfaces);

  CreateToggleButton(menu, "disableTexturesButton",
		     disable_textures,
		     NULL, (XtPointer)&disable_textures);

  CreateToggleButton(menu, "monochromeButton",
		     monochrome,
		     NULL, (XtPointer)&monochrome);

  CreateCascadeDialog(menu, "nqcdivsButton", 
		      CreateNrQuartCircDivDialog, 
		      "nqcdivsDialog",
		      NULL, NULL);

  return menu;
}



static int LoadFile(char *filename, FILE *fp, int ispipe, Widget loadBox)
{
  int succes;

  if (CanvasGetMode() == CANVASMODE_WORKING) {
    Warning(NULL, "Interrupt computations first before loading a new scene");
    return 0;	
  }

  if (!fp) {
    Error(NULL, "Please enter a file name to be loaded");
    return 0;
  }

  UpdateCurrentFileCamera();

  CanvasPushMode(CANVASMODE_WORKING);
  succes = ReadFile(filename);
  CanvasPullMode();

  if (succes)
    PushRecentFile(filename);

  RenderNewDisplayList();
  CanvasPostRedraw();

  return succes;	
}

static int LoadBgFile(char *filename, FILE *fp, int ispipe, Widget loadBox)
{
  int succes;

  if (!fp) {
    Error(NULL, "Please enter a file name to be loaded");
    return 0;
  }

  UpdateCurrentFileCamera();

  CanvasPushMode(CANVASMODE_WORKING);
  succes = InitBackground(fp);
  CanvasPullMode();

  RenderNewDisplayList();
  CanvasPostRedraw();


  return succes;	
}



static Widget CreateLoadBox(Widget parent, char *name)
{
  return CreateFileSelectionDialog(parent, name, LoadFile, "r");
}


static Widget CreateLoadBgBox(Widget parent, char *name)
{
  return CreateFileSelectionDialog(parent, name, LoadBgFile, "r");
}


static void FixVertexOrderInViewCallback(Widget w, XtPointer client_data, XtPointer call_data)
{
  FixVertexOrderInView();
}

static void CreateFixMenu(Widget fileMenu)
{
  Widget fixMenu;

  fixMenu = CreateSubMenu(fileMenu, "fixButton", "fixMenu");
  CreatePushButton(fixMenu, "fixVertexOrderInViewButton", FixVertexOrderInViewCallback, (XtPointer)NULL);
}


static int SaveImage(char *fname, FILE *fp, int ispipe, Widget saveBox)
{
  if (!fp)
    return 1;	

  
  XtUnmanageChild(saveBox);
  CheckForEvents();
  CanvasRedraw();

  CanvasPushMode(CANVASMODE_WORKING);
  SaveScreen(fname, fp, ispipe);
  CanvasPullMode();

  return 0;	
}

static Widget CreateSaveImageBox(Widget parent, char *name)
{
  return CreateFileSelectionDialog(parent, name, SaveImage, "w");
}

static int SaveModel(char *fname, FILE *fp, int ispipe, Widget saveBox)
{
  clock_t t;

  if (!fp)
    return 1;	

  if (!World) {
    Warning(NULL, "There is no model loaded");
    return 1;
  }

  CanvasPushMode(CANVASMODE_WORKING);
  CanvasPullMode();

  return 1;	
}

static Widget CreateSaveModelBox(Widget parent, char *name)
{
  return CreateFileSelectionDialog(parent, name, SaveModel, "w");
}



static char *fileStatsFormatString = (char *)NULL;


static Widget fileStatsMessage = (Widget)NULL;


void UpdateFileStats(void)
{
  char buf[MAX_LABEL_STRING_LENGTH];

  if (!fileStatsFormatString || !fileStatsMessage)
    return;

  sprintf(buf, fileStatsFormatString,
	  nrgeoms, nrcompounds, nrsurfaces, nrvertices, nrpatches,
	  total_area, 
	  ColorLuminance(total_emitted_power), 
	  ColorLuminance(max_selfemitted_power),
	  M_PI*ColorLuminance(max_selfemitted_radiance),
	  M_PI*ColorLuminance(estimated_average_radiance),
	  ColorGray(average_reflectivity),
	  (int)GetMemoryUsage()/1024,
	  (int)GetMemoryOverhead()/1024);

  SetLabelString(fileStatsMessage, buf);
}


static void UpdateFileStatsCallback(Widget statsBox, XtPointer client_data, XtPointer call_data)
{
  UpdateFileStats();
}


static void DismissStatsCallback(Widget statsBox, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild(statsBox);
}


static Widget CreateFileStatsBox(Widget parent, char *name)
{
  Widget statsBox;

  
  statsBox = CreateDialog(parent, name);

  
  fileStatsMessage = CreateLabel(statsBox, "fileStatsMessage");

  
  fileStatsFormatString = GetLabelString(fileStatsMessage);

  
  UpdateFileStats();

  XtAddCallback(statsBox, XmNokCallback, UpdateFileStatsCallback, (XtPointer)NULL);
  XtAddCallback(statsBox, XmNcancelCallback, DismissStatsCallback, (XtPointer)NULL);

  return statsBox;
}


static void ShowFileStats(Widget statsButton, XtPointer client_data, XtPointer call_data)
{
  Widget statsBox = (Widget)client_data;

  UpdateFileStats();
  XtManageChild(statsBox);
}



static void Exit(Widget w, XtPointer client_data, XtPointer call_data)
{
  UpdateCurrentFileCamera();
  SaveUserOptions();
  exit(0);
}




char *GetFilePart(char *name)
{
  char *filepart;

  if(name == NULL)
    return NULL;

  filepart = rindex(name, '/');

  if(filepart == NULL)
  {
    return(name);
  }

  return(filepart + 1); 
}




void SetRecentFileLabels(void)
{
  char *name;
  int i;

  for(i=0; i < NUM_RECENT_FILES; i++)
  {
    if(appData.recentFile[i].spec == NULL)
    {
      name = NULL;
    }
    else
    {
      name = GetFilePart(appData.recentFile[i].spec);
    }

    if(name == NULL || name[0] == '-')
    {
      XtUnmanageChild(appData.recentFile[i].widget);
    }
    else
    {
      XtVaSetValues(appData.recentFile[i].widget,
		    XtVaTypedArg, XmNlabelString, XmRString,
		    name, strlen(name)+1, NULL);
      XtManageChild(appData.recentFile[i].widget);
    }
  }
}


static int IsFilePushed = 0;

static void PushRecentFile(char *filename)
{
  int i;

  IsFilePushed = 1;

  

  i = 0;
  while((i < NUM_RECENT_FILES - 1) && ((appData.recentFile[i].spec == NULL) ||
				      (strcmp(filename, 
					      appData.recentFile[i].spec))))
  {
    i++;
  }

  

  

  

  while(i > 0)
  {
    
    appData.recentFile[i].spec = appData.recentFile[i-1].spec;
    appData.recentFile[i].cam = appData.recentFile[i-1].cam;
    appData.recentFile[i].acam = appData.recentFile[i-1].acam;
    i--;
  }

  appData.recentFile[0].spec = strdup(filename);

  SetRecentFileLabels();
}

static void LoadRecentFile(Widget w, XtPointer client_data, 
			   XtPointer call_data)
{
  int fileNr = (int)client_data;
  char *filename = appData.recentFile[fileNr].spec;
  CAMERA    oldCam, oldACam;
  CAMERA  * newCam   = &(appData.recentFile[fileNr].cam);
  CAMERA  * newACam  = &(appData.recentFile[fileNr].acam);
  int camPushed = FALSE;

  fprintf(stderr, "LoadRecentFile '%s'\n", filename);

  UpdateCurrentFileCamera();

  CanvasPushMode(CANVASMODE_WORKING);
  
  
  oldCam = Camera;
  oldACam = AlternateCamera;

  if (ReadFile(filename)) {
    
 
    if(!CameraCompare(&oldCam, &Camera))
    {
      

      CameraPush(&Camera);
      camPushed = TRUE;
    }

    if(!(CameraSet(&Camera, &(newCam->eyep), &(newCam->lookp), 
		   &(newCam->updir), newCam->fov, Camera.hres,
		   Camera.vres, &(Camera.background))))
    {
      if(camPushed)
      {
	CameraPop(&Camera);
      }
      else
      {
	Camera = oldCam;
      }
    }

    
    if(!(CameraSet(&AlternateCamera, &(newACam->eyep), &(newACam->lookp), 
		   &(newACam->updir), newACam->fov, Camera.hres,
		   Camera.vres, &(AlternateCamera.background))))
    {
      Warning("LoadRecentFile","cannot set alternate camera"); 
      AlternateCamera = oldACam;
    }

    RenderNewDisplayList();  
    
    oldCam = Camera;

    RenderScene();
    PushRecentFile(filename);
  }
  CanvasPullMode();
}



static void UpdateCurrentFileCamera(void)
{
  

  if(IsFilePushed)
  {
    appData.recentFile[0].cam = Camera;
    appData.recentFile[0].acam = AlternateCamera;
  }
}





void CreateFileMenu(Widget menuBar)
{
  int i;
  Widget fileMenu = CreateSubMenu(menuBar, "fileButton", "fileMenu");

  CreateFileOptionsMenu(fileMenu);
  CreateCascadeDialog(fileMenu, "loadButton", CreateLoadBox, "loadBox", NULL, NULL);

  CreateSeparator(fileMenu, "fileSeparator");
  CreateCascadeDialog(fileMenu, "saveImageButton", CreateSaveImageBox, "saveImageBox", NULL, NULL);
  CreateCascadeDialog(fileMenu, "saveModelButton", CreateSaveModelBox, "saveModelBox", NULL, NULL);

  CreateSeparator(fileMenu, "fileSeparator");
  CreateCascadeDialog(fileMenu, "loadBgButton", CreateLoadBgBox, "loadBgBox", NULL, NULL);

  CreateSeparator(fileMenu, "fileSeparator");
  CreateCascadeDialog(fileMenu, "statsButton", CreateFileStatsBox, "statsBox", ShowFileStats, NULL);

  
  CreateSeparator(fileMenu, "fileSeparator");
  CreateSeparator(fileMenu, "fileSeparator");

  

  for(i = 0; i < NUM_RECENT_FILES; i++)
  {
    char tmpStr[20];

    sprintf(tmpStr, "lastFile%iButton", i);

    appData.recentFile[i].widget = CreatePushButton(fileMenu, tmpStr, 
						  LoadRecentFile, 
						  (XtPointer)i);
  }

  SetRecentFileLabels();

  
  CreateSeparator(fileMenu, "fileSeparator");
  CreateSeparator(fileMenu, "fileSeparator");
  CreatePushButton(fileMenu, "exitButton", Exit, (XtPointer)NULL);
}

