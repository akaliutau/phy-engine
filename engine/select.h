/* select.h: everything concerning selection of patches */

#ifndef _SELECT_H_
#define _SELECT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "patch_type.h"

/* specify a routine to be called when a patch has been selected. */
extern void SelectPatchSetCallback(void (*proc)(PATCH *patch, VECTOR *hitp));

/* called from CanvasMouseEvent() in canvas.c when a mouse button is released
 * in CANVAS_SELECT_PATCH mode */
extern void SelectPatch(int pixx, int pixy);

/* specify a routine to be called when a patch has been selected. */
extern void SelectPixelSetCallback(void (*proc)(int pixx, int pixy));

/* called from CanvasMouseEvent() in canvas.c when a mouse button is released
 * in CANVAS_SELECT_PIXEL mode */
extern void SelectPixel(int pixx, int pixy);

#ifdef __cplusplus
}
#endif

#endif /*_SELECT_H_*/
