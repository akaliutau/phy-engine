/* geom.h: GEOMetries */
#ifndef _GEOM_H_
#define _GEOM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "geom_type.h"
#include "geomlist.h"

/* This function is used to create a new GEOMetry with given specific data and
 * methods. A pointer to the new GEOMetry is returned. */
extern GEOM *GeomCreate(void *obj, struct GEOM_METHODS *methods);

/* This function prints the GEOMetry data to the file out */
extern void GeomPrint(FILE *out, GEOM *geom);

/* This function returns a bounding box for the GEOMetry */
extern float *GeomBounds(GEOM *geom);

/* This function destroys the given GEOMetry */
extern void GeomDestroy(GEOM *geom);

/* This function returns nonzero if the given GEOMetry is an aggregate. An
 * aggregate is a geometry that consists of simpler geometries. Currently,
 * there is only one type of aggregate geometry: the compound, which is basically 
 * just a list of simpler geometries. Other aggregate geometries are also
 * possible, e.g. CSG objects. If the given GEOMetry is a primitive, zero is 
 * returned. A primitive GEOMetry is a GEOMetry that does not consist of
 * simpler GEOMetries. */
extern int GeomIsAggregate(GEOM *geom);

/* Returns a linear list of the simpler GEOMEtries making up an aggregate GEOMetry.
 * A NULL pointer is returned if the GEOMetry is a primitive. */
extern struct GEOMLIST *GeomPrimList(GEOM *geom);

/* Returns a linear list of patches making up a primitive GEOMetry. A NULL
 * pointer is returned if the given GEOMetry is an aggregate. */
extern struct PATCHLIST *GeomPatchList(GEOM *geom);

/* This routine returns NULL is the ray doesn't hit the discretisation of the
 * GEOMetry. If the ray hits the discretisation of the GEOM, a pointer to a
 * struct containing (among other information) the hit patch is returned. 
 * The hitflags (defined in ray.h) determine whether the nearest intersection
 * is returned, or rather just any intersection (e.g. for shadow rays in 
 * ray tracing or for form factor rays in radiosity), whether to consider
 * intersections with front/back facing patches and what other information
 * besides the hit patch (interpolated normal, intersection point, material 
 * properties) to return. 
 * The argument hitstore points to a HITREC in which the hit data can be
 * filled in. */
extern struct HITREC *GeomDiscretisationIntersect(GEOM *geom, RAY *ray, 
						 float mindist, float *maxdist, 
						 int hitflags, HITREC *hitstore);

/* similar, but returns a doubly linked list with all encountered intersections. */
extern HITLIST *GeomAllDiscretisationIntersections(HITLIST *hits, 
						   GEOM *geom, RAY *ray, 
						   float mindist, float maxdist,
						   int hitflags);


/* Will avoid intersection testing with geom1 and geom2 (possibly NULL 
 * pointers). Can be used for avoiding immediate selfintersections. */
extern void GeomDontIntersect(GEOM *geom1, GEOM *geom2);

/* This routine creates and returns a duplicate of the given geometry. Needed for
 * shaft culling. */
extern GEOM *GeomDuplicate(GEOM *geom);

#ifdef __cplusplus
}
#endif

#endif /*_GEOM_H_*/

