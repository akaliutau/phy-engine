/* geom_type.h: GEOMetry type declaration */

#ifndef _GEOM_TYPE_H_
#define _GEOM_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bounds.h"
#include "geom_methods.h"

/* Currently, there are three types of GEOMetries:
 *
 * - the COMPOUND: an aggregate GEOMetry which is basically a list of other 
 *	GEOMetries, useful for representing the scene in a hierarchical manner
 *	see compound.[ch].
 *
 * - the SURFACE: a primitive GEOMetry which is basically a list of
 *	PATCHES representing some simple object with given MATERIAL properties
 *	etc.. see surface.[ch].
 *
 * - the PATCHLIST: a primitive GEOMetry consisting of a list of patches
 *	without material properties and such. Used during shaft culling only,
 *	see patchlist.c, patchlist_geom.h, shaftculling.[ch].
 *
 * Each of these primitieves has certain specific data. The GEOM struct
 * contains data that is independent of GEOMetry type. */

typedef struct GEOM {
  /* unique ID number */
  int id;

  /* specific data for the geometry: varies according to the type of 
   * geometry. */
  void 	*obj;	

  /* a set of functions (methods) to operate on a geometry, defined in
   * geom_methods.h. */
  struct GEOM_METHODS	*methods;

  /* a bounding box for the geometry. */
  BOUNDINGBOX bounds;

  /* data specific to the radiance algorithm being used. */
  void *radiance_data;

  /* display list ID for faster hardware rendering - initialised to -1 */
  int dlistid;

  /* temporary data. */
  union {int i; void *p;} tmp;

  /* a flag indicating whether or not the geometry has a bounding box */
  char bounded, 	/* nonzero if bounded geometry */
       shaftcullgeom,	/* generated during shaftculling */
       omit, 		/* indicates that the GEOM should not be considered
			 * for a number of things, such as intersection
			 * testing. Set to FALSE by default, don't forget
			 * to set to FALSE again after you changed it!! */
       flag2;		/* to make a multiple of 4 bytes */
} GEOM;

#ifdef __cplusplus
}
#endif

#endif /*_GEOM_TYPE_H_*/

