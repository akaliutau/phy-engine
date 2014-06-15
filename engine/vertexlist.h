/* vertexlist.h: linear lists of VERTEX structures */

#ifndef _VERTEXLIST_H_
#define _VERTEXLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vertex_type.h"
#include "List.h"

/* same layout as LIST in generics/List.h */
typedef struct VERTEXLIST {
	struct VERTEX *vertex;
	struct VERTEXLIST *next;
} VERTEXLIST;

#define VertexListCreate	(VERTEXLIST *)ListCreate

#define VertexListAdd(vertexlist, vertex)	\
        (VERTEXLIST *)ListAdd((LIST *)vertexlist, (void *)vertex)

#define VertexListCount(vertexlist) \
        ListCount((LIST *)vertexlist)

#define VertexListGet(vertexlist, index) \
        (VERTEX *)ListGet((LIST *)vertexlist, index)

#define VertexListNext(pvertexlist) \
        (VERTEX *)ListNext((LIST **)pvertexlist)

#define VertexListRemove(vertexlist, vertex) \
        (VERTEXLIST *)ListRemove((LIST *)vertexlist, (void *)vertex)

#define VertexListIterate(vertexlist, proc) \
        ListIterate((LIST *)vertexlist, (void (*)(void *))proc)

#define VertexListDestroy(vertexlist) \
        ListDestroy((LIST *)vertexlist)

#define ForAllVertices(v, vertexlist) ForAllInList(VERTEX, v, vertexlist)

/* Looks up a vertex with given coordinates, normal and texture coords in the given
 * vertex list. Returns a pointer to the VERTEX if found, or (VERTEX *)NULL if no 
 * such vertex is found. Vertices are compared using VertexCompare() declared in
 * vertex.h. */
extern struct VERTEX *VertexListFind(VERTEXLIST *vl, VECTOR *p, VECTOR *norm, VECTOR *texCoord);

#ifdef __cplusplus
}
#endif

#endif /* _VERTEXLIST_H_ */
