/* geomlist.h: linear lists of GEOMetries */

#ifndef _GEOMLIST_H_
#define _GEOMLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "geom_type.h"
#include "List.h"

/* same layout as LIST in generics/List.h, so the same generic functions for
 * operating on lists can be used for lists of geometries */
typedef struct GEOMLIST {
	struct GEOM *geom;
	struct GEOMLIST *next;
} GEOMLIST;

#define GeomListCreate	(GEOMLIST *)ListCreate

#define GeomListAdd(geomlist, geom)	\
        (GEOMLIST *)ListAdd((LIST *)geomlist, (void *)geom)

#define GeomListCount(geomlist) \
        ListCount((LIST *)geomlist)

#define GeomListGet(geomlist, index) \
        (GEOM *)ListGet((LIST *)geomlist, index)

#define GeomListDuplicate(geomlist) \
        (GEOMLIST *)ListDuplicate((LIST *)geomlist)

#define GeomListNext(pgeomlist) \
        (GEOM *)ListNext((LIST **)pgeomlist)

#define GeomListMerge(geomlist1, geomlist2) \
        (GEOMLIST *)ListMerge((LIST *)geomlist1, (LIST *)geomlist2);

#define GeomListRemove(geomlist, geom) \
        (GEOMLIST *)ListRemove((LIST *)geomlist, (void *)geom)

#define GeomListIterate(geomlist, proc) \
        ListIterate((LIST *)geomlist, (void (*)(void *))proc)

#define GeomListIterate1A(geomlist, proc, data) \
        ListIterate1A((LIST *)geomlist, (void (*)(void *, void *))proc, (void *)data)

#define GeomListIterate1B(geomlist, proc, data) \
        ListIterate1B((LIST *)geomlist, (void (*)(void *, void *))proc, (void *)data)

#define GeomListDestroy(geomlist) \
        ListDestroy((LIST *)geomlist)

#define ForAllGeoms(geom, geomlist)	ForAllInList(GEOM, geom, geomlist)

/* this function computes a bounding box for a list of geometries. The bounding box is
 * filled in in 'boundingbox' and a pointer returned. */
extern float *GeomListBounds(GEOMLIST *geomlist, float *boundingbox);

/* Builds a linear list of PATCHES making up all the GEOMetries in the list, whether
 * they are primitive or not. */
extern struct PATCHLIST *BuildPatchList(GEOMLIST *world, struct PATCHLIST *patchlist);

/* Tests if the RAY intersects the discretisation of the GEOMetries in the list. 
 * See geom.h (GeomDiscretisationIntersect()) */
extern struct HITREC *GeomListDiscretisationIntersect(GEOMLIST *geomlist, RAY *ray, float mindist, float *maxdist, int hitflags, HITREC *hitstore);

extern struct HITLIST *GeomListAllDiscretisationIntersections(struct HITLIST *hits, GEOMLIST *geomlist, RAY *ray, float mindist, float maxdist, int hitflags);

#ifdef __cplusplus
}
#endif

#endif /* _GEOMLIST_H_ */

