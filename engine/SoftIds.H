/* softids.h: software ID rendering */

#ifndef _SOFTIDS_H_
#define _SOFTIDS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "graphicutils/draw.h"
#include "patch.h"

/* sets up a software rendering context and initialises transforms and 
 * viewport for the current view. The new renderer is made current. */
extern DRAW_CONTEXT *SetupSoftFrameBuffer(void);

/* renders all Patches in the current draw renderer. PatchPixel returns
 * and DRAW_PIXEL value for a given PATCH */
extern void SoftRenderPatches(DRAW_PIXEL (*PatchPixel)(PATCH *));

/* software ID rendering */
extern unsigned long *SoftRenderIds(long *x, long *y);

#ifdef __cplusplus
}
#endif

#endif
