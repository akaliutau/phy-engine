
#include <stdio.h>
#include <stdlib.h>
#include "geom.h"
#include "bounds.h"
#include "error.h"
#include "extmath.h"
#include "pools.h"
#include "radiance.h"

#ifdef NOPOOLS
#define NEWGEOM()	(GEOM *)Alloc(sizeof(GEOM))
#define DISPOSEGEOM(ptr) Free((char *)ptr, sizeof(GEOM))
#else
static POOL *geomPool = (POOL *)NULL;
#define NEWGEOM()	(GEOM *)NewPoolCell(sizeof(GEOM), 0, "geoms", &geomPool)
#define DISPOSEGEOM(ptr) Dispose((char *)ptr, &geomPool)
#endif


int nrgeoms = 0, id = 0;

static void BoundsEnlargeTinyBit(float *bounds)
{
  float Dx = (bounds[MAX_X] - bounds[MIN_X]) * 1e-4;
  float Dy = (bounds[MAX_Y] - bounds[MIN_Y]) * 1e-4;
  float Dz = (bounds[MAX_Z] - bounds[MIN_Z]) * 1e-4;
  if (Dx < EPSILON) Dx = EPSILON;
  if (Dy < EPSILON) Dy = EPSILON;
  if (Dz < EPSILON) Dz = EPSILON;
  bounds[MIN_X] -= Dx; bounds[MAX_X] += Dx;
  bounds[MIN_Y] -= Dy; bounds[MAX_Y] += Dy;
  bounds[MIN_Z] -= Dz; bounds[MAX_Z] += Dz;
}


GEOM *GeomCreate(void *obj, GEOM_METHODS *methods)
{
  GEOM *p = (GEOM *)NULL;

  if (obj == NULL) return (GEOM *)NULL;

  p = NEWGEOM(); nrgeoms++;
  p->id = id++;
  p->obj = obj;
  p->methods = methods;

  if (methods->bounds) {
    methods->bounds(obj, p->bounds);
    
    BoundsEnlargeTinyBit(p->bounds);
    p->bounded = TRUE;
  } else {
    BoundsInit(p->bounds);
    p->bounded = FALSE;
  }
  p->shaftcullgeom = FALSE;

  p->radiance_data = (void *)NULL;
  p->tmp.i = 0;
  p->omit = FALSE;

  p->dlistid = -1;

  return p;
}


void GeomPrint(FILE *out, GEOM *geom)
{
  fprintf(out, "Geom %d, bounded = %s, shaftcullgeom = %s:\n",
	  geom->id, 
	  geom->bounded ? "TRUE" : "FALSE",
	  geom->shaftcullgeom ? "TRUE" : "FALSE");

  geom->methods->print(out, geom->obj);
}


float *GeomBounds(GEOM *geom)
{
  return geom->bounds;
}


void GeomDestroy(GEOM *geom)
{
  geom->methods->destroy(geom->obj);
  DISPOSEGEOM(geom); nrgeoms--;
}


int GeomIsAggregate(GEOM *geom)
{
  return geom->methods->primlist != (GEOMLIST *(*)(void *))NULL;
}


GEOMLIST *GeomPrimList(GEOM *geom)
{
  if (geom->methods->primlist)
    return geom->methods->primlist(geom->obj);
  else
    return (GEOMLIST *)NULL;
}


PATCHLIST *GeomPatchList(GEOM *geom)
{
  if (geom->methods->patchlist)
    return geom->methods->patchlist(geom->obj);
  else
    return (PATCHLIST *)NULL;
}


GEOM *GeomDuplicate(GEOM *geom)
{
  GEOM *p = (GEOM *)NULL;

  if (!geom->methods->duplicate) {
    Error("GeomDuplicate", "geometry has no duplicate method");
    return (GEOM *)NULL;
  }

  p = NEWGEOM(); nrgeoms++;
  *p = *geom;
  p->obj = geom->methods->duplicate(geom->obj);

  return p;
}


GEOM *excludedGeom1=(GEOM *)NULL, *excludedGeom2=(GEOM *)NULL;
void GeomDontIntersect(GEOM *geom1, GEOM *geom2)
{
  excludedGeom1 = geom1;
  excludedGeom2 = geom2;
}

#ifdef IDEBUG
extern int idebug;
#endif


HITREC *GeomDiscretisationIntersect(GEOM *geom, RAY *ray, 
				   float mindist, float *maxdist,
				   int hitflags, HITREC *hitstore)
{
  VECTOR vtmp;
  float nmaxdist;

#ifdef IDEBUG
  if (idebug) {
    fprintf(stderr, "====> %s %d: GeomDiscretisationIntersect\n", __FILE__, __LINE__);
  }
#endif

  if (geom==excludedGeom1 || geom==excludedGeom2) {
#ifdef IDEBUG
    if (idebug) {
      fprintf(stderr, "%s %d: excluded geometry -> no intersection\n", __FILE__, __LINE__);
    }
#endif
    return (HITREC *)NULL;
  }

  if (geom->bounded) {
    
    VECTORADDSCALED(ray->pos, mindist, ray->dir, vtmp);
    if (OutOfBounds(&vtmp, geom->bounds)) {
      nmaxdist = *maxdist;
      if (!BoundsIntersect(ray, geom->bounds, mindist, &nmaxdist)) {
#ifdef IDEBUG
	if (idebug) {
	  fprintf(stderr, "%s %d: bounding box test fails -> no intersection\n", __FILE__, __LINE__);
	}
#endif
	return NULL;
      }
    }
  }
#ifdef IDEBUG
  if (idebug) {
    fprintf(stderr, "%s %d: bounding box test succeeded, now calling discretisatoinintersect method ...\n", __FILE__, __LINE__);
  }
#endif
  return geom->methods->discretisation_intersect(geom->obj, ray, mindist, maxdist, hitflags, hitstore);
}

HITLIST *GeomAllDiscretisationIntersections(HITLIST *hits, 
					    GEOM *geom, RAY *ray, 
					    float mindist, float maxdist,
					    int hitflags)
{
  VECTOR vtmp;
  float nmaxdist;

  if (geom==excludedGeom1 || geom==excludedGeom2) {
    return hits;
  }

  if (geom->bounded) {
    
    VECTORADDSCALED(ray->pos, mindist, ray->dir, vtmp);
    if (OutOfBounds(&vtmp, geom->bounds)) {
      nmaxdist = maxdist;
      if (!BoundsIntersect(ray, geom->bounds, mindist, &nmaxdist)) {
	return hits;
      }
    }
  }
  return geom->methods->all_discretisation_intersections(hits, geom->obj, ray, mindist, maxdist, hitflags);
}

