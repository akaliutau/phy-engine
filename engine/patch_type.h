/* patch_type.h: PATCH type declaration in separate header file */

#ifndef _PATCH_TYPE_H_
#define _PATCH_TYPE_H_

/* #define LOWMEM_INTERSECT      * to new ray-polygon intersection code (Oct 2000)
                                 * which is slower than the old October 1997
				 * code, but saves 36 bytes per patch */

/* #undef CHECK_INTERSECT	 * compare both ray-polygon intersection 
                                 * strategies: fast with lots of storage and 
				 * slower with less storage */

#ifndef FAST_INTERSECT
#define LOWMEM_INTERSECT	/* make lowmem intersect the default */
#endif

#ifdef CHECK_INTERSECT
#undef LOWMEM_INTERSECT		/* in order to include required old code */
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "vertex_type.h"
#include "surface.h"
#include "math/jacobian.h"
#include "bbox.h"

#define PATCHMAXVERTICES 4	/* max. 4 vertices per patch */

typedef struct PATCH {
  unsigned 	id;		/* identification number for debugging, ID rendering ... */

  struct PATCH *twin;	/* twin face (for double sided surfaces) */
  BBOX_FACE *brep_data;	/* topological data for the patch. Only filled
			 * in if a radiance method needs it. */    

  struct VERTEX	*vertex[PATCHMAXVERTICES];	/* pointers to the vertices */

  struct SURFACE *surface; /* pointer to surface data (contains vertexlist, 
			    * material properties ... */

  float	*bounds;	/* bounding box */

  VECTOR normal;	/* patch normal */
  float plane_constant;	/* patch plane constant */
  float tolerance;	/* patch plane tolerance */
  float	area;		/* patch area */
  VECTOR midpoint;	/* patch midpoint */
  JACOBIAN *jacobian;	/* shape-related constants for irregular quadrilaterals.
			 * Used for sampling the quadrilateral and for computing
			 * integrals. */
  float	direct_potential; /* directly received hemispherical potential
			 * (ref: Pattanaik, ACM Trans Graph). 
			 * Only determined when asked to do so 
			 * (see potential.[ch]). */
#ifndef LOWMEM_INTERSECT
  float eslope[4], edist[4];
  unsigned char eflags[4];
#endif
  char	nrvertices;	/* nr of vertices: 3 or 4 */
  char	index;		/* indicates dominant part of patch normal */
  char omit;		/* indicates that the patch should not be considered
			 * for a couple of things, such as intersection
			 * testing, shaft culling, ... set to FALSE by
			 * default. Don't forget to set to FALSE again
			 * after you changed it!! */
  unsigned char	flags;	/* other flags */

  RGB	color;		/* color used to flat render the patch. */

  void *radiance_data;	/* data needed for radiance computations. Type 
			 * depends on the current radiance algorithm. 
			 * see radiance.h. */
#ifdef RECORD_MONITORED
  FILE *monitor_file[MAX_RECORD_FILES];
#endif
} PATCH; 

#ifdef __cplusplus
}
#endif

#endif /*_PATCH_TYPE_H_*/
