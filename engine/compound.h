/* compound.h: COMPOUND geometries. A COMPOUND is basically a list of
 * GEOMetries, useful for representing a scene in a hierarchical manner. */
#ifndef _COMPOUND_H_
#define _COMPOUND_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "geomlist.h"

/* The specific data for a COMPOUND GEOMetry (see geom_type.h) is
 * nothing more than the GEOMLIST containing the children GEOMs. */
#define COMPOUND GEOMLIST

/* This function creates a COMPOUND from a linear list of GEOMetries. 
 * Actually, it just counts the number of COMOUNDS in the scene and
 * returns the geomlist. You can as well directly pass 'geomlist'
 * to GeomCreate() for creating a COMPOUND GEOM if you don't want
 * it to be counted. */
extern COMPOUND *CompoundCreate(GEOMLIST *geomlist);

/* A struct containing pointers to the functions (methods) to operate
 * on COMPOUNDs. */
extern struct GEOM_METHODS compoundMethods;
#define CompoundMethods()	(&compoundMethods)

/* Tests if a GEOM is a compound. */
#define GeomIsCompound(geom) 	(geom->methods == &compoundMethods)

/* "cast" a geom to a compound */
#define GeomGetCompound(geom)    (GeomIsCompound(geom) ? (COMPOUND*)(geom->obj) : (COMPOUND*)NULL)

#ifdef __cplusplus
}
#endif

#endif /*_COMPOUND_H_*/
