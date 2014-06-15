/* interface.h: some global variables related to the graphics interface */

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <Xm/Xm.h>

/* application class name --- determines the name of the application defaults file
 * for the application. */
#define APP_CLASS_NAME			"Phy2"

/* the program detects the best X visual to use and creates a suitables
 * standard colormap for use with it automatically. visual.c
 * contains the routijnes to do the choice */
extern XVisualInfo best_visual;
extern Colormap cmap;

/* the X visual info is not correctly passed from parent to sub menus
 * with Motif 1.2, so we set it explicitely ourselves when creating
 * the submenus */
extern Arg visargs[3];
extern int nrvisargs;

/* X application context and some important widgets */
extern XtAppContext app_context;
extern Widget topLevel, canvas;
extern Display *display;

/* current directory needed when loading MGF files. */
extern char *current_directory;

/* Called during radiance computations - checks for events (like the user clicking
 * on some button, or moving the mouse while holding one of the mouse buttons ... ) 
 * makes the program behave much more flexible.  */
extern void ProcessWaitingEvents(void);

/* same as above, but avoids checking more than once every second (using the clock() 
 * function and a static clock counter). */
extern void CheckForEvents(void);

/* Tries to read the scene in the given file. Returns False if not succesful.
 * Returns True if succesful. When a file cannot be read, the current scene is 
 * restored. */
extern Boolean ReadFile(char *filename);

/* returns TRUE if the program thinks it is working and FALSE if it
 * thinks it is not working. The program thinks it is working when the 
 * current canvas mode is CANVASMODE_WORKING, see canvas.h. This
 * routine is used for e.g. warning the user about options that can't be
 * changed until the computations are interrupted. */
extern int Working(void);

/* sets up the user interface and starts the application main loop. */
extern void StartUserInterface(int *argc, char **argv);

/* external declarations of submenu creation functions. */
extern void CreateFileMenu(Widget menuBar),
            CreateRadianceMenu(Widget menuBar),
            CreateRenderMenu(Widget menuBar),
            CreateToneMappingMenu(Widget menuBar),
            CreateRayTracingMenu(Widget menuBar),
            CreateCameraMenu(Widget menuBar),
            CreateDebugMenu(Widget menuBar),
            CreateHelpMenu(Widget menuBar);

extern void CreateCanvasPopupMenu(void);

/* external declarations of extra ui config routines */
extern void LoadUserOptions(void);
extern void SaveUserOptions(void);

/* command line option processing related to the user interface */
extern void ParseInterfaceOptions(int *argc, char **argv);
extern void PrintInterfaceOptions(FILE *fp);
extern void InterfaceDefaults(void);

extern void SetWindowTitle(char *title);

#ifdef __cplusplus
}
#endif

#endif /*_INTERFACE_H_*/
