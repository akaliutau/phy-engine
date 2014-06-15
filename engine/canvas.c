

#include <Xm/Xm.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#include "canvas.h"
#include "ui.h"
#include "render.h"
#include "camera.h"
#include "select.h"
#include "error.h"

static CANVAS_TYPE canvastype;
static Window canvas_window;
static Display *canvas_display;
static int canvas_inited = FALSE;

static void CanvasResizeCallback(Widget canvas, XtPointer client_data, XtPointer call_data);
static void CanvasExposeCallback(Widget canvas, XtPointer client_data, XtPointer call_data);
static void CanvasMouseEvent(Widget canvas, XtPointer client_data, XEvent *event);
static void CanvasKeyEvent(Widget canvas, XtPointer client_data, XEvent *event);

int twobutton_motion;


Widget CreateIntegratedCanvasWindow(Widget parent, Dimension hres, Dimension vres)
{
  canvastype = INTEGRATED_CANVAS;

  canvas = RenderCreateWindow(parent);

  if (hres > 0)		
    XtVaSetValues(canvas, XmNwidth, hres, NULL);
  else
    XtVaGetValues(canvas, XmNwidth, &hres, NULL);
  
  if (vres > 0)		
    XtVaSetValues(canvas, XmNheight, vres, NULL);
  else
    XtVaGetValues(canvas, XmNheight, &vres, NULL);

  
  CameraSet(&Camera, &Camera.eyep, &Camera.lookp, &Camera.updir,
	    Camera.fov, hres, vres, &Camera.background);

  
  XtAddCallback(canvas, XmNexposeCallback, CanvasExposeCallback, (XtPointer)NULL);

  
  XtAddCallback(canvas, XmNresizeCallback, CanvasResizeCallback, (XtPointer)NULL);

  
  XtAddEventHandler(canvas, 
		    ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
		    False,	
		    (XtEventHandler)CanvasMouseEvent,
		    (XtPointer)NULL);	

  
  XtAddEventHandler(canvas,
		     KeyReleaseMask,
		    False,	
		    (XtEventHandler)CanvasKeyEvent,
		    (XtPointer)NULL);	

  return canvas;
}


void ConnectExternalCanvasWindow(Display *display, Window window)
{
  XWindowAttributes wattr;
  XVisualInfo *vinfo, vinfo_template;
  long vinfo_mask;
  int nreturn;

  canvas_window = window;
  canvas_display = display;
  canvastype = EXTERNAL_CANVAS;

  
  if (!XGetWindowAttributes(display, window, &wattr))
    Fatal(1, NULL, "Bad external canvas window");

  
  vinfo_template.visualid = XVisualIDFromVisual(wattr.visual);
  vinfo_mask = VisualIDMask;
  vinfo = XGetVisualInfo(display, vinfo_mask, &vinfo_template, &nreturn);
  if (nreturn < 1)
    Fatal(1, NULL, "Couldn't find X visual info for external canvas window");

  
  if (wattr.all_event_masks & ButtonPressMask)
    Warning(NULL, "ButtonPress events are already being handled on the canvas window.\nNavigation via the mouse will not be possible");

  
  XSelectInput(canvas_display, canvas_window,
	       ExposureMask | StructureNotifyMask |
	       ((wattr.all_event_masks & ButtonPressMask) ? 0 : ButtonPressMask) | ButtonReleaseMask | ButtonMotionMask |
	        KeyReleaseMask);

  
  RenderInitWindow(display, window, vinfo);

  
  CameraSet(&Camera, &Camera.eyep, &Camera.lookp, &Camera.updir,
	    Camera.fov, wattr.width, wattr.height, &Camera.background);

  
  RenderScene();
}


void CreateOffscreenCanvasWindow(int hres, int vres)
{
  canvastype = OFFSCREEN_CANVAS;

  RenderCreateOffscreenWindow(hres, vres);

  
  CameraSet(&Camera, &Camera.eyep, &Camera.lookp, &Camera.updir,
	    Camera.fov, hres, vres, &Camera.background);

  
  RenderScene();
}


#define CANVASMODESTACKSIZE 5
static int cursordefined = 0, modestackidx;
static CANVAS_MODE canvasmode = CANVASMODE_NORMAL;
static CANVAS_MODE modestack[CANVASMODESTACKSIZE];

static Cursor working_cursor, select_cursor, render_cursor;
#define WORKING_CURSOR	XC_watch
#define SELECT_CURSOR XC_crosshair
#define RENDER_CURSOR XC_spraycan


void CanvasInit(void)
{
  switch (canvastype) {
  case INTEGRATED_CANVAS:
    canvas_window = XtWindow(canvas);
    canvas_display = XtDisplay(canvas);
    break;

  case EXTERNAL_CANVAS:
    break;

  case OFFSCREEN_CANVAS:
    return;

  default:
    Fatal(-1, "CanvasInit", "Invalid canvas type 5d", canvastype);
  }

  canvasmode = CANVASMODE_NORMAL;
  modestackidx = 0;
  modestack[modestackidx] = canvasmode;
  cursordefined = FALSE;

  
  working_cursor = XCreateFontCursor(canvas_display, WORKING_CURSOR);	

  
  select_cursor = XCreateFontCursor(canvas_display, SELECT_CURSOR); 

  
  render_cursor = XCreateFontCursor(canvas_display, RENDER_CURSOR);

  canvas_inited = TRUE;
}


static void CanvasSetMode(CANVAS_MODE mode)
{
  if (!canvas_inited || canvastype == OFFSCREEN_CANVAS)
    return;

  switch (mode) {
  case CANVASMODE_NORMAL:
    if (cursordefined) {
      XUndefineCursor(canvas_display, canvas_window);
      cursordefined = FALSE;
    }

    canvasmode = CANVASMODE_NORMAL;
    break;
  case CANVASMODE_WORKING:
    
    XDefineCursor(canvas_display, canvas_window, working_cursor);
    XSync(canvas_display, False);
    cursordefined = TRUE;
    
    canvasmode = CANVASMODE_WORKING;
    break;
  case CANVASMODE_SELECT_PATCH:
    
    XDefineCursor(canvas_display, canvas_window, select_cursor);
    XSync(canvas_display, False);
    cursordefined = TRUE;
    
    canvasmode = CANVASMODE_SELECT_PATCH;
    break;
  case CANVASMODE_SELECT_PIXEL:
    
    XDefineCursor(canvas_display, canvas_window, select_cursor);
    XSync(canvas_display, False);
    cursordefined = TRUE;
    
    canvasmode = CANVASMODE_SELECT_PIXEL;
    break;
  case CANVASMODE_RENDER:
    
    XDefineCursor(canvas_display, canvas_window, render_cursor);
    XSync(canvas_display, False);
    cursordefined = TRUE;
    
    canvasmode = CANVASMODE_RENDER;
    break;
  default:
    Fatal(4, "CanvasSetMode", "Invalid mode %d - internal error.", mode);
    break;
  }

  modestack[modestackidx] = canvasmode;
}


CANVAS_MODE CanvasGetMode(void)
{
  return canvasmode;
}


void CanvasPushMode(CANVAS_MODE mode)
{
  modestackidx++;
  if (modestackidx >= CANVASMODESTACKSIZE) 
    Fatal(4, "CanvasPushMode", "Mode stack size (%d) exceeded.", CANVASMODESTACKSIZE);

  CanvasSetMode(mode);
}


void CanvasPullMode(void)
{
  modestackidx--;
  if (modestackidx < 0)
    Fatal(4, "CanvasPullMode", "Canvas mode stack underflow.\n");
  
  CanvasSetMode(modestack[modestackidx]);
}


static void CanvasResize(int width, int height)
{
  CameraSet(&Camera, &Camera.eyep, &Camera.lookp, &Camera.updir,
	    Camera.fov, width, height, &Camera.background);

  RenderScene();
}

static void CanvasResizeCallback(Widget canvas, XtPointer client_data, XtPointer call_data)
{
  Dimension width, height;

  XtVaGetValues(canvas,
		XmNwidth, &width,
		XmNheight, &height,
		NULL);

  CanvasResize(width, height);
}


static int RedrawWorkProcInstalled = FALSE;
static XtWorkProcId RedrawWorkProcId;

static Boolean RedrawWorkProc(XtPointer client_data)
{
  RenderScene();
  RedrawWorkProcInstalled = FALSE;
  return TRUE;
}


void CanvasPostRedraw(void)
{
  if (canvastype == INTEGRATED_CANVAS && !RedrawWorkProcInstalled) {
    RedrawWorkProcId = XtAppAddWorkProc(app_context, RedrawWorkProc, (XtPointer)NULL);
    RedrawWorkProcInstalled = TRUE;
  }
}


void CanvasCancelRedraw(void)
{
  if (canvastype == INTEGRATED_CANVAS && RedrawWorkProcInstalled) {
    XtRemoveWorkProc(RedrawWorkProcId);
    RedrawWorkProcInstalled = FALSE;
  }
}


void CanvasRedraw(void)
{
  RenderScene();
  CanvasCancelRedraw();
}

static void ExternalCanvasExpose(void)
{
  if (canvasmode == CANVASMODE_RENDER)
    return;	

  RenderScene();
}


static void CanvasExposeCallback(Widget canvas, XtPointer client_data, XtPointer call_data)
{
  if (canvasmode == CANVASMODE_RENDER)
    return;	

  CanvasPostRedraw();
}


static void DoMotion(int x, int y, int lastx, int lasty, int buttonspressed)
{
  Dimension maxx, maxy;
  float fov, aspect, a, w, view_dist;
  VECTOR d;

  maxx = Camera.hres;
  maxy = Camera.vres;
  fov = 2. * Camera.fov * M_PI / 180.;
  aspect = (float)maxx/(float)maxy;
  if (aspect > 1) fov *= aspect;
  VECTORSUBTRACT(Camera.lookp, Camera.eyep, d);
  view_dist = VECTORNORM(d);
  w = view_dist * fov;

  
  a = VECTORDOTPRODUCT(Camera.Z, Camera.updir) / VECTORNORM(Camera.updir);
  a = sin(acos(a < -1. ? -1. : (a > 1. ? 1. : a)));

  
  if (twobutton_motion) {
    switch (buttonspressed) {
    case 1:  	
      if (x != lastx) 
	CameraTurnRight(&Camera, (float)(x - lastx)/(float)maxx * fov * a);
      if (y != lasty) 
	CameraMoveHorizontal(&Camera, (float)(lasty - y)/(float)maxy * 2. * view_dist);
      break;

    case 2:   
      if (x != lastx) 
	CameraMoveRight(&Camera, (float)(x - lastx)/(float)maxx * w);
      if (y != lasty) 
	CameraMoveUp(&Camera, (float)(y - lasty)/(float)maxx * w / aspect);
      break;

    case 3:	
      if (x != lastx) 
	CameraTurnRight(&Camera, (float)(x - lastx)/(float)maxx * fov * a);
      if (y != lasty) 
	CameraTurnUp(&Camera, (float)(lasty - y)/(float)maxy * fov / aspect);
      break;

    default:
      
      return;
    }
  } else { 
    switch (buttonspressed) {
    case 1:  	
      if (x != lastx) 
	CameraTurnRight(&Camera, (float)(x - lastx)/(float)maxx * fov * a);
      if (y != lasty) 
	CameraTurnUp(&Camera, (float)(lasty - y)/(float)maxy * fov / aspect);
      break;
      
    case 2:   
      if (x != lastx) 
	CameraMoveRight(&Camera, (float)(x - lastx)/(float)maxx * w);
      if (y != lasty) 
	CameraMoveUp(&Camera, (float)(y - lasty)/(float)maxx * w / aspect);
      break;

    case 3:	
    case 4:	
      if (x != lastx) 
	CameraMoveRight(&Camera, (float)(x - lastx)/(float)maxx * w);
      if (y != lasty) 
	CameraMoveForward(&Camera, (float)(lasty - y)/(float)maxy * 2. * view_dist);
      break;

    default:
      
      return;
    }
  }

  
  RenderScene();
}

static PATCH *the_patch;
static POINT the_point;
static void GetThePatch(PATCH *patch, POINT *hitp)
{
  the_patch = patch;
  the_point = *hitp;
}

static void CanvasClick(XEvent *event)
{
  the_patch = (PATCH *)NULL;
  SelectPatchSetCallback(GetThePatch);
  CanvasPushMode(CANVASMODE_SELECT_PATCH);
  SelectPatch(event->xbutton.x, event->xbutton.y);

  if (the_patch)
    PopUpCanvasMenu(event, the_patch, &the_point);
}


static int buttonspressed = 0;	

static void CanvasMouse(XEvent *event)
{
  static int lastx, lasty;
  int x, y;

  if (canvasmode == CANVASMODE_RENDER)
    return;	

  switch (event->type) {
  case ButtonPress: 
    x = lastx = event->xbutton.x;
    y = lasty = event->xbutton.y;
    
    switch (event->xbutton.button) {
    case Button1: buttonspressed |= 1; break;
    case Button2: buttonspressed |= 2; break;
    case Button3: 
      if (twobutton_motion) 
	CanvasClick(event);
      else
	buttonspressed |= 4;
      break;
    default: break;
    }

    break;
	
  case ButtonRelease: 
    x = event->xbutton.x;
    y = event->xbutton.y;

    if (canvasmode == CANVASMODE_SELECT_PATCH || 
		canvasmode == CANVASMODE_SELECT_PIXEL) {
      switch (buttonspressed) {
      case 1:	
	if(canvasmode == CANVASMODE_SELECT_PATCH)
	  SelectPatch(event->xbutton.x, event->xbutton.y);
	else
	  SelectPixel(event->xbutton.x, event->xbutton.y);
	break;
      default: 
	CanvasPullMode();
	break;
      }
    } else {
#ifdef SLOW_RENDERER
      
      if (lastx != x || lasty != y)
	DoMotion(x, y, lastx, lasty, buttonspressed);

      lastx = x;
      lasty = y;
#endif 
    }

    switch (event->xbutton.button) {
    case Button1: buttonspressed &= ~1; break;
    case Button2: buttonspressed &= ~2; break;
    case Button3:
      if (!twobutton_motion)
	buttonspressed &= ~4;
      break;
    default: break;
    }

    break;

#ifndef SLOW_RENDERER	
  case MotionNotify:
    x = event->xmotion.x;
    y = event->xmotion.y;
 
    DoMotion(x, y, lastx, lasty, buttonspressed);

    lastx = x;
    lasty = y;
    break;
#endif 

  default:
    
    break;
  }
}

static void CanvasMouseEvent(Widget canvas, XtPointer client_data, XEvent *event)
{
  CanvasMouse(event);
}


static KeySym GetKey(XKeyEvent *event)
{
  char buf[20];
  int bufsize = 20;
  KeySym key;
  XComposeStatus compose;
  int charcount;

  charcount = XLookupString(event, buf, bufsize, &key, &compose);
  buf[charcount] = '\0';

#ifdef DEBUG
  switch (key) {
  case XK_Shift_L: fprintf(stderr, "Shift_L "); break;
  case XK_Shift_R: fprintf(stderr, "Shift_R "); break;
  case XK_Shift_Lock: fprintf(stderr, "Shift_Lock "); break;
  case XK_Caps_Lock: fprintf(stderr, "Caps_Lock "); break;
  default: fprintf(stderr, "Key %x '%s' ", (unsigned)key, buf);
  }
#endif
  return key;
}


static void CanvasKey(XKeyEvent *event)
{
  
  switch (GetKey(event)) {
  case XK_Q: exit(0);  break;
    
  default:
    
    break;
  }
}

static void CanvasKeyEvent(Widget canvas, XtPointer client_data, XEvent *event)
{
  CanvasKey((XKeyEvent *)event);
}


void ExternalCanvasEvent(XEvent *event)
{
  switch (event->type) {
  case Expose:
    
    while (XCheckWindowEvent(canvas_display, canvas_window, ExposureMask, event)) {}
    ExternalCanvasExpose();
    break;

  case ConfigureNotify:
    CanvasResize(event->xconfigure.width, event->xconfigure.height);
    break;

  case KeyPress: case KeyRelease:
    CanvasKey((XKeyEvent *)event);
    break;

  case ButtonPress: case ButtonRelease:
    CanvasMouse(event);
    break;

  case MotionNotify:
    
    while (XCheckWindowEvent(canvas_display, canvas_window, ButtonMotionMask, event)) {}
    CanvasMouse(event);
    break;

  case DestroyNotify:
    fprintf(stderr, "External canvas window destroyed!\n");
    exit(0);
    break;

  default:
    
    break;
  }
}








