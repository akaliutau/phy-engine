

#include "mcradP.h"
#include "localline.h"
#include "scene.h"
#include "spherical.h"
#include "ui.h"

#include "render.h"


RAY McrGenerateLocalLine(PATCH *patch, double *xi)
{
  static PATCH *prevpatch = (PATCH *)NULL;
  static COORDSYS coordsys;
  RAY ray;
  double pdf;

  if (patch != prevpatch) {
    
    PatchCoordSys(patch, &coordsys);
    prevpatch = patch;
  }

  PatchUniformPoint(patch, xi[0], xi[1], &ray.pos);
  ray.dir = SampleHemisphereCosTheta(&coordsys, xi[2], xi[3], &pdf);

  return ray;
}


static void SomeFeedback(void)
{
  if ((mcr.traced_rays + mcr.imp_traced_rays)%1000 == 0)
    fputc('.', stderr);

  if (mcr.wake_up) {
    ProcessWaitingEvents();
    mcr.wake_up = FALSE;
  }
}


HITREC *McrShootRay(PATCH *P, RAY *ray, HITREC *hitstore)
{
  float dist = HUGE;
  HITREC *hit;

  
  PatchDontIntersect(2, P, P->twin);
  hit = GridIntersect(WorldGrid, ray, EPSILON < P->tolerance ? EPSILON : P->tolerance, &dist, HIT_FRONT|HIT_POINT, hitstore);
  PatchDontIntersect(0);
  SomeFeedback();

  return hit;
}

