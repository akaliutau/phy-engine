

#include "geomlist.h"
#include "geom.h"
#include "hitlist.h"


float *GeomListBounds(GEOMLIST *geomlist, float *bounds)
{
  BoundsInit(bounds);
  ForAllGeoms(geom, geomlist) {
    BoundsEnlarge(bounds, GeomBounds(geom));
  } EndForAll;
  return bounds;
}


PATCHLIST *BuildPatchList(GEOMLIST *world, PATCHLIST *patchlist)
{
  ForAllGeoms(geom, world) {
    if (GeomIsAggregate(geom))
      patchlist = BuildPatchList(GeomPrimList(geom), patchlist);
    else
      patchlist = PatchListMerge(patchlist, GeomPatchList(geom));
  } EndForAll;

  return patchlist;
}

HITREC *GeomListDiscretisationIntersect(GEOMLIST *geomlist, RAY *ray, float mindist, float *maxdist, int hitflags, HITREC *hitstore)
{
  HITREC *h, *hit;

  hit = (HITREC *)NULL;
  ForAllGeoms(geom, geomlist) {
    if ((h=GeomDiscretisationIntersect(geom, ray, mindist, maxdist, hitflags, hitstore))) {
      if (hitflags & HIT_ANY)
	return h; 
      else 
	hit = h;
    }
  } EndForAll;
  return hit;
}

HITLIST *GeomListAllDiscretisationIntersections(HITLIST *hits, GEOMLIST *geomlist, RAY *ray, float mindist, float maxdist, int hitflags)
{
  ForAllGeoms(geom, geomlist) {
    hits = GeomAllDiscretisationIntersections(hits, geom, ray, mindist, maxdist, hitflags);
  } EndForAll;
  return hits;
}
