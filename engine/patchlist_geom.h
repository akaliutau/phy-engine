/* patchlist_geom.h: makes PATCHLISTs look like GEOMs. Required for shaft culling. */

#ifndef _PATCHLIST_GEOM_H_
#define _PATCHLIST_GEOM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "geom_methods.h"
#include "geom_type.h"

extern struct GEOM_METHODS patchlistMethods;

/* "returns" the GEOM methods for a patchlist */
#define PatchListMethods() (&patchlistMethods)

/* "returns" TRUE if the GEOMetry is a patchlist */
#define GeomIsPatchlist(geom) (geom->methods == &patchlistMethods)

#ifdef __cplusplus
}
#endif

#endif /*_PATCHLIST_GEOM_H_*/
