/* namedvertex.h: routine for dealing with named vertices (needed for linking
 * global vertices when reading MGF files */

#ifndef _NAMEDVERTEX_H_
#define _NAMEDVERTEX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vertex_type.h"

/* used to keep a list of vertices with equal name */
typedef struct NAMEDVERTEXLIST {
  char *name;
  VERTEXLIST *vertices;
} NAMEDVERTEXLIST;

/* same layout as struct BINTREE in Bintree.h */
typedef struct NAMEDVERTEXTREE {
  NAMEDVERTEXLIST *namedvertices;
  struct NAMEDVERTEXTREE *left, *right;
} NAMEDVERTEXTREE;

#define NamedVertexTreeCreate()	(NAMEDVERTEXTREE *)NULL

extern void NamedVertexTreeDestroy(NAMEDVERTEXTREE *namedvertices);

extern VERTEX *NamedVertexFind(NAMEDVERTEXTREE *namedvertices, char *name, POINT *point, VECTOR *normal);

/* the vertex is not duplicated when installed in the tree */
extern NAMEDVERTEXTREE *NamedVertexAdd(NAMEDVERTEXTREE *namedvertices, char *name, VERTEX *vertex);

#ifdef __cplusplus
}
#endif

#endif /*_NAMEDVERTEX_H_*/
