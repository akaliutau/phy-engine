/* vertexutils.h: routines to fix vertex order etc... of the current scene. */

#ifndef _FIX_H_
#define _FIX_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Fixes the vertex order and normal direction for all patches visible in the 
 * current view. */
extern void FixVertexOrderInView(void);

#ifdef __cplusplus
}
#endif

#endif /*_FIX_H_*/
