/* topology.h: all things that have to do with the BBOX library, for adding
 * more topological data to VERTEXes and PATCHes. */

#ifndef _TOPOLOGY_H_
#define _TOPOLOGY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "patchlist.h"
#include "bbox.h"

/* Adds topological information to VERTEXes and PATCHes. This information is
 * automatically freed when destroying the VERTEXes and PATCHes or
 * explicitely using BBoxCheckOut(). The
 * topological data for a VERTEX is a BBOX_VERTEX with a pointer to the
 * VERTEX as client data. The data for a PATCH is a BBOX_FACE with as client
 * data a pointer to the PATCH. This face contains one contour, with wings 
 * connecting the vertices of the patch in order. No client data is passed
 * when creating the BBOX_CONTOURs or BBOX_EDGEs. It is possible to connect
 * your own client data to the topological entities by setting a create 
 * callback for the entities, see BBOX/bbox.h. */
extern void BBoxCheckIn(PATCHLIST *patches);

/* Destroys the topological information kept with the PATCHes and their 
 * VERTEXes. */
extern void BBoxCheckOut(PATCHLIST *patches);

#ifdef __cplusplus
}
#endif

#endif /*_TOPOLOGY_H_*/
