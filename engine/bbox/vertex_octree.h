/* vertex_octree.h: Boundary Representation vertices organised in an
 *		    octree. For fast vertex lookup. */

#ifndef _BBOX_VERTEX_OCTREE_H_
#define _BBOX_VERTEX_OCTREE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bbox.h"
#include "pools.h"
#include "Octree.h"

/* struct BBOX_VERTEX_OCTREE is defined in bbox.h and has same layout as
 * struct OCTREE in Octree.h */

#define BBoxVertexOctreeCreate()	((BBOX_VERTEX_OCTREE *)OctreeCreate())

#define BBoxVertexOctreeAdd(poctree, pbrep_vertex)     \
        (BBOX_VERTEX_OCTREE *)OctreeAddWithDuplicates((OCTREE *)poctree, (void *)pbrep_vertex, (int (*)(void *, void *))BBoxVertexCompare)

#define BBoxVertexOctreeFind(poctree, pbrep_vertex)     \
        (BBOX_VERTEX *)OctreeFind((OCTREE *)poctree, (void *)pbrep_vertex, (int (*)(void *, void *))BBoxVertexCompare)

#define BBoxVertexOctreeDestroy(poctree) \
        OctreeDestroy((OCTREE *)poctree)

#define BBoxVertexOctreeIterate(poctree, func) \
        OctreeIterate((OCTREE *)poctree, (void (*)(void *))func)

/* compares two vertices. Calls a user installed routine to compare the
 * client data of two vertices. The routine to be used is specified with
 * BBoxSetVertexCompareRoutine() in bbox.h */
extern int BBoxVertexCompare(BBOX_VERTEX *v1, BBOX_VERTEX *v2);

#ifdef __cplusplus
}
#endif

#endif /* _BBOX_VERTEX_OCTREE_H_ */
