/* polygon.h */

#ifndef _POLYGON_H_
#define _POLYGON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "patch_type.h"

/* A structure describing polygons. Only used for shaftculling for the moment. */
typedef struct POLYGON {
  VECTOR normal;
  float plane_constant;
  BOUNDINGBOX bounds;
  POINT vertex[PATCHMAXVERTICES];
  int nrvertices, index;
} POLYGON;

#ifdef __cplusplus
}
#endif

#endif /*_POLYGON_H_*/
