

#include <string.h>

#include "SoftIds.H"
#include "scene.h"
#include "camera.h"
#include "render.h"
#include "error.h"

DRAW_CONTEXT *SetupSoftFrameBuffer(void)
{
  DRAW_CONTEXT *draw;

  draw = drawOpen(Camera.hres, Camera.vres);
  drawDepthTesting(TRUE);
  drawClipping(TRUE);
  drawClear((DRAW_PIXEL)0, DRAW_ZMAX);

  drawLoadMatrix(Perspective(Camera.fov*2.*M_PI/180., (float)Camera.hres/(float)Camera.vres, Camera.near, Camera.far)); 
  drawMultMatrix(LookAt(Camera.eyep, Camera.lookp, Camera.updir)); 

  return draw;
}

static DRAW_PIXEL (*PatchPixel)(PATCH *) = NULL;

static void SoftRenderPatch(PATCH *P)
{
  VECTOR verts[4];

  if (renderopts.backface_culling &&
      VECTORDOTPRODUCT(P->normal, Camera.eyep) + P->plane_constant < EPSILON)
    return;

  verts[0] = *P->vertex[0]->point;
  verts[1] = *P->vertex[1]->point;
  verts[2] = *P->vertex[2]->point;
  if (P->nrvertices > 3)
    verts[3] = *P->vertex[3]->point;

  drawSetColor(PatchPixel(P));
  drawPolygon(P->nrvertices, verts);
}

void SoftRenderPatches(DRAW_PIXEL (*patch_pixel)(PATCH *))
{
  PatchPixel = patch_pixel;

  if (renderopts.frustum_culling) {
    int use_display_lists = renderopts.use_display_lists;
    renderopts.use_display_lists = FALSE;  
    RenderWorldOctree(SoftRenderPatch);
    renderopts.use_display_lists = use_display_lists;    
  } else {
    ForAllPatches(P, Patches) {
      SoftRenderPatch(P);
    } EndForAll;
  }
}

static DRAW_PIXEL PatchID(PATCH *P)
{
  return (DRAW_PIXEL)P->id;
}

static void SoftRenderPatchIds(void)
{
  SoftRenderPatches(PatchID);
}

unsigned long *SoftRenderIds(long *x, long *y)
{
  DRAW_CONTEXT *draw, *olddraw;
  unsigned long *ids;

  if (sizeof(DRAW_PIXEL)!=sizeof(long)) {
    Fatal(-1, "SoftRenderIds", "sizeof(DRAW_PIXEL)!=sizeof(long). Disable SOFT_ID_RENDERING");
  }

  olddraw = drawGetCurrent();
  draw = SetupSoftFrameBuffer();
  SoftRenderPatchIds();

  *x = draw->width; *y = draw->height;
  ids = (unsigned long *)Alloc((int)(*x) * (int)(*y) * sizeof(unsigned long));
  memcpy(ids, draw->fbuf, draw->width*draw->height*sizeof(long));

  drawClose(draw);
  drawMakeCurrent(olddraw);

  return ids;
}
