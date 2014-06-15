/* ui_pathdebug.h: declaration of functions for debugging paths */

#ifndef _UI_PATHDEBUG_H_
#define _UI_PATHDEBUG_H_

#ifdef __cplusplus
extern "C" {
#endif

/* This functions shows the path debug box that
   has all necessary path debug functionality */
void ShowPathDebugPanel(Widget w, XtPointer client_data, 
						XtPointer call_data);

/* InitPathDebug() releases possible existing paths */
void InitPathDebug(void);


#ifdef __cplusplus
}
#endif

#endif /* _UI_PATHDEBUG_H_ */
