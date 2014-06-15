/* raymatting.h: create mattes of the current scene with filtering */
/*                                                                 */
/* original version by Vincent Masselus                            */
/* addapted by Pieter Peers (2001-06-01)                           */

#ifndef _RAYMATTE_H_
#define _RAYMATTE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "raytracing.h"

extern RAYTRACINGMETHOD RayMatting;

/* Ray-Casts the current Scene-Matt. Output is displayed on the sceen
 * edges are filtered.                                                */
extern void RayMatte(char *fname, FILE *fp, int ispipe);

#ifdef __cplusplus
}
#endif

#endif /*_RAYMATTE_H_*/
