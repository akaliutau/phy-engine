/* surface.h: SURFACEs: surfaces are basically a list of PATCHes representing
 * a simple object with given MATERIAL. */

#ifndef _SURFACE_H_
#define _SURFACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vectorlist.h"
#include "vertexlist.h"
#include "patchlist.h"
#include "material.h"
#include "geom_methods.h"

typedef struct SURFACE {
  /* an id which can be used for debugging. */
  int 	id;	

  /* MATERIAL the surface is made of */
  MATERIAL *material;

  /* a list of points at the vertices of the patches of the surface */
  VECTORLIST *points;

  /* a list of normals at the vertices of the patches. */
  VECTORLIST *normals;

  /* a list of texture coordinates of the vertices of the patches */
  VECTORLIST *texCoords;

  /* the vertices of the patches. Each vertex contains a pointer to the vertex 
   * coordinates and normal vector at the vertex, which are in the SURFACEs 'points'
   * and 'normals' list. Different vertices can share the same coordinates and/or 
   * normals. */
  struct VERTEXLIST *vertices;

  /* the PATCHes making up the SURFACE. Each PATCH contains pointers to three
   * or four VERTEXes in the 'vertices' list of the SURFACE. Different PATCHes
   * can share the same VERTEX. Each VERTEX also contains a list of pointers to
   * the PATCHes that share the VERTEX. This can be used for e.g. Gouraud shading
   * if a color is assigned to each VERTEX. Each PATCH also contains a backpointer to
   * to SURFACE to which it belongs. */
  struct PATCHLIST *faces;
} SURFACE;

#include <stdio.h>
#include "geom_methods.h"

enum COLORFLAGS {NO_COLORS, VERTEX_COLORS, FACE_COLORS};

/* This routine creates a SURFACE struct with given material, points, etc... */
extern SURFACE *SurfaceCreate(MATERIAL *material, 
			      VECTORLIST *points, VECTORLIST *normals, VECTORLIST *texCoords,
			      struct VERTEXLIST *vertices, struct PATCHLIST *faces,
			      enum COLORFLAGS flags);

/* A struct containing pointers to functions to operate on a SURFACE,
 * as declared in geom_methods.h. */
extern struct GEOM_METHODS surfaceMethods;
#define SurfaceMethods()	(&surfaceMethods)

/* tests whether a GEOM is a SURFACE. */
#define GeomIsSurface(geom)	(geom->methods == &surfaceMethods)

/* "cast" a geom to a surface */
#define GeomGetSurface(geom)    (GeomIsSurface(geom) ? (SURFACE*)(geom->obj) : (SURFACE*)NULL)

#ifdef __cplusplus
}
#endif

#endif /*_SURFACE_H_*/
