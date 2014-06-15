/* elementtype.h: Monte Carlo radiosity element type */

#ifndef _ELEMENT_TYPE_H_
#define _ELEMENT_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define HMC		/* don't try to switch it off in this version of the code */

#include "translatenames.h"
#include "elementlist.h"
#include "basis.h"
#include "niederreiter.h"

typedef union PatchOrGeomPtr {
  PATCH *patch;
  GEOM *geom;
} PatchOrGeomPtr;

typedef struct ELEMENT {
  PatchOrGeomPtr pog;	/* pointer to patch for surface elements or
			 * to geometry for cluster elements */
  long id;		/* unique ID number */
  COLOR Ed, Rd;		/* average diffuse emittance and reflectance of element */

  niedindex ray_index; 	/* incremented each time a ray is shot from the elem */

  float quality;	/* for merging the result of multiple iterations */
  float prob;		/* sampling probability */
  float ng;		/* nr of samples gathered on the patch */
  float area;		/* area of all surfaces contained in the element */

  BASIS *basis;		/* radiosity approximation data, see basis.h */
  /* higher order approximations need an array of color values for representing
   * radiance. */
  COLOR	*rad, *unshot_rad, *received_rad;
  COLOR source_rad;   	/* always constant source radiosity */

#ifdef IDMCR		/* for view-importance driven sampling */
  float imp, unshot_imp, received_imp, source_imp;
  niedindex imp_ray_index;	/* ray index for importance propagation */
#endif /*IDMCR */

#ifdef HMC
  VECTOR midpoint;			/* element midpoint */
  VERTEX *vertex[4];		 	/* up to 4 vertex pointers for
					 * surface elements */
  struct ELEMENT *parent;		/* parent element in hierarchy */
  struct ELEMENT **regular_subelements;	/* for surface elements with regular
					 * quadtree subdivision */
  struct ELEMENTLIST *irregular_subelements; /* clusters */
  TRANSFORM2D *uptrans;			/* relates surface element (u,v)
					 * coordinates to patch (u,v)
					 * coordinates */
  signed char child_nr;			/* -1 for clusters or toplevel 
					 * surface elements, 0..3 for
					 * regular surface subelements */
  char nrvertices;			/* nr of surf. element vertices */
  char iscluster;			/* whether it is a cluster or not */
  char flags;				/* unused so far */
#endif
} ELEMENT;

#ifdef __cplusplus
}
#endif

#endif /*_ELEMENT_TYPE_H_*/
