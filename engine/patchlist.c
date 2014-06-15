

#include "patchlist.h"
#include "patchlist_geom.h"
#include "bounds.h"
#include "patch.h"
#include "hitlist.h"


float *PatchListBounds(PATCHLIST *pl, float *boundingbox)
{
  BOUNDINGBOX b;

  BoundsInit(boundingbox);
  ForAllPatches(patch, pl) {
    PatchBounds(patch, b);
    BoundsEnlarge(boundingbox, b);
  } EndForAll;

  return boundingbox;
}


HITREC *PatchListIntersect(PATCHLIST *pl, RAY *ray, float mindist, float *maxdist, int hitflags, HITREC *hitstore)
{
  HITREC *hit = (HITREC *)NULL;
  ForAllPatches(P, pl) {
    HITREC *h = PatchIntersect(P, ray, mindist, maxdist, hitflags, hitstore);
    if (h) {
      if (hitflags & HIT_ANY)
	return h;
      else
	hit = h;
    }
  } EndForAll;
  return hit;
}

HITLIST *PatchListAllIntersections(HITLIST *hits, PATCHLIST *patches, RAY *ray, float mindist, float maxdist, int hitflags)
{
  HITREC hitstore;
  ForAllPatches(P, patches) {
    float tmax = maxdist;	
    HITREC *hit = PatchIntersect(P, ray, mindist, &tmax, hitflags, &hitstore);
    if (hit) {
      hits = HitListAdd(hits, DuplicateHit(hit));
    }
  } EndForAll;
  return hits;
}


static void PatchlistDestroy(PATCHLIST *patchlist)
{
  PatchListDestroy(patchlist);
}

static void PatchPrintId(FILE *out, PATCH *patch)
{
  fprintf(out, "%d ", patch->id);
}

static void PatchlistPrint(FILE *out, PATCHLIST *patchlist)
{
  fprintf(out, "patchlist:\n");
  PatchListIterate1B(patchlist, PatchPrintId, out);
  fprintf(out, "\nend of patchlist.\n");
}

static PATCHLIST *PatchlistPatchlist(PATCHLIST *patchlist)
{
  return patchlist;
}

static PATCHLIST *PatchlistDuplicate(PATCHLIST *patchlist)
{
  return PatchListDuplicate(patchlist);
}

GEOM_METHODS patchlistMethods = {
  (float *(*)(void *, float *))PatchListBounds,
  (void (*)(void *))PatchlistDestroy,
  (void (*)(FILE *, void *))PatchlistPrint,
  (GEOMLIST *(*)(void *))NULL,
  (PATCHLIST *(*)(void *))PatchlistPatchlist,
  (HITREC *(*)(void *, RAY *, float, float *, int, HITREC *))PatchListIntersect,
  (HITLIST *(*)(HITLIST *, void *, RAY *, float, float, int))PatchListAllIntersections,
  (void *(*)(void *))PatchlistDuplicate
};
