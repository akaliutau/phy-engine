

#include <math.h>
#include "geometry3d.h"
#include "error.h"

int PlaneToPlaneIntersection(DVECTOR *norm1, double d1, DVECTOR *norm2, double d2, 
			     DPOINT *p, DVECTOR *d)
{
  double dnorm, maxabs, abs;
  int index;

  
  VECTORCROSSPRODUCT(*norm1, *norm2, *d);
  dnorm = VECTORNORM(*d);
  if (dnorm < EPSILON)
    return FALSE;

  
  maxabs = fabs(d->x); index = XNORMAL;
  if ((abs = fabs(d->y)) > maxabs) {maxabs = abs; index = YNORMAL;}
  if ((abs = fabs(d->z)) > maxabs) {maxabs = abs; index = ZNORMAL;}

  switch (index) {
  case XNORMAL:
    p->x = 0.;
    p->y = (norm1->z * d2 - norm2->z * d1) / d->x;
    p->z = (norm2->y * d1 - norm1->y * d2) / d->x;
    break;
  case YNORMAL:
    p->y = 0.;
    p->z = (norm1->x * d2 - norm2->x * d1) / d->y;
    p->x = (norm2->z * d1 - norm1->z * d2) / d->y;
    break;
  case ZNORMAL:
    p->z = 0.;
    p->x = (norm1->y * d2 - norm2->y * d1) / d->z;
    p->y = (norm2->x * d1 - norm1->x * d2) / d->z;
    break;
  default:
    Fatal(2, "PlaneToPlaneIntersection", "something impossible wrong");
  }

  
  VECTORSCALEINVERSE(dnorm, *d, *d);
  return TRUE;
}

int LineToPlaneIntersection(DPOINT *linept, DVECTOR *linedir, 
			    DVECTOR *norm, double d, double *t)
{
  *t = VECTORDOTPRODUCT(*linedir, *norm);
  if (fabs(*t) < EPSILON)
    return FALSE;
  *t = - (VECTORDOTPRODUCT(*linept, *norm) + d) / *t;
  return TRUE;
}

