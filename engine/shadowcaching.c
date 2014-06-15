
#include "shadowcaching.h"
#include "patch.h"
#include "grid.h"
#include "scene.h"


int ShadowRays=0, ShadowCacheHits=0;

#define MAXCACHE 5	
static PATCH *cache[MAXCACHE];
static int cachedpatches, ncached;


void InitShadowCache(void)
{
  int i;

  ncached = cachedpatches = 0;
  for (i=0; i<MAXCACHE; i++) cache[i] = NULL;
}


HITREC *CacheHit(RAY *ray, float *dist, HITREC *hitstore)
{
  int i; 
  HITREC *hit;
  
  for (i=0; i<ncached; i++) {
    if ((hit = PatchIntersect(cache[i], ray, EPSILON*(*dist), dist, HIT_FRONT|HIT_ANY, hitstore)))
      return hit;
  }
  return NULL;
}
				

void AddToShadowCache(PATCH *patch)
{
  cache[cachedpatches % MAXCACHE] = patch;
  cachedpatches++;
  if (ncached<MAXCACHE) ncached++;
}


HITREC *ShadowTestDiscretisation(RAY *ray, GEOMLIST *world, float dist, HITREC *hitstore)
{
  HITREC *hit=NULL;

  ShadowRays++;
  if ((hit = CacheHit(ray, &dist, hitstore)))
    ShadowCacheHits++;
  else {
    if (world != ClusteredWorld && world != World) 
      hit = GeomListDiscretisationIntersect(world, ray, EPSILON*dist, &dist, HIT_FRONT|HIT_ANY, hitstore);
    else
      hit = GridIntersect(WorldGrid, ray, EPSILON*dist, &dist, HIT_FRONT|HIT_ANY, hitstore);
    if (hit)
      AddToShadowCache(hit->patch);
  }

  return hit;
}


HITREC *ShadowTestPatchlist(RAY *ray, PATCHLIST *ShadowList, float dist, HITREC *hitstore)
{
  HITREC *hit=NULL;

  ShadowRays++;
  if ((hit = CacheHit(ray, &dist, hitstore)))
    ShadowCacheHits++;
  else if ((hit = PatchListIntersect(ShadowList, ray, EPSILON*dist, &dist, HIT_FRONT|HIT_ANY, hitstore)))
    AddToShadowCache(hit->patch);
  
  return hit;
}

