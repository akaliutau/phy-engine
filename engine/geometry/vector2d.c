#include "vector2d.h"

#include "Float.h"

VEC2D origin2d = {0.0,0.0};

void VEC2DPointOnLineNearestPoint(VEC2D *pt,VEC2D *lineorg,VEC2D *linedir,VEC2D *res)
{
  VEC2D  l;
  float  c,q;
  
  VEC2DPERP(*linedir,l);
  c = -VEC2DDOTPRODUCT(l,*lineorg);
  
  q = -(c + VEC2DDOTPRODUCT(l,*pt));
  VEC2DADDSCALED(*pt,q,l,*res);
}

void VEC2DReflectPointLine(VEC2D *pt, VEC2D *lineorg, VEC2D *linedir, VEC2D *res)
{
  VEC2DPointOnLineNearestPoint(pt,lineorg,linedir,res);
  VEC2DSUBTRACT(*res,*pt,*res);
  VEC2DADDSCALED(*pt,2.0,*res,*res);
}


double VEC2DAngle(VEC2D* a,VEC2D* b)
{
  double  sinD,cosD,ac;
  
  sinD = VEC2DCROSSPRODUCT(*a,*b);
  cosD = VEC2DDOTPRODUCT(*a,*b);

  
  if (sinD>1.0)  sinD=1.0;
  if (sinD<-1.0) sinD=-1.0;
  if (cosD>1.0)  cosD=1.0;
  if (cosD<-1.0) cosD=-1.0;

  ac = acos(cosD);
  if (sinD<0.0)
    ac = 2.0*M_PI-ac;

  return ac;
}
  
int    intersectLineWithCircle(VEC2D *origin,VEC2D *dir,VEC2D *center,double r2)
{
  VEC2D  eo;
  double v,disc,l;
  
  VEC2DSUBTRACT(*center,*origin,eo);
  v = VEC2DDOTPRODUCT(eo,*dir);
  l = VEC2DDOTPRODUCT(eo,eo);
  disc = r2 - (l - (v*v));
  
  
  if (disc > 0.0)
    return 2;

  if (disc == 0.0)
    return 1;

  return 0;
}


