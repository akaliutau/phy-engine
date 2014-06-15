/* brep_private.h: brep.c and friends private data structures and routines */

#ifndef _BBOX_PRIVATE_H_
#define _BBOX_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bbox.h"

/* common first fields of BBOX_SHELL, ... */
typedef struct BBOX_RING {
  void *client_data;
  struct BBOX_RING *prev, *next;
} BBOX_RING;

/* iterators over a ring such as BBOX_SHELL, ... */
extern void BBoxIterate(BBOX_RING *ring, void (*func)(BBOX_RING *));
extern void BBoxIterate1A(BBOX_RING *ring, void (*func)(BBOX_RING *, void *), void *parm);
extern void BBoxIterate2A(BBOX_RING *ring, void (*func)(BBOX_RING *, void *, void *), void *parm1, void *parm2);

/* prints an informational, warning, error, fatal error  message */
extern void BBoxInfo(void *client_data, char *routine, char *text, ...);
extern void BBoxWarning(void *client_data, char *routine, char *text, ...);
extern void BBoxError(void *client_data, char *routine, char *text, ...);
extern void BBoxFatal(void *client_data, char *routine, char *text, ...);

/* various actions to be performed when a shell is specified completely */
extern void BBoxCloseShell(BBOX_SHELL *shell);

/* various actions to be performed when a face is specified completely */
extern void BBoxCloseFace(BBOX_FACE *face);

/* various actions to be performed when a contour is specified completely */
extern void BBoxCloseContour(BBOX_CONTOUR *contour);

/* various actions to be performed when an edge has been specified 
 * completely, i.e. it is enclosed in two contours. */
extern void BBoxCloseEdge(BBOX_EDGE *edge);

/* various actions to be performed when a contour has been updated */
extern void BBoxUpdateContour(BBOX_CONTOUR *contour);

/* Creates an edge connecting vertex1 to vertex2. It is not
 * connected to a contour. It calls the CreateEdge callback
 * if set. */
extern BBOX_EDGE *BBoxCreateEdge(BBOX_VERTEX *vertex1, BBOX_VERTEX *vertex2, void *client_data);

/* Creates a wing from vertex1 to vertex2. First looks for an
 * existing edge connecting the vertices. Creates such an edge if 
 * if there is not yet one. Returns a pointer to an unused wing in
 * the edge. If there is no unused wing in this edge, complains, and
 * creates a second edge between the two vertices. 
 * The wing still needs to be connected in a contour. */
extern BBOX_WING *BBoxCreateWingWithoutContour(BBOX_VERTEX *vertex1, BBOX_VERTEX *vertex2, void *client_data);

/* connects the shell to the solid */
extern void BBoxConnectShellToSolid(BBOX_SHELL *shell, BBOX_SOLID *solid);

/* disconnect the shell from the solid */
extern void BBoxDisconnectShellFromSolid(BBOX_SHELL *shell);

/* connects the face to the shell */
extern void BBoxConnectFaceToShell(BBOX_FACE *face, BBOX_SHELL *shell);

/* disconnect the face from its containing shell */
extern void BBoxDisconnectFaceFromShell(BBOX_FACE *face);

/* connects a contour to the specified face */
extern void BBoxConnectContourToFace(BBOX_CONTOUR *contour, BBOX_FACE *face);

/* disconnects a contour from its containing face */
extern void BBoxDisconnectContourFromFace(BBOX_CONTOUR *contour);

/* connect the wing as last wing in the contour */
extern void BBoxConnectWingToContour(BBOX_WING *wing, BBOX_CONTOUR *contour);

/* Disconnect the wing from the contour. Use with care: the contour
 * might not be a loop anymore after disconnecting the wing! */
extern void BBoxDisconnectWingFromContour(BBOX_WING *wing);

/* Connect the wing to its starting vertex. The wing must have been connected 
 * to its contour before. */
extern void BBoxConnectWingToVertex(BBOX_WING *wing);

/* Disconnect the wing from its starting vertex. The wing must be properly 
 * connected to a contour. */
extern void BBoxDisconnectWingFromVertex(BBOX_WING *wing);

/* release all storage associated with an edge if not used anymore in
 * any contour (the given edge must already be disconnected from its contours.) */
extern void BBoxDestroyEdge(BBOX_EDGE *edge);

#ifdef __cplusplus
}
#endif

#endif /*_BBOX_PRIVATE_H_*/
