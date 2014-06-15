

#include <stdio.h>
#include <stdlib.h>
#include "compound.h"
#include "error.h"
#include "pools.h"
#include "geom.h"

int nrcompounds = 0;


COMPOUND *CompoundCreate(GEOMLIST *geomlist)
{
  nrcompounds++;
  return geomlist;
}


static float *CompoundBounds(COMPOUND *obj, float *boundingbox)
{
  return GeomListBounds(obj, boundingbox);
}


static void   CompoundDestroy(COMPOUND *obj)
{
  GeomListIterate(obj, GeomDestroy);
  GeomListDestroy(obj);
  nrcompounds--;
}


void   CompoundPrint(FILE *out, COMPOUND *obj)
{
  fprintf(out, "compound\n");
  GeomListIterate1B(obj, GeomPrint, out);  
  fprintf(out, "end of compound\n");
}


GEOMLIST *CompoundPrimitives(COMPOUND *obj)
{
  return obj;
}

struct HITREC *CompoundDiscretisationIntersect(COMPOUND *obj, RAY *ray, float mindist, float *maxdist, int hitflags, HITREC *hitstore)
{
  return GeomListDiscretisationIntersect(obj, ray, mindist, maxdist, hitflags, hitstore);
}

struct HITLIST *CompoundAllDiscretisationIntersections(HITLIST *hits, COMPOUND *obj, RAY *ray, float mindist, float maxdist, int hitflags)
{
  return GeomListAllDiscretisationIntersections(hits, obj, ray, mindist, maxdist, hitflags);
}


GEOM_METHODS compoundMethods = {
  (float *(*)(void *, float *))CompoundBounds,
  (void (*)(void *))CompoundDestroy,
  (void (*)(FILE *, void *))CompoundPrint,
  (GEOMLIST *(*)(void *))CompoundPrimitives,
  (PATCHLIST *(*)(void *))NULL,
  (HITREC *(*)(void *, RAY *, float, float *, int, HITREC *))CompoundDiscretisationIntersect,
  (HITLIST *(*)(HITLIST *, void *, RAY *, float, float, int))CompoundAllDiscretisationIntersections,
  (void *(*)(void *))NULL
};


