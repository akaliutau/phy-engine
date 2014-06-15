/* vectorlist.h: linear lists of VECTORs */

#ifndef _VECTORLIST_H_
#define _VECTORLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vector.h"
#include "List.h"

/* same layout as LIST in generics/List.h in order to be able to use
 * the generic list procedures defined in generics/List.c */
typedef struct VECTORLIST {
	struct VECTOR *vector;
	struct VECTORLIST *next;
} VECTORLIST;

#define VectorListCreate	(VECTORLIST *)ListCreate

#define VectorListAdd(vectorlist, vector)	\
        (VECTORLIST *)ListAdd((LIST *)vectorlist, (void *)vector)

#define VectorListCount(vectorlist) \
        ListCount((LIST *)vectorlist)

#define VectorListGet(vectorlist, index) \
        (VECTOR *)ListGet((LIST *)vectorlist, index)

#define VectorListNext(pvectorlist) \
        (VECTOR *)ListNext((LIST **)pvectorlist)

#define VectorListRemove(vectorlist, vector) \
        (VECTORLIST *)ListRemove((LIST *)vectorlist, (void *)vector)

#define VectorListIterate(vectorlist, proc) \
        ListIterate((LIST *)vectorlist, (void (*)(void *))proc)

#define VectorListDestroy(vectorlist) \
        ListDestroy((LIST *)vectorlist)

#define ForAllVectors(v, veclist) ForAllInList(VECTOR, v, veclist)

/* This routine looks up a vector in a list of vectors. If a vector
 * being equal within EPSILON bounds to the given vector is found in the list,
 * a popointer to the vector in the list is returned. If no such vector is
 * in the list, (VECTOR *)NULL is returned. */
extern struct VECTOR *VectorListFind(struct VECTORLIST *vl, struct VECTOR *vector);

#ifdef __cplusplus
}
#endif

#endif /* _VECTORLIST_H_ */
