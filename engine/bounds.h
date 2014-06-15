/* bounds.h: bounding boxes */

#ifndef _BOUNDS_H_
#define _BOUNDS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "vector.h"
#include "ray.h"
#include "transform.h"

/* a bounding box is represented as a n array of 6 floating point numbers.
 * The meaning of the numbers is given by the constants MIN_X ... below. */
typedef float BOUNDINGBOX[6];

/* 
 * the following defines must obay the following rules:
 * 1) DIR_X = (MIN_X)%3 and DIR_X = (MAX_X)%3 and same for DIR_Y, DIR_Z 
 * 2) (MIN_X+3)%6 = MAX_X and (MAX_X+3)%6 = MIN_X and same for MIN_Y, ...
 * 3) MIN_X+1 = MIN_Y, MIN_Y+1 = MIN_Z and MAX_X+1 = MAX_Y, MAX_Y+1 = MAX_Z 
 */
#define MIN_X 	0
#define MIN_Y 	1
#define MIN_Z 	2
#define MAX_X 	3
#define MAX_Y 	4
#define MAX_Z 	5

#define offsetname(i) (i <= MIN_Z ? ( i==MIN_X ? "MIN_X" : 		\
				     (i==MIN_Y ? "MIN_Y" : "MIN_Z")) : 	\
		                    ( i==MAX_X ? "MAX_X" : 		\
				     (i==MAX_Y ? "MAX_Y" : "MAX_Z")))

#define DIR_X	0
#define DIR_Y	1
#define DIR_Z	2

#define dirname(i) (i==DIR_X ? "X" : (i==DIR_Y ? "Y" : "Z"))

extern float * BoundsCreate(void);
extern float * BoundsCopy(float * from, float * to);
extern void BoundsPrint(FILE *out, float * box);
extern void BoundsDestroy(float *bounds);

extern float * BoundsInit(float * bounds);
extern float * BoundsEnlarge(float * bounds, float * extra);
extern float * BoundsEnlargePoint(float * bounds, VECTOR *point);

/* OutOfBounds(VECTOR *p, float * bounds) */
#define OutOfBounds(p, bounds) 	(((p)->x < (bounds)[MIN_X] || (p)->x > (bounds)[MAX_X]) || \
				 ((p)->y < (bounds)[MIN_Y] || (p)->y > (bounds)[MAX_Y]) || \
				 ((p)->z < (bounds)[MIN_Z] || (p)->z > (bounds)[MAX_Z]))

/* "returns" nonzero if the two given boundingboxes are disjunct */
#define DisjunctBounds(b1, b2) ((b1[MIN_X] > b2[MAX_X]) || (b2[MIN_X] > b1[MAX_X]) || \
				(b1[MIN_Y] > b2[MAX_Y]) || (b2[MIN_Y] > b1[MAX_Y]) || \
				(b1[MIN_Z] > b2[MAX_Z]) || (b2[MIN_Z] > b1[MAX_Z]))

extern int BoundsIntersect(RAY *ray, float * bounds, float mindist, float *maxdist);

/* returns TRUE if the boundingbox is behind the plane defined by norm and d */
extern int BoundsBehindPlane(float *bounds, VECTOR *norm, float d);

/* Computes boundingbox after transforming bbx with xf. Result is filled
 * in transbbx and a pointer to transbbx returned. */
extern float *BoundsTransform(float *bbx, TRANSFORM *xf, float *transbbx);

/* This routine computes the segment of intersection of the ray and the bounding box.
 * On input, tmin and tmax contain minimum and maximum allowed distance to the ray 
 * origin. On output, tmin and tmin contain the distance to the eye origin of
 * the intersection points of the ray with the boundingbox.
 * If there are no intersection in the given interval, FALSE is returned. If there
 * are intersections, TRUE is returned. */
extern int BoundsIntersectingSegment(RAY *ray, float *bounds, float *tmin, float *tmax);

#ifdef __cplusplus
}
#endif

#endif /*_BOUNDS_H_*/
