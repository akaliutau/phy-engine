

#include "hitlist.h"
#include "patch.h"
#include "pools.h" 

#ifndef NOPOOLS
static POOL *hitPool = (POOL *)NULL; 
#define NEWHIT()	(HITREC *)NewPoolCell(sizeof(HITREC), 0, "hitrecs", &hitPool)
#define DISPOSEHIT(ptr) Dispose((unsigned char *)ptr, &hitPool)
#else 
#include <stdlib.h>
#define NEWHIT()	(HITREC *)Alloc(sizeof(HITREC))
#define DISPOSEHIT(ptr) Free((char *)ptr, sizeof(HITREC))
#endif 


HITREC *DuplicateHit(HITREC *hit)
{
  HITREC *duplhit = NEWHIT();
  *duplhit = *hit;
  return duplhit;
}


void DestroyHitlist(HITLIST *hitlist)
{
  ForAllHits(hit, hitlist) {
    DISPOSEHIT(hit);
  } EndForAll;
  HitListDestroy(hitlist);
}


void PrintHit(FILE *out, HITREC *hit)
{
  fprintf(out, "dist=%g, patch %d, %s\n", hit->dist, hit->patch->id, hit->flags&HIT_BACK ? "back" : "front");
}

void PrintHits(FILE *out, HITLIST *hits)
{
  ForAllHits(hit, hits) {
    PrintHit(out, hit);
  } EndForAll;
}

