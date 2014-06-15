/* bidirpath.h : header for bidirectional path tracing */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _BIDIRPATH_H_
#define _BIDIRPATH_H_

#include "raytracing.h"
#include "color.h"
extern RAYTRACINGMETHOD RT_BidirPathMethod;

COLOR Bidir_GetPixel(int nx, int ny);

#endif /* _BIDIRPATH_H_ */

#ifdef __cplusplus
}
#endif
