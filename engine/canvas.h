/* canvas.h: handles mouse and expose events and cursor shape in the canvas 
 * window. */

#ifndef _CANVAS_H_INCLUDED_
#define _CANVAS_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <Xm/Xm.h>

/* A struct for keeping the type of the canvas window */
typedef enum {INTEGRATED_CANVAS, EXTERNAL_CANVAS, OFFSCREEN_CANVAS} CANVAS_TYPE;

/* creates an integrated canvas window with given resolution. If hres or vres is 0,
 * the resolution from the application defaults resource database is used. */
extern Widget CreateIntegratedCanvasWindow(Widget parent, Dimension hres, Dimension vres);

/* connects to an external canvas window with given ID on the given display.
 * The display needs not be the same display as the one containing the rest of
 * the user interface. */
extern void ConnectExternalCanvasWindow(Display *dpy, Window w);

/* creates an offscreen canvas "window" with given horizontal and vertical
 * resolution. */
extern void CreateOffscreenCanvasWindow(int hres, int vres);

/* initializes event handling and cursor shape on the canvas window */
extern void CanvasInit(void);

/* canvas modes: canvas modes determine what cursor shape will be used on the 
 * canvas window and how the program will react on mouse events. */
typedef enum {

/* Moving the mouse while pressing one of the buttons updates the camera. 
 * The cursor shape is the default cursor. This mode is used when the program is
 * not busy computing or rendering and when not selecting a patch. It is the
 * default canvas mode set when calling CanvasInit(). */
  CANVASMODE_NORMAL,

/* Moving the mouse while pressing one of the mouse buttons updates the camera,
 * the a clock cursor is shown instead of the default (arrow) cursor. This mode
 * is used to indicate that computations are going on. */
  CANVASMODE_WORKING,

/* This mode is used to select a patch. The cursor shown is a crosshair cursor.
 * When the first mouse button is pressed and then released, the patch seen through the
 * pixel at the current mouse position is selected. Another mouse button will cancel
 * the selection. */
  CANVASMODE_SELECT_PATCH,

/* This mode is used to select a pixel. The cursor shown is a crosshair cursor.
 * When the first mouse button is pressed and then released, the patch seen through the
 * pixel at the current mouse position is selected. Another mouse button will cancel
 * the selection. */
  CANVASMODE_SELECT_PIXEL,

/* A spraycan cursor is shown. This mode indicates that the scene is being 
 * rendered. Mouse events are ignored during rendering. */
  CANVASMODE_RENDER

} CANVAS_MODE;

/* Saves the current canvas mode and sets a new */
extern void CanvasPushMode(CANVAS_MODE canvasmode);

/* restores the previous canvas mode */
extern void CanvasPullMode(void);

/* returns the current canvas mode. */
extern CANVAS_MODE CanvasGetMode(void);

/* Takes care of possible expose events immediately: if the scene needs to be
 * redrawn, this routine removes the installed redraw work procedure and immediately
 * rerenders the scene. A redraw work procedure is used for expose event compression:
 * Normally, the scene is redrawn in the background, after all pending events have been
 * processed. */
extern void CanvasRedraw(void);

/* Handles events on an external canvas window (which may even reside
 * on a different display than the rest of the GUI) */
extern void ExternalCanvasEvent(XEvent *event);

/* post a request for redrawing the scene. */
extern void CanvasPostRedraw(void);

/* cancels previous redrawing requests due to e.g. expose events on the 
 * canvas window. */
extern void CanvasCancelRedraw(void);

/* implemented in ui_canvas.c */
#include "patch_type.h"
extern void PopUpCanvasMenu(XEvent *event, PATCH *patch, POINT *point);

extern int twobutton_motion;	/* initialized and set in ui_main.c */

#ifdef __cplusplus
}
#endif

#endif /*_CANVAS_H_INCLUDED_*/
