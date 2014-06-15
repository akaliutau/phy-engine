

#include "select.h"
#include "ray.h"
#include "camera.h"
#include "scene.h"
#include "render.h"
#include "canvas.h"
#include "error.h"
#include "grid.h"

static void (*SelectPatchCallbackProc)(PATCH *patch, VECTOR *hitp) = NULL;
static void (*SelectPixelCallbackProc)(int pixx, int pixy) = NULL;

void SelectPatchSetCallback(void (*proc)(PATCH *patch, VECTOR *hitp))
{
  SelectPatchCallbackProc = proc;
}

void SelectPatch(int pixx, int pixy)
{
  RAY ray;
  float v, h, x, y, dist;
  HITREC *hit, hitstore;

  CanvasPullMode();

  h = 2. * tan(Camera.hfov * M_PI/180.) / (float)(Camera.hres-1);
  v = 2. * tan(Camera.vfov * M_PI/180.) / (float)(Camera.vres-1);
  x = h * ((float)pixx - (float)(Camera.hres-1)/2.);
  y = v * ((float)pixy - (float)(Camera.vres-1)/2.);
  
  ray.pos = Camera.eyep;
  VECTORCOMB3(Camera.Z, x, Camera.X, y, Camera.Y, ray.dir);
  VECTORNORMALIZE(ray.dir);

  dist = HUGE;
  
  if ((hit = GridIntersect(WorldGrid, &ray, 0., &dist,
	     HIT_POINT|HIT_FRONT|(renderopts.backface_culling ? 0 : HIT_BACK), &hitstore))) {
    hit->normal = PatchInterpolatedNormalAtPoint(hit->patch, &hit->point);

    
    RenderSetColor(&renderopts.outline_color);
    RenderPatchOutline(hit->patch);
    RenderPatchNormals(hit->patch);
    RenderFinish();

    
    if (SelectPatchCallbackProc) {
      static PATCH *hitpatch;
      static POINT hitpoint; 
      hitpatch = hit->patch;
      hitpoint = hit->point;
      SelectPatchCallbackProc(hitpatch, &hitpoint);
    }
  } else
    Error(NULL, "No surfaces there.");
}


void SelectPixelSetCallback(void (*proc)(int pixx, int pixy))
{
  SelectPixelCallbackProc = proc;
}

void SelectPixel(int pixx, int pixy)
{
  

  CanvasPullMode();

  if(SelectPixelCallbackProc)
  {
	SelectPixelCallbackProc(pixx, pixy);
  }
}
