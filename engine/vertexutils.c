

#include "vertexutils.h"
#include "render.h"
#include "camera.h"
#include "scene.h"
#include "canvas.h"
#include "patch.h"
#include "pools.h"
#include "error.h"

static void BuildIdToPatchTable(PATCH *patch, PATCH **id2patch)
{
  id2patch[patch->id] = patch;
}


static void PatchReverseVertexOrder(PATCH *patch)
{
  VERTEX *vtmp = patch->vertex[2];
  patch->vertex[2] = patch->vertex[0];
  patch->vertex[0] = vtmp;

  VECTORSCALE(-1, patch->normal, patch->normal);
  patch->plane_constant = -patch->plane_constant;
}


void FixVertexOrderInView(void)
{
  PATCH **id2patch, *lastpatch;
  unsigned long *ids, *id;
  long i, x, y, nrpixels, maxpatchid, lostpixels;
  int backface_culling;

  if (!Patches)
    return;

  
  backface_culling = renderopts.backface_culling;
  renderopts.backface_culling = FALSE;

  CanvasPushMode(CANVASMODE_RENDER);

  
  ids = RenderIds(&x, &y);

  CanvasPullMode();
  CanvasPushMode(CANVASMODE_WORKING);

  
  renderopts.backface_culling = backface_culling;

  if (!ids) return;
  nrpixels = x*y; lostpixels=0;

  
  maxpatchid = PatchGetNextID()-1;  
  id2patch = (PATCH **)Alloc((int)(maxpatchid+1) * sizeof(PATCH *));
  for (i=0; i<=maxpatchid; i++) id2patch[i] = (PATCH *)NULL;
  PatchListIterate1A(Patches, BuildIdToPatchTable, (void *)id2patch);

  lastpatch = (PATCH *)NULL;
  for (i=0, id=ids; i < nrpixels; i++, id++) {
    unsigned long the_id = (*id)&0xffffff;
    PATCH *patch = (the_id <= maxpatchid) ? id2patch[the_id] : (PATCH *)NULL;

    if (patch && patch != lastpatch)
      
      if (VECTORDOTPRODUCT(patch->normal, Camera.eyep) + patch->plane_constant < EPSILON)
	PatchReverseVertexOrder(patch);

    lastpatch = patch;
  }

  if (lostpixels > 0)
    Warning(NULL, "%d lost pixels", lostpixels);

  Free((char *)id2patch, (int)(maxpatchid+1) * sizeof(PATCH *));
  Free((char *)ids, (int)nrpixels * sizeof(unsigned long));

  CanvasPullMode();
}


