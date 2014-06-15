/* geom_methods.h: methods for operating on GEOMetries */

#ifndef _METHODS_H_
#define _METHODS_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "math/Boolean.h"
#include "bounds.h"
#include "patchlist.h"
#include "geomlist.h"
#include "ray.h"
#include "hitlist_type.h"

/* This struct contains pointers to functions (methods) to operate on a 
 * GEOMetry. The 'void *obj' passed to the functions is the "state" data 
 * for the GEOMetry, cast to a 'void *'. The implementation of these methods
 * varies according to the type of GEOMetry. For COMPOUND geometries, the
 * methods are implemented in compound.c. For SURFACEs, the methods are
 * implemented in surface.c. */
typedef struct GEOM_METHODS {
  /* this method will compute a bounding box for a GEOMetry. The bounding box
   * is filled in in boundingbox and a pointer to the filled in boundingbox 
   * returned. */
  float *(*bounds)(void *obj, float *boundingbox);

  /* this method will destroy the GEOMetry and it's children GEOMetries if 
   * any */
  void (*destroy)(void *obj);

  /* this method will print the GEOMetry to the file out */
  void (*print)(FILE *out, void *obj);

  /* returns the list of children GEOMetries if the GEOM is an aggregate.
   * This method is not implemented for primitive GEOMetries. */
  struct GEOMLIST *(*primlist)(void *obj);

  /* returns the list of PATCHes making up a primitive GEOMetry. This
   * method is not implemented for aggregate GEOMetries. */
  struct PATCHLIST *(*patchlist)(void *obj);

  /* discretisation_intersect returns NULL is the ray doesn't hit the discretisation
   * of the object. If the ray hits the object, a hit record is returned containing
   * information about the intersection point. See geom.h for more explanation. */
  struct HITREC *(*discretisation_intersect)(void *obj, RAY *ray, float mindist, float *maxdist, int hitflags, HITREC *hitstore);

  /* similar, but appends all found intersections to the hitlist hit record
   * list. The possibly modified hitlist is returned. */
  struct HITLIST *(*all_discretisation_intersections)(struct HITLIST *hits, void *obj, RAY *ray, float mindist, float maxdist, int hitflags);

  /* duplicate: returns a duplicate of the object's data */
  void *(*duplicate)(void *obj);
} GEOM_METHODS;

#ifdef __cplusplus
}
#endif

#endif /*_METHODS_H_*/
