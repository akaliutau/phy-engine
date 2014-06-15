/* geometry3d.h: 3D plane-to-plane, plane-to-line, ... intersection and
 * similar stuff. */

#ifndef _GEOMETRY3D_H_
#define _GEOMETRY3D_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Boolean.h"
#include "vector.h"

/* returns TRUE if the planes defines by norm1, d1 and norm2, d2 intersect and
 * FALSE if they don't. If they intersect, the intersection line direction
 * is filled in in 'd' and a point on the intersection line in 'p'. */
extern int PlaneToPlaneIntersection(DVECTOR *norm1, double d1, DVECTOR *norm2, double d2, 
				    DPOINT *p, DVECTOR *d);

/* Computes line to plane intersction. linept is a point on the line, linedir
 * is the line direction, norm and d define the plane. If there is an intersection,
 * the intersection point line parameter is filled in in 't'. The intersection
 * point is linept + t x linedir. If there isn't an intersection, FALSE is returned. */
extern int LineToPlaneIntersection(DPOINT *linept, DVECTOR *linedir, 
				   DVECTOR *norm, double d, double *t);

#ifdef __cplusplus
}
#endif

#endif /*_GEOMETRY3D_H_*/
