/* vectoroctree.h: octrees containing vectors */

#ifndef _VECTOROCTREE_H_
#define _VECTOROCTREE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vector.h"
#include "Octree.h"

/* same layout as struct OCTREE in generics/Octree.h. */
typedef struct VECTOROCTREE {
	struct VECTOR *vector;
	struct VECTOROCTREE *child[8];
} VECTOROCTREE;

#define VectorOctreeCreate()	((VECTOROCTREE *)OctreeCreate())

#define VectorOctreeAdd(poctree, pvector)     \
        (VECTOROCTREE *)OctreeAdd((OCTREE *)poctree, (void *)pvector, (int (*)(void *, void *))VectorCompare)

#define VectorOctreeFind(poctree, pvector)     \
        (VECTOR *)OctreeFind((OCTREE *)poctree, (void *)pvector, (int (*)(void *, void *))VectorCompare)

#define VectorOctreeDestroy(poctree) \
        OctreeDestroy((OCTREE *)poctree)

#ifdef __cplusplus
}
#endif

#endif /*_VECTOROCTREE_H_*/
