/* patchlist.h: linear list of PATCHes */

#ifndef _PATCHLIST_H_
#define _PATCHLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "patch_type.h"
#include "List.h"

/* same layout as struct LIST in generics/List.h in order to be able to use the
 * generic List routines for lists of PATCHes. */
typedef struct PATCHLIST {
	struct PATCH *patch;
	struct PATCHLIST *next;
} PATCHLIST;

#define PatchListCreate	(PATCHLIST *)ListCreate

#define PatchListAdd(patchlist, patch)	\
        (PATCHLIST *)ListAdd((LIST *)patchlist, (void *)patch)

#define PatchListCount(patchlist) \
        ListCount((LIST *)patchlist)

#define PatchListGet(patchlist, index) \
        (PATCH *)ListGet((LIST *)patchlist, index)

#define PatchListDuplicate(patchlist) \
        (PATCHLIST *)ListDuplicate((LIST *)patchlist)

#define PatchListNext(ppatchlist) \
        (PATCH *)ListNext((LIST **)ppatchlist)

#define PatchListMerge(patchlist1, patchlist2) \
        (PATCHLIST *)ListMerge((LIST *)patchlist1, (LIST *)patchlist2);

#define PatchListRemove(patchlist, patch) \
        (PATCHLIST *)ListRemove((LIST *)patchlist, (void *)patch)

#define PatchListIterate(patchlist, proc) \
        ListIterate((LIST *)patchlist, (void (*)(void *))proc)

#define PatchListIterate1A(patchlist, proc, data) \
        ListIterate1A((LIST *)patchlist, (void (*)(void *, void *))proc, (void *)data)

#define PatchListIterate1B(patchlist, proc, data) \
        ListIterate1B((LIST *)patchlist, (void (*)(void *, void *))proc, (void *)data)

#define PatchListDestroy(patchlist) \
        ListDestroy((LIST *)patchlist)

#define PatchListPrint(fp, patchlist) \
        PatchListIterate1B(patchlist, PatchPrint, fp);

#define ForAllPatches(P, patches) ForAllInList(PATCH, P, patches)

#include "ray.h"

/* Computes a bounding box for the given list of PATCHes. The bounding box is
 * filled in in 'boundingbox' and a pointer to it returned. */
extern float *PatchListBounds(PATCHLIST *pl, float *boundingbox);

/* Tests whether the RAY intersect the PATCHes in the list. See geom.h 
 * (GeomDiscretisationIntersect()) for more explanation. */
extern struct HITREC *PatchListIntersect(PATCHLIST *pl, RAY *ray, float mindist, float *maxdist, int hitflags, HITREC *hitstore);

/* similar, but adds all found intersections to the hitlist */
extern struct HITLIST *PatchListAllIntersections(struct HITLIST *hits, PATCHLIST *patches, RAY *ray, float mindist, float maxdist, int hitflags);


#ifdef __cplusplus
}
#endif

#endif /* _PATCHLIST_H_ */
