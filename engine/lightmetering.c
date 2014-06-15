

#include "lightmetering.h"
#include "grid.h"
#include "camera.h"
#include "render.h"
#include "scene.h"
#include "radiance.h"
#include "imageutil/image.h"

#define NUM_H 32
#define NUM_V 32
static int num_samples = NUM_H * NUM_V;

float DetermineAverageIncidentLuminance(void)
{
  float h, v;
  int i, accepted_samples;
  int x = Camera.hres, y = Camera.vres;
  RAY ray;
  HITREC *hit, hitstore;
  float avlum = 0.;

  if (!Radiance)
    return 0.;

  
  h = 2. * tan(Camera.hfov * M_PI/180.) / (float)x;
  v = 2. * tan(Camera.vfov * M_PI/180.) / (float)y;

  
  ray.pos = Camera.eyep;

  accepted_samples = 0; avlum = 0.;

  for (i=0; i<num_samples; i++) {
    float xsample = -h*(float)(x-1)/2. + ((float)(i%NUM_H) + drand48())/(float)NUM_H *x*h;
    float ysample = -v*(float)(y-1)/2. + ((float)(i/NUM_H) + drand48())/(float)NUM_V *y*v;
    float dist;

    
    VECTORCOMB3(Camera.Z, xsample, Camera.X, ysample, Camera.Y, ray.dir);
    VECTORNORMALIZE(ray.dir);

    dist = HUGE;
    hit = GridIntersect(WorldGrid, &ray, 0., &dist, HIT_FRONT|HIT_PATCH|HIT_POINT, &hitstore);
    if (hit) {
      COLOR rad;
      double u, v;
      VECTOR dir;
      float max;

      RenderSetColor(&Blue);
      RenderLine(&hit->point, &hit->point);

      VECTORSCALE(-1., ray.dir, dir);
      PatchUV(hit->patch, &hit->point, &u, &v);

      rad = Radiance->GetRadiance(hit->patch, u, v, dir);
      if (hit->patch->surface->material->edf) {
	
	COLOR ed = EdfDiffuseRadiance(hit->patch->surface->material->edf, hit);
	COLORSUBTRACT(rad, ed, rad);
      }
      max = COLORMAXCOMPONENT(rad);
      if (max > 1e-32) {
	accepted_samples ++;
	avlum += max;
      }
    }
  }

  if (accepted_samples > 0)
    return avlum / (float)accepted_samples;
  else
    return 0.;
}

