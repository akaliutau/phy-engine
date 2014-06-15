/* shadowcaching.h: shadow caching routines. */

#ifndef _SHADOWCACHING_H_
#define _SHADOWCACHING_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "patchlist.h"
#include "geomlist.h"
#include "ray.h"

/* initialize/empty the shadow cache */
extern void InitShadowCache(void);

/* test ray against patches in the shadow cache. Returns NULL if the ray hits
 * no patches in the shadow cache, or a pointer to the first hit patch 
 * otherwise.  */
extern HITREC *CacheHit(RAY *ray, float *dist, HITREC *hitstore);

/* replace least recently added patch */
extern void AddToShadowCache(PATCH *patch);

/* Tests whether the ray intersects the discretisation of a GEOMetry in the list 
 * 'world'. Returns NULL if the ray hits no geometries. Returns an arbitrary hit 
 * patch if the ray does intersect one or more geometries. Intersections
 * further away than dist are ignored. Patches in the shadow cache are
 * checked first. */
extern HITREC *ShadowTestDiscretisation(RAY *ray, GEOMLIST *world, float dist, HITREC *hitstore);

/* like ShadowTestDiscretisation, except that a list of PATCHes is tested
 * for intersection. */
extern HITREC *ShadowTestPatchlist(RAY *ray, PATCHLIST *ShadowList, float dist, HITREC *hitstore);

#ifdef __cplusplus
}
#endif

#endif /*_SHDOWCACHING_H_*/
