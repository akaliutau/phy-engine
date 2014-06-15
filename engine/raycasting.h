/* hwrcast.h: hardware-assisted ray casting */

#ifndef _HWRCAST_H_
#define _HWRCAST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "raytracing.h"
extern RAYTRACINGMETHOD RayCasting;

/* Ray-Casts the current Radiance solution. Output is displayed on the sceen
 * and saved into the file with given name and file pointer. 'ispipe' 
 * reflects whether this file pointer is a pipe or not. */
extern void RayCast(char *fname, FILE *fp, int ispipe);

/* Ray-Casts the current Radiance solution and determines the static
 * adaptation luminance for tone mapping after scaling with scalefactor. */
extern double RayCastAdaptationLuminance(float scalefactor);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include "radiance.h"
#include "raytracing/screenbuffer.H"

void RayCast(GETRADIANCE_FT cb, CScreenBuffer *screen = NULL);
#endif



#endif /*_HWRCAST_H_*/
