/* potential.h: routines dealing with view potential */

#ifndef _POTENTIAL_H_
#define _POTENTIAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "patch_type.h"

/* Determines the actual directly received potential of the patches in
 * the scene. */
extern void UpdateDirectPotential(void);

/* Updates view visibility status of all patches. */
extern void UpdateDirectVisibility(void);

extern void RenderDirectPotential(void);

#ifdef __cplusplus
}
#endif

#endif /*_POTENTIAL_H_*/
