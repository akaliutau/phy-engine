

#include "vertex.h"
#include "patch.h"
#include "pools.h"

#define NEWVERTEX()	(VERTEX *)Alloc(sizeof(VERTEX))
#define DISPOSEVERTEX(ptr) Free((char *)ptr, sizeof(VERTEX))

int nrvertices = 0;


VERTEX *VertexCreate(POINT *point, VECTOR *normal, VECTOR *texCoord, PATCHLIST *patches)
{
  VERTEX *v;

  v = NEWVERTEX(); v->id = nrvertices++;
  v->point = point;
  v->normal = normal;
  v->texCoord = texCoord;
  v->patches = patches;
  RGBSET(v->color, 0., 0., 0.);
  v->brep_data = (BBOX_VERTEX *)NULL;
  v->radiance_data = (void *)NULL;
  v->back = (VERTEX *)NULL;

  return v;
}


void VertexDestroy(VERTEX *vertex)
{
  PatchListDestroy(vertex->patches);
  if (vertex->brep_data)
    BBoxDestroyVertex(vertex->brep_data);
  DISPOSEVERTEX(vertex); nrvertices--;
}


void VertexPrint(FILE *out, VERTEX *vertex)
{
  fprintf(out, "ID %d: (", vertex->id); 
  VectorPrint(out, *(vertex->point)); 
  fprintf(out, ")");
  if (vertex->normal) {
    fprintf(out, "/("); VectorPrint(out, *(vertex->normal)); fprintf(out, ")");
  } else
    fprintf(out, "/(no normal)");
  if (vertex->texCoord) {
    fprintf(out, "/("); VectorPrint(out, *(vertex->texCoord)); fprintf(out, ")");
  } else
    fprintf(out, "/(no texCoord)");
  fprintf(out, " color = ("); RGBPrint(out, vertex->color); fprintf(out, ")," );

  fprintf(out, "patches: ");
  ForAllPatches(P, vertex->patches) {
    PatchPrintID(out, P);
  } EndForAll;

  fprintf(out, "\n");
}


void ComputeVertexColor(VERTEX *vertex)
{
  PATCHLIST *patches;
  int nrpatches;

  RGBSET(vertex->color, 0., 0., 0.); nrpatches=0;
  for (patches=vertex->patches; patches; patches=patches->next) {
    vertex->color.r += patches->patch->color.r;
    vertex->color.g += patches->patch->color.g;
    vertex->color.b += patches->patch->color.b;
    nrpatches++;
  }
  if (nrpatches > 0) {
    vertex->color.r /= (float)nrpatches;
    vertex->color.g /= (float)nrpatches;
    vertex->color.b /= (float)nrpatches;
  }
}


void PatchComputeVertexColors(PATCH *patch)
{
  int i;

  for (i=0; i<patch->nrvertices; i++)
     ComputeVertexColor(patch->vertex[i]);
}

static unsigned int vertex_compare_flags = VCMP_LOCATION | VCMP_NORMAL | VCMP_TEXCOORD;

unsigned VertexSetCompareFlags(unsigned flags)
{
  unsigned oldflags = vertex_compare_flags;
  vertex_compare_flags = flags;
  return oldflags;
}


int VertexCompareLocation(VERTEX *v1, VERTEX *v2)
{
  
  float tolerance = VECTORTOLERANCE(*v1->point) + VECTORTOLERANCE(*v2->point);
  return VectorCompare(v1->point, v2->point, tolerance);
}


int VertexCompareNormal(VERTEX *v1, VERTEX *v2)
{
  int code = VectorCompare(v1->normal, v2->normal, EPSILON);
  if (code == XYZ_EQUAL && !(vertex_compare_flags & VCMP_NO_NORMAL_IS_EQUAL_NORMAL))
    if (v1->normal->x == 0. && v1->normal->y == 0. && v1->normal->z == 0.)
      return 0;				
  return code;
}


int VertexCompareTexCoord(VERTEX *v1, VERTEX *v2)
{
  if (!v1->texCoord) {
    if (!v2->texCoord) 
      return XYZ_EQUAL;
    else
      return 0;		
  } else {
    if (!v2->texCoord)
      return X_GREATER+Y_GREATER+Z_GREATER;		
    else
      return VectorCompare(v1->texCoord, v2->texCoord, EPSILON);
  }
}


int VertexCompare(VERTEX *v1, VERTEX *v2)
{
  int code = XYZ_EQUAL;

  
  if (vertex_compare_flags & VCMP_LOCATION) {
    code = VertexCompareLocation(v1, v2);
    if (code != XYZ_EQUAL)
      return code;
  }

  
  if (vertex_compare_flags & VCMP_NORMAL) {
    code = VertexCompareNormal(v1, v2);
    if (code != XYZ_EQUAL)
      return code;
  }

  
  if (vertex_compare_flags & VCMP_TEXCOORD) {
    code = VertexCompareTexCoord(v1, v2);
    if (code != XYZ_EQUAL)
      return code;
  }

  return XYZ_EQUAL;
}

