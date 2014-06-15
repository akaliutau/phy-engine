

#include "patch.h"
#include "surface.h"
#include "vertex.h"
#include "ray.h"
#include "pools.h"
#include "error.h"

#ifdef NOPOOLS
#define NEWSURFACE()	(SURFACE *)Alloc(sizeof(SURFACE))
#define DISPOSESURFACE(ptr) Free((char *)ptr, sizeof(SURFACE))
#else
static POOL *surfacePool = (POOL *)NULL;
#define NEWSURFACE()	(SURFACE *)NewPoolCell(sizeof(SURFACE), 0, "surfaces", &surfacePool)
#define DISPOSESURFACE(ptr) Dispose((char *)ptr, &surfacePool)
#endif


static int surfID = 0;
int nrsurfaces = 0;


enum COLORFLAGS colorflags = NO_COLORS;

static void NormalizeVertexColor(VERTEX *vertex)
{
  int nrpatches = 0;
  PATCHLIST *patches;
  for (patches=vertex->patches; patches; patches=patches->next) 
    nrpatches++;
  if (nrpatches > 0) {
    vertex->color.r /= (float)nrpatches;
    vertex->color.g /= (float)nrpatches;
    vertex->color.b /= (float)nrpatches;
  }
}


void SurfaceConnectFace(SURFACE *surf, PATCH *face)
{
  int i;
  COLOR rho;

  face->surface = surf;

  
  switch (colorflags) {
  case FACE_COLORS:
    break;
  case VERTEX_COLORS:
    
    RGBSET(face->color, 0, 0, 0);
    for (i=0; i<face->nrvertices; i++) {
      face->color.r += face->vertex[i]->color.r;
      face->color.g += face->vertex[i]->color.g;
      face->color.b += face->vertex[i]->color.b;
    }
    face->color.r /= (float)i;
    face->color.g /= (float)i;
    face->color.b /= (float)i;
    break;
  default:
    {
      rho = PatchAverageNormalAlbedo(face, BRDF_DIFFUSE_COMPONENT|BRDF_GLOSSY_COMPONENT);
      ColorToRGB(rho, &face->color);
    }
  }
}


SURFACE *SurfaceCreate(MATERIAL *material, 
		       VECTORLIST *points, VECTORLIST *normals, VECTORLIST *texCoords,
		       VERTEXLIST *vertices, PATCHLIST *faces,
		       enum COLORFLAGS flags)
{
  SURFACE *surf;

  surf = NEWSURFACE(); nrsurfaces++;
  surf->id = surfID++;
  surf->material = material;
  surf->points = points;
  surf->normals = normals;
  surf->vertices = vertices;
  surf->texCoords = texCoords;
  surf->faces = faces;

  colorflags = flags;
 
  
  if (colorflags == VERTEX_COLORS)
    VertexListIterate(surf->vertices, NormalizeVertexColor);

  
  ForAllPatches(face, surf->faces) {
    SurfaceConnectFace(surf, face);
  } EndForAll;
  
  
  if (colorflags != VERTEX_COLORS)
    VertexListIterate(surf->vertices, ComputeVertexColor);

  colorflags = NO_COLORS;
  return surf;
}


void SurfaceDestroy(SURFACE *surf)
{
  
  PatchListIterate(surf->faces, PatchDestroy);
  PatchListDestroy(surf->faces);

  VertexListIterate(surf->vertices, VertexDestroy);
  VertexListDestroy(surf->vertices);

  VectorListIterate(surf->points, VectorDestroy);
  VectorListDestroy(surf->points);

  VectorListIterate(surf->normals, VectorDestroy);
  VectorListDestroy(surf->normals);

  VectorListIterate(surf->texCoords, VectorDestroy);
  VectorListDestroy(surf->texCoords);

  DISPOSESURFACE(surf); nrsurfaces--;
}


void SurfacePrint(FILE *out, SURFACE *surface)
{
  fprintf(out, "Surface id %d: material %s, patches ID: ",
	  surface->id, surface->material->name);
  PatchListIterate1B(surface->faces, PatchPrintID, out);
  fprintf(out, "\n");

  PatchListIterate1B(surface->faces, PatchPrint, out);
}


float *SurfaceBounds(SURFACE *surf, float *boundingbox)
{
  return PatchListBounds(surf->faces, boundingbox);
}


PATCHLIST *SurfacePatchlist(SURFACE *surf)
{
  return surf->faces;
}

HITREC *SurfaceDiscretisationIntersect(SURFACE *surf, RAY *ray, float mindist, float *maxdist, int hitflags, HITREC *hitstore)
{
  return PatchListIntersect(surf->faces, ray, mindist, maxdist, hitflags, hitstore);
}

HITLIST *SurfaceAllDiscretisationIntersections(HITLIST *hits, SURFACE *surf, RAY *ray, float mindist, float maxdist, int hitflags)
{
  return PatchListAllIntersections(hits, surf->faces, ray, mindist, maxdist, hitflags);
}

GEOM_METHODS surfaceMethods = {
  (float *(*)(void *, float *))SurfaceBounds,
  (void (*)(void *))SurfaceDestroy,
  (void (*)(FILE *, void *))SurfacePrint,
  (GEOMLIST *(*)(void *))NULL,
  (PATCHLIST *(*)(void *))SurfacePatchlist,
  (HITREC *(*)(void *, RAY *, float, float *, int, HITREC *))SurfaceDiscretisationIntersect,
  (HITLIST *(*)(HITLIST *, void *, RAY *, float, float, int))SurfaceAllDiscretisationIntersections,
  (void *(*)(void *))NULL
};





