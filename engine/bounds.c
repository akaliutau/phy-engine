
#include <stdio.h>
#include <math.h>
#include "bounds.h"
#include "Boolean.h"
#include "extmath.h"
#include "error.h"
#include "pools.h"

#ifdef NOPOOLS
#define NEWBOUNDINGBOX()	(float *)Alloc(sizeof(BOUNDINGBOX))
#define DISPOSEBOUNDINGBOX(ptr) Free((char *)ptr, sizeof(BOUNDINGBOX))
#else
static POOL *boundsPool = (POOL *)NULL;
#define NEWBOUNDINGBOX()	(float *)NewPoolCell(sizeof(BOUNDINGBOX), 0, "boundingboxes", &boundsPool)
#define DISPOSEBOUNDINGBOX(ptr) Dispose((char *)ptr, &boundsPool)
#endif

float *BoundsCreate(void)
{
  float *bounds;

  bounds = NEWBOUNDINGBOX();
  return bounds;
}

void BoundsDestroy(float *bounds)
{
  DISPOSEBOUNDINGBOX(bounds);
}

float *BoundsInit(float *bounds)
{
  bounds[MIN_X] = bounds[MIN_Y] = bounds[MIN_Z] = HUGE;
  bounds[MAX_X] = bounds[MAX_Y] = bounds[MAX_Z] = -HUGE;
  return bounds;
}

#define SetIfLess(a, b)		(a = ((a) < (b) ? (a) : (b)))
#define SetIfGreater(a, b)	(a = ((a) > (b) ? (a) : (b)))


float *BoundsEnlarge(float *bounds, float *extra)
{
  SetIfLess(bounds[MIN_X], extra[MIN_X]);
  SetIfLess(bounds[MIN_Y], extra[MIN_Y]);
  SetIfLess(bounds[MIN_Z], extra[MIN_Z]);
  SetIfGreater(bounds[MAX_X], extra[MAX_X]);
  SetIfGreater(bounds[MAX_Y], extra[MAX_Y]);
  SetIfGreater(bounds[MAX_Z], extra[MAX_Z]);
  return bounds;
}

float *BoundsEnlargePoint(float *bounds, POINT *point)
{	
  SetIfLess(bounds[MIN_X], point->x);
  SetIfLess(bounds[MIN_Y], point->y);
  SetIfLess(bounds[MIN_Z], point->z);
  SetIfGreater(bounds[MAX_X], point->x);
  SetIfGreater(bounds[MAX_Y], point->y);
  SetIfGreater(bounds[MAX_Z], point->z);
  return bounds;
}

float *BoundsCopy(float *from, float *to)
{
  to[MIN_X] = from[MIN_X];
  to[MIN_Y] = from[MIN_Y];
  to[MIN_Z] = from[MIN_Z];
  to[MAX_X] = from[MAX_X];
  to[MAX_Y] = from[MAX_Y];
  to[MAX_Z] = from[MAX_Z];
  return to;
}

void BoundsPrint(FILE *out, float *box)
{
  fprintf(out, "\tX: %f to %f\n", box[MIN_X], box[MAX_X]);
  fprintf(out, "\tY: %f to %f\n", box[MIN_Y], box[MAX_Y]);
  fprintf(out, "\tZ: %f to %f\n", box[MIN_Z], box[MAX_Z]);
}




int BoundsIntersectingSegment(RAY *ray, float *bounds, float *tmin, float *tmax)
{
  float t, mindist, maxdist;
  float dir, pos;

  mindist = *tmin;
  maxdist = *tmax;

  dir = ray->dir.x;
  pos = ray->pos.x;

  if (dir < 0) {
    t = (bounds[MIN_X] - pos) / dir;
    if (t < *tmin)
      return FALSE;
    if (t <= *tmax)
      *tmax = t;
    t = (bounds[MAX_X] - pos) / dir;
    if (t >= *tmin) {
      if (t > *tmax * (1.+EPSILON))
	return FALSE;
      *tmin = t;
    }
  } else if (dir > 0.) {
    t = (bounds[MAX_X] - pos) / dir;
    if (t < *tmin)
      return FALSE;
    if (t <= *tmax)
      *tmax = t;
    t = (bounds[MIN_X] - pos) / dir;
    if (t >= *tmin) {
      if (t > *tmax * (1.+EPSILON))
	return FALSE;
      *tmin = t;
    }
  } else if (pos < bounds[MIN_X] || pos > bounds[MAX_X])
    return FALSE;

  dir = ray->dir.y;
  pos = ray->pos.y;

  if (dir < 0) {
    t = (bounds[MIN_Y] - pos) / dir;
    if (t < *tmin)
      return FALSE;
    if (t <= *tmax)
      *tmax = t;
    t = (bounds[MAX_Y] - pos) / dir;
    if (t >= *tmin) {
      if (t > *tmax * (1.+EPSILON))
	return FALSE;
      *tmin = t;
    }
  } else if (dir > 0.) {
    t = (bounds[MAX_Y] - pos) / dir;
    if (t < *tmin)
      return FALSE;
    if (t <= *tmax)
      *tmax = t;
    t = (bounds[MIN_Y] - pos) / dir;
    if (t >= *tmin) {
      if (t > *tmax * (1.+EPSILON))
	return FALSE;
      *tmin = t;
    }
  } else if (pos < bounds[MIN_Y] || pos > bounds[MAX_Y])
    return FALSE;
  
  dir = ray->dir.z;
  pos = ray->pos.z;
  
  if (dir < 0) {
    t = (bounds[MIN_Z] - pos) / dir;
    if (t < *tmin)
      return FALSE;
    if (t <= *tmax)
      *tmax = t;
    t = (bounds[MAX_Z] - pos) / dir; 
    if (t >= *tmin) {
      if (t > *tmax * (1.+EPSILON))
	return FALSE;
      *tmin = t;
    }
  } else if (dir > 0.) {
    t = (bounds[MAX_Z] - pos) / dir;
    if (t < *tmin)
      return FALSE;
    if (t <= *tmax)
      *tmax = t;
    t = (bounds[MIN_Z] - pos) / dir;
    if (t >= *tmin) {
      if (t > *tmax * (1.+EPSILON))
	return FALSE;
      *tmin = t;
    }
  } else if (pos < bounds[MIN_Z] || pos > bounds[MAX_Z])
    return FALSE;

  
  if (*tmin == mindist) {
    if (*tmax < maxdist) {
      return TRUE;
    }
  } else {
    if (*tmin < maxdist) {
      return TRUE;
    }
  }
  return FALSE;	
}

int BoundsIntersect(RAY *ray, float *bounds, float mindist, float *maxdist)
{
  float tmin, tmax;
  int hit;

  tmin = mindist;
  tmax = *maxdist;
  hit = BoundsIntersectingSegment(ray, bounds, &tmin, &tmax);
  if (hit) {
    
    if (tmin == mindist) {
      if (tmax < *maxdist) 
	*maxdist = tmax;
    } else {
      if (tmin < *maxdist)
	*maxdist = tmin;
    }
  }
  return hit;
}



int BoundsBehindPlane(float *bounds, VECTOR *norm, float d)
{
  VECTOR P;

  if (norm->x > 0.)
    P.x = bounds[MAX_X];
  else
    P.x = bounds[MIN_X];

  if (norm->y > 0.)
    P.y = bounds[MAX_Y];
  else
    P.y = bounds[MIN_Y];

  if (norm->z > 0.)
    P.z = bounds[MAX_Z];
  else
    P.z = bounds[MIN_Z];

  return VECTORDOTPRODUCT(*norm, P) + d <= 0.;
}


float *BoundsTransform(float *bbx, TRANSFORM *xf, float *transbbx)
{
  VECTOR v[8];
  int i;
  double d;

  VECTORSET(v[0], bbx[MIN_X], bbx[MIN_Y], bbx[MIN_Z]);
  VECTORSET(v[1], bbx[MAX_X], bbx[MIN_Y], bbx[MIN_Z]);
  VECTORSET(v[2], bbx[MIN_X], bbx[MAX_Y], bbx[MIN_Z]);
  VECTORSET(v[3], bbx[MAX_X], bbx[MAX_Y], bbx[MIN_Z]);
  VECTORSET(v[4], bbx[MIN_X], bbx[MIN_Y], bbx[MAX_Z]);
  VECTORSET(v[5], bbx[MAX_X], bbx[MIN_Y], bbx[MAX_Z]);
  VECTORSET(v[6], bbx[MIN_X], bbx[MAX_Y], bbx[MAX_Z]);
  VECTORSET(v[7], bbx[MAX_X], bbx[MAX_Y], bbx[MAX_Z]);

  BoundsInit(transbbx);
  for (i=0; i<8; i++) {
    TRANSFORM_POINT_3D(*xf, v[i], v[i]);
    BoundsEnlargePoint(transbbx, &v[i]);
  }

  d = (transbbx[MAX_X] - transbbx[MIN_X]) * EPSILON;
  transbbx[MIN_X] -= d;
  transbbx[MAX_X] += d;
  d = (transbbx[MAX_Y] - transbbx[MIN_Y]) * EPSILON;
  transbbx[MIN_Y] -= d;
  transbbx[MAX_Y] += d;
  d = (transbbx[MAX_Z] - transbbx[MIN_Z]) * EPSILON;
  transbbx[MIN_Z] -= d;
  transbbx[MAX_Z] += d;

  return transbbx;
}
