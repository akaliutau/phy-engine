

#include <time.h>
#include "potential.h"
#include "radiance.h"
#include "patch.h"
#include "patch_flags.h"
#include "vertex.h"
#include "camera.h"
#include "render.h"
#include "scene.h"
#include "canvas.h"
#include "statistics.h"
#include "pools.h"
#include "error.h"
#include "SoftIds.H"




void UpdateDirectPotential(void)
{
  PATCH **id2patch;
  unsigned long *ids, *id;
  long i, j, x, y, nrpixels, maxpatchid, lostpixels;
  VECTOR pixdir;
  float v, h, xsample, ysample;
  float *new_direct_importance, delta_importance, pixarea;

  CanvasPushMode(CANVASMODE_RENDER);

  
  ids = RenderIds(&x, &y);

  CanvasPullMode();

  if (!ids) return;
  nrpixels = x*y; lostpixels=0;

  
  maxpatchid = PatchGetNextID()-1;  
  id2patch = (PATCH **)Alloc((int)(maxpatchid+1) * sizeof(PATCH *));
  for (i=0; i<=maxpatchid; i++) id2patch[i] = (PATCH *)NULL;
  ForAllPatches(P, Patches) {
    id2patch[P->id] = P;
  } EndForAll;

  
  new_direct_importance = (float *)Alloc((int)(maxpatchid+1) * sizeof(float));
  for (i=0; i<=maxpatchid; i++) new_direct_importance[i] = 0.;

  
  h = 2. * tan(Camera.hfov * M_PI/180.) / (float)x;
  v = 2. * tan(Camera.vfov * M_PI/180.) / (float)y;
  pixarea = h * v;

  for (j=y-1, ysample=-v*(float)(y-1)/2.; j>=0; j--, ysample+=v) {
    id = ids + j*x;
    for (i=0, xsample=-h*(float)(x-1)/2.; i<x; i++, id++, xsample+=h) {
      unsigned long the_id = (*id)&0xffffff;

      if (the_id > 0 && the_id <= maxpatchid) {
	
	VECTORCOMB3(Camera.Z, xsample, Camera.X, ysample, Camera.Y, pixdir);

	
	delta_importance = VECTORDOTPRODUCT(Camera.Z, pixdir) / VECTORDOTPRODUCT(pixdir, pixdir);
	delta_importance *= delta_importance * pixarea;

	new_direct_importance[the_id] += delta_importance;
      } else if (the_id > maxpatchid)
	lostpixels++;
    }
  }

  if (lostpixels > 0)
    Warning(NULL, "%d lost pixels", lostpixels);

  average_direct_potential = total_direct_potential = 
    max_direct_potential = max_direct_importance = 0.;
  for (i=1; i<=maxpatchid; i++) {
    PATCH *patch = id2patch[i];

    patch->direct_potential = new_direct_importance[i] / patch->area;

    if (patch->direct_potential > max_direct_potential)
      max_direct_potential = patch->direct_potential;
    total_direct_potential += new_direct_importance[i];
    average_direct_potential += new_direct_importance[i];

    if (new_direct_importance[i] > max_direct_importance)
      max_direct_importance = new_direct_importance[i];
  }
  average_direct_potential /= total_area;

  Free((char *)new_direct_importance, (int)(maxpatchid+1) * sizeof(float));
  Free((char *)id2patch, (int)(maxpatchid+1) * sizeof(PATCH *));
  Free((char *)ids, (int)nrpixels * sizeof(unsigned long));
}

void RenderDirectPotential(void)
{
  ForAllPatches(P, Patches) {
    float gray = P->direct_potential;
    if (gray > 1.) gray = 1.;
    if (gray < 0.) gray = 0.;
    RGBSET(P->color, gray, gray, gray);
  } EndForAll;

  RenderNewDisplayList();
  renderopts.render_raytraced_image = FALSE;
  RenderScene();
}

static DRAW_PIXEL PatchPointer(PATCH *P)
{
  return (DRAW_PIXEL)P;
}

static void SoftGetPatchPointers(DRAW_CONTEXT *draw)
{
  DRAW_PIXEL *pix;
  int i;

  ForAllPatches(P, Patches) {
    PATCH_SET_INVISIBLE(P);      
  } EndForAll;

  for (pix = draw->fbuf, i=0; i<draw->width * draw->height; pix++, i++) {
    PATCH *P = (PATCH *)(*pix);
    if (P)
      PATCH_SET_VISIBLE(P);  
  }
}

void SoftUpdateDirectVisibility(void)
{
  clock_t t = clock();
  DRAW_CONTEXT *olddraw = drawGetCurrent();
  DRAW_CONTEXT *draw = SetupSoftFrameBuffer();
  SoftRenderPatches(PatchPointer);
  SoftGetPatchPointers(draw);
  drawClose(draw);
  drawMakeCurrent(olddraw);

  fprintf(stderr, "Determining visible patches in software took %g sec\n",
	  (float)(clock() - t) / (float)CLOCKS_PER_SEC);
}


void UpdateDirectVisibility(void)
{
#ifdef SOFT_ID_RENDERING
  CanvasPushMode(CANVASMODE_RENDER);
  SoftUpdateDirectVisibility();
  CanvasPullMode();
#else 
  PATCH **id2patch;
  unsigned long *ids, *id;
  long i, x, y, nrpixels, maxpatchid;

  CanvasPushMode(CANVASMODE_RENDER);

  
  ids = RenderIds(&x, &y);

  CanvasPullMode();

  if (!ids) return;
  nrpixels = x*y;

  
  maxpatchid = PatchGetNextID()-1;  
  id2patch = (PATCH **)Alloc((int)(maxpatchid+1) * sizeof(PATCH *));
  for (i=0; i<=maxpatchid; i++) id2patch[i] = (PATCH *)NULL;
  ForAllPatches(P, Patches) {
    id2patch[P->id] = P;
  } EndForAll;

  for (i=0,id=ids; i<nrpixels; i++, id++) {
    unsigned long the_id = (*id)&0xffffff;
    PATCH *P = id2patch[the_id];
    if (P) {
      PATCH_SET_VISIBLE(P);

      
      id2patch[the_id] = (PATCH*)NULL;
    }
  }

  
  for (i=0; i<=maxpatchid; i++) {
    PATCH *P = id2patch[i];
    if (P) {
      PATCH_SET_INVISIBLE(P);
    }
  }

  Free((char *)id2patch, (int)(maxpatchid+1) * sizeof(PATCH *));
  Free((char *)ids, (int)nrpixels * sizeof(unsigned long));
#endif 
}

