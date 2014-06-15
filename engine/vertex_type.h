/* vertex_type.h: vertex type declaration. Separate header file to avoid too
 * much recompilation when e.g. adding a function to vertex.h */

#ifndef _VERTEX_TYPE_H_
#define _VERTEX_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vector.h"
#include "patchlist.h"
#include "color.h"
#include "bbox.h"

typedef struct VERTEX {
  VECTOR *point;	/* pointer to the coordinates of the vertex  */
  VECTOR *normal;	/* pointer to the normal vector at the vertex */
  VECTOR *texCoord;	/* texture coordinates */
  struct PATCHLIST  *patches;	/* list of patches sharing the vertex */
  RGB    color;		/* color for the vertex when rendering with Gouraud 
			 * interpolation */
  BBOX_VERTEX *brep_data; /* topological data for the VERTEX,
			 * only filled in if a radiance method needs
			 * it. */
  void *radiance_data;	/* data for the vertex maintained by the current
			 * radiance method. */
  struct VERTEX *back;	/* vertex at the same position, but with 
			 * reversed normal, for back faces. */
  int tmp;		/* some temporary storage for vertices, used e.g. for
			 * saving VRML. Do not assume the contents of
			 * this storage remain unchanged after leaving
			 * control to the user. */
  int id;		/* id number for debugging etc... */
} VERTEX;

#ifdef __cplusplus
}
#endif

#endif /*_VERTEX_TYPE_H_*/
