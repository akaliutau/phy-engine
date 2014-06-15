

#include "topology.h"


void BBoxCheckIn(PATCHLIST *patches)
{
  BBOX_FACE *face;
  BBOX_CONTOUR *contour;
  int i;

  for (; patches; patches = patches->next) {
    PATCH *patch = patches->patch;

    if (patch->brep_data)
      continue;

    
    for (i=0; i<patch->nrvertices; i++) 
      if (!patch->vertex[i]->brep_data)
	patch->vertex[i]->brep_data = BBoxCreateVertex(patch->vertex[i]);

    
    patch->brep_data = face = BBoxCreateFace((BBOX_SHELL *)NULL, patch);
    
    
    contour = BBoxCreateContour(face, NULL);

    
    for (i=0; i<patch->nrvertices; i++) 
      BBoxCreateWing(patch->vertex[i]->brep_data, 
		     patch->vertex[(i+1)%patch->nrvertices]->brep_data,
		     contour, NULL);

    
    BBoxCloseFace(face);
  }
}


void BBoxCheckOut(PATCHLIST *patches)
{
  int i;

  for (; patches; patches = patches->next) {
    PATCH *patch = patches->patch;

    if (patch->brep_data) {
      BBoxDestroyFace(patch->brep_data);
      patch->brep_data = (BBOX_FACE *)NULL;
    }

    for (i=0; i<patch->nrvertices; i++) {
      if (patch->vertex[i]->brep_data &&
	  !patch->vertex[i]->brep_data->wing_ring) {
	
	BBoxDestroyVertex(patch->vertex[i]->brep_data);
	patch->vertex[i]->brep_data = (BBOX_VERTEX *)NULL;
      }
    }
  }
}
