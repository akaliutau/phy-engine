

#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#include <Xm/Xm.h>
#include <Xm/RepType.h>

#include "config.h"

#include "ui.h"
#include "ui_sdk.h"
#include "defaults.h"
#include "render.h"
#include "canvas.h"
#include "batch.h"
#include "error.h"
#include "options.h"
#include "camera.h"
#include "ipc.h"




static CANVAS_TYPE canvastype = INTEGRATED_CANVAS;
int batch_processing = FALSE;
int batch_quit_at_end = FALSE;
static Window extern_canvas_window = 0;
static Dimension hres = 0, vres = 0;
static char *canvas_dpyname = NULL;
static Display *canvas_display;



XVisualInfo best_visual;
Colormap cmap;


Arg visargs[3];
int nrvisargs;


XtAppContext app_context;
Widget topLevel=0, canvas;
Display *display;


char *current_directory;


extern char *fallback_resources[];

static void ProcessWaitingExternalCanvasEvents(void)
{
  if (canvastype == EXTERNAL_CANVAS) {
    while (XPending(canvas_display)) {
      XEvent event;
      XNextEvent(canvas_display, &event);
      ExternalCanvasEvent(&event);
    }
  }
}


void ProcessWaitingEvents(void)
{
  if (Ipc.have_ipc)
    IpcCheckForMessages();

  if (!batch_processing || canvastype == INTEGRATED_CANVAS) {
    XtInputMask mask;
    while ((mask = XtAppPending(app_context))) 
      XtAppProcessEvent(app_context, mask);
  }

  ProcessWaitingExternalCanvasEvents();
}


void CheckForEvents(void)
{
  clock_t lastt = 0;

  if (batch_processing && canvastype == OFFSCREEN_CANVAS)
    return;	

  
  if (clock() - lastt < CLOCKS_PER_SEC)
    return;

  ProcessWaitingEvents();

  lastt = clock();
}


int Working(void)
{
  return CanvasGetMode() == CANVASMODE_WORKING;
}

void SetWindowTitle(char *title)
{
  if (topLevel)
    SetDialogTitle(topLevel, title);
}


static void BatchOption(void *value)
{
  batch_processing = TRUE;
}

static void BatchQuitOption(void *value)
{
  batch_quit_at_end = TRUE;
}

static void OffscreenOption(void *value)
{
  canvastype = OFFSCREEN_CANVAS;
}

static void CanvasIdOption(void *value)
{
  extern_canvas_window = *(int *)value;
  canvastype = EXTERNAL_CANVAS;
}

static void CloseCanvasDisplay(void)
{
  XCloseDisplay(canvas_display);
}

static void CanvasDisplayOption(void *value)
{
  canvas_dpyname = *(char **)value;
}

static void HResOption(void *value)
{
  hres = *(int *)value;
}

static void VResOption(void *value)
{
  vres = *(int *)value;
}

#ifdef EXTERNAL_CANVAS_TESTING

static void CreateTestCanvas(void *value)
{
  Display *dpy;
  Window win;

  dpy = XOpenDisplay(NULL);  
  if (RenderGetVisualInfoAndColormap(DefaultScreenOfDisplay(dpy),
				     &best_visual,
				     &cmap)) {
    XSetWindowAttributes swattr;
    swattr.background_pixmap = None;
    swattr.background_pixel = 0;
    swattr.border_pixmap = CopyFromParent;
    swattr.border_pixel = 0;
    swattr.backing_store = NotUseful;
    swattr.colormap = cmap;
    win = XCreateWindow(dpy, DefaultRootWindow(dpy), 0, 0, 640, 480, 0, 
		        best_visual.depth, InputOutput, best_visual.visual, 
		        CWBackPixel | CWBorderPixel | CWBackingStore | CWColormap, 
		        &swattr);
  } else {
    win = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0, 640 ,480, 0, 0, 0);
  }

  XSelectInput(dpy, win, KeyReleaseMask);
  XMapWindow(dpy, win);

  fprintf(stderr, "win = %ld\n", win);

  while (1) {
    XEvent event;
    XNextEvent(dpy, &event);
    switch (event.type) {
    case KeyRelease:
      exit(0);
    default:
      break;
    }
  }
}
#endif

static void ThreeButtonMotionOption(void *value)
{
  twobutton_motion = FALSE;
}

static void TwoButtonMotionOption(void *value)
{
  twobutton_motion = TRUE;
}

static CMDLINEOPTDESC interfaceOptions[] = {
  {"-batch",	3,	TYPELESS,	NULL,	BatchOption,
   "-batch         \t\t: non-interactive processing"},
  {"-batch-quit-at-end", 8,	TYPELESS,	NULL,	BatchQuitOption,
   "-batch-quit-at-end\t: (batch mode) quit program at end"},
  {"-offscreen", 3,	TYPELESS,	NULL,	OffscreenOption,
   "-offscreen     \t\t: render into offscreen window"},
  {"-canvas-window",	3,	Tint,		NULL, 	CanvasIdOption,
   "-canvas-window <window-id>\t: render into external window with given id"},
  {"-canvas-display", 9, Tstring,	NULL, 	CanvasDisplayOption,
   "-canvas-display <display-name>\t: display name of external canvas window"},
  {"-hres",	3,	Tint,		NULL,	HResOption,
   "-hres <integer>\t\t: horizontal resolution (private canvas only)"},
  {"-vres",	3,	Tint,		NULL,	VResOption,
   "-vres <integer>\t\t: vertical resolution (private canvas only)"},
#ifdef EXTERNAL_CANVAS_TESTING
  {"-create-test-canvas", 3, TYPELESS, 	NULL,	CreateTestCanvas,
   "-create-test-canvas\t: for external canvas testing"},
#endif
  {"-2button-motion", 3, TYPELESS, 	NULL,	TwoButtonMotionOption,
   "-2button-motion\t: camera manipulation using 2 mouse buttons"},
  {"-3button-motion", 3, TYPELESS, 	NULL,	ThreeButtonMotionOption,
   "-3button-motion\t: camera manipulation using 3 mouse buttons"},
  {NULL	, 	0,	TYPELESS, 	NULL, 	DEFAULT_ACTION,
   NULL}
};

void InterfaceDefaults(void)
{
  twobutton_motion = FALSE;
}

void ParseInterfaceOptions(int *argc, char **argv)
{
  ParseOptions(interfaceOptions, argc, argv);
  ParseBatchOptions(argc, argv);
}

void PrintInterfaceOptions(FILE *fp)
{
  fprintf(fp, "\nInterface options:\n");
  PrintOptions(fp, interfaceOptions);
  PrintBatchOptions(fp);
}



static void CreateMainMenu(Widget menuBar)
{
  CreateFileMenu(menuBar);
  CreateRadianceMenu(menuBar);
  CreateRenderMenu(menuBar);
  CreateRayTracingMenu(menuBar);
  CreateToneMappingMenu(menuBar);
  CreateCameraMenu(menuBar);
#ifdef GIDEBUG
  CreateDebugMenu(menuBar);
#endif
  CreateHelpMenu(menuBar);
}


static Boolean StartBatchProcessing(XtPointer client_data)
{
  while (!RenderInitialized())
    ProcessWaitingEvents();
  RenderScene();

  Batch();
  if (batch_quit_at_end)
    exit(0);

  switch (canvastype) {
  case OFFSCREEN_CANVAS:
    exit(0);

  case EXTERNAL_CANVAS:
    while (1) {
      XEvent event;
      if (XCheckMaskEvent(canvas_display, ~NoEventMask, &event))
	ExternalCanvasEvent(&event);

      if (Ipc.have_ipc)
	IpcCheckForMessages();

      usleep(100000);
    }

  default:
    
    break;
  }

  return True;
}

void StartUserInterface(int *argc, char **argv)
{
  Widget mainWindow, menuBar;
  XSetWindowAttributes wattrs;

  if (!batch_processing || canvastype == INTEGRATED_CANVAS) {
  
  topLevel = XtVaAppInitialize(&app_context,	
				 APP_CLASS_NAME,	
			       NULL, 0,		
			       argc, argv,	
			       fallback_resources, 
			       NULL);		

  XmRepTypeInstallTearOffModelConverter();

  display = XtDisplay(topLevel);
  } else
    display = (Display *)NULL;

  
  if (*argc > 1) {
    if (*argv[1] == '-')
      Error(NULL, "Unrecognized option '%s'", argv[1]);
    else
      ReadFile(argv[1]);
  }

  
  if (canvastype == INTEGRATED_CANVAS &&
      RenderGetVisualInfoAndColormap(XtScreen(topLevel), 
				     &best_visual,
				     &cmap)) {
    
    XtSetArg(visargs[0], XtNvisual, best_visual.visual);
    XtSetArg(visargs[1], XtNdepth, best_visual.depth);
    XtSetArg(visargs[2], XtNcolormap, cmap);
    nrvisargs = 3;
    XtSetValues(topLevel, visargs, nrvisargs);
  } else
    nrvisargs = 0;

  if (!batch_processing) {
    
    LoadUserOptions();

    
    mainWindow = CreateForm(topLevel, "mainWindow");

    
    menuBar = CreateMenuBar(mainWindow, "menuBar");
  
    
    CreateMainMenu(menuBar);
    XtManageChild(menuBar);
  } else
    mainWindow = topLevel;

  
  switch (canvastype) {
  case INTEGRATED_CANVAS:
    canvas = CreateIntegratedCanvasWindow(mainWindow, hres, vres);
    if (twobutton_motion && !batch_processing) {
      
      CreateCanvasPopupMenu();
    }
    break;

  case EXTERNAL_CANVAS:
    canvas_display = XOpenDisplay(canvas_dpyname);
    if (!canvas_display)
      Fatal(1, NULL, "Couldn't open connection to display '%s'", 
	    canvas_dpyname ? canvas_dpyname : "(null)");
    else
      atexit(CloseCanvasDisplay);		

    ConnectExternalCanvasWindow(canvas_display, extern_canvas_window);
    break;

  case OFFSCREEN_CANVAS:
    if (hres <= 0) hres = 640;	
    if (vres <= 0) vres = 480;	
    CreateOffscreenCanvasWindow(hres, vres);
    break;

  default:
    Fatal(-1, "StartUserInterface", "Invalid canvas type %d", canvastype);
  }

  if (!batch_processing) {
    XtManageChild(mainWindow);
  }

  if (!batch_processing || canvastype == INTEGRATED_CANVAS) {
    
    XtRealizeWidget(topLevel);
  }

  if (canvastype == INTEGRATED_CANVAS) {
    
    wattrs.backing_store = NotUseful;
    XChangeWindowAttributes(XtDisplay(canvas), XtWindow(canvas), CWBackingStore, &wattrs);
#ifdef EXTERNAL_CANVAS_TESTING
    fprintf(stderr, "Canvas Window ID: %ld\n", XtWindow(canvas));
#endif
  }

  
  CanvasInit();

  if (batch_processing) {
    if (canvastype == INTEGRATED_CANVAS)
      XtAppAddWorkProc(app_context, StartBatchProcessing, NULL);
    else {
      StartBatchProcessing((XtPointer)NULL);
      exit(0);
    }
  }

  
  if (canvastype == EXTERNAL_CANVAS) {
    
    while (1) {
      ProcessWaitingEvents();
      usleep(50000);
    }
  } else
    XtAppMainLoop(app_context);
}

