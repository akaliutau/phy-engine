/* vertex.h */

#ifndef _PHY_VERTEX_H_
#define _PHY_VERTEX_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "vertex_type.h"

/* create a vertex with given coordinates, normal vector and list of patches
 * sharing the vertex. Several vertices can share the same coordinates
 * and normal vector. Several patches can share the same vertex. */
extern VERTEX *VertexCreate(VECTOR *point, VECTOR *normal, VECTOR *texCoord, struct PATCHLIST *patches);

/* destroys the vertex. Does not destroy the coordinate vector and
 * normal vector, neither the patches sharing the vertex. */
extern void VertexDestroy(VERTEX *vertex);

/* prints the vertex data to the file 'out' */
extern void VertexPrint(FILE *out, VERTEX *vertex);

/* averages the color of each patch sharing the vertex and assign the 
 * resulting color to the vertex. */
extern void ComputeVertexColor(VERTEX *vertex);

/* computes a vertex color for the vertices of the patch */
extern void PatchComputeVertexColors(PATCH *patch);

/*
 * Vertex comparison
 *
 * Vertices have a coordinate, normal and texture coordinate.
 * The following flags determine what is taken into account
 * when comparing vertices:
 * - VCMP_LOCATION: compare location
 * - VCMP_NORMAL: compare normal
 * - VCMP_TEXCOORD: compare texture coordinates
 * The flags are set using SetVertexCompareFlags().
 *
 * The comparison order is as follows:
 * First the location is compared (if so requested).
 * If location is equal, compare the normals (if requested)
 * If location and normal is equal, compare texture coordinates.
 *
 * The vertex comparison routines return
 * XYZ_EQUAL: is the vertices are equal
 * a code from 0 to 7 if the vertices are not equal. This code can be used
 * to sort vertices in an octree. The code is a combination of the flags
 * X_GREATER, Y_GREATER and Z_GREATER and is the same as for VectorCompare in
 * vector.h.
 */

#define VCMP_LOCATION		       0x01
#define VCMP_NORMAL		       0x02
#define VCMP_TEXCOORD		       0x04
#define VCMP_NO_NORMAL_IS_EQUAL_NORMAL 0x80

/* returns current flags */
extern unsigned VertexSetCompareFlags(unsigned flags);

extern int VertexCompare(VERTEX *v1, VERTEX *v2);
extern int VertexCompareLocation(VERTEX *v1, VERTEX *v2);

#ifdef __cplusplus
}
#endif

#endif /*_PHY_VERTEX_H_*/
