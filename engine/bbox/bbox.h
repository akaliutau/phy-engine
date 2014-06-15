#ifndef _BBOX_H_
#define _BBOX_H_

#ifdef __cplusplus
extern "C" {
#endif

/* ******************* The Topological Data Structure *********************/
/*
 * data structure representing the boundary of polyhedral solids
 * References:
 *
 */

typedef struct BBOX_SOLID {
  void *client_data;			/* pointer to whatever data the user 
					 * wants to keep with the solid */
  struct BBOX_SHELL *shells;
  struct BBOX_VERTEX_OCTREE *vertices;	/* vertices, sorted in an octree 
					 * for fast lookup. */
} BBOX_SOLID;

typedef struct BBOX_SHELL {
  void *client_data;
  struct BBOX_SHELL *prev, *next;	/* doubly linked ring to make
					 * insertions and removals easy */
  struct BBOX_FACE *faces;
  struct BBOX_SOLID *solid;		/* backpointer to the containing 
					 * solid */
} BBOX_SHELL;

typedef struct BBOX_FACE {
  void *client_data;
  struct BBOX_FACE *prev, *next;	/* faces form a doubly linked ring */
  struct BBOX_CONTOUR *outer_contour;
  struct BBOX_SHELL *shell;		/* backpointer to the containing 
					 * shell */
} BBOX_FACE;

typedef struct BBOX_CONTOUR {
  void *client_data;
  struct BBOX_CONTOUR *prev, *next;	/* contours form a doubly linked ring 
					 * in order to facilitate the splitting
					 * and merging of contours. There is 
					 * one ring per face, containing the 
					 * outer contour as well as the 
					 * rings. */
  struct BBOX_WING *wings;
  struct BBOX_FACE *face;		/* backpointer to the containing 
					 * face */
} BBOX_CONTOUR;

typedef struct BBOX_WING {
  struct BBOX_VERTEX *vertex;		/* startvertex */
  struct BBOX_WING *prev, *next;	/* ClockWise and CounterClockWise
					 * next wing within outer contours */
  struct BBOX_EDGE *edge;		/* pointer to the containing edge */
  struct BBOX_CONTOUR *contour;		/* backpointer to the containing 
					 * contour */
} BBOX_WING;

typedef struct BBOX_EDGE {
  void *client_data;
  struct BBOX_WING wing[2];		/* two wings */
} BBOX_EDGE;

typedef struct BBOX_WING_RING {
  BBOX_WING *wing;
  struct BBOX_WING_RING *prev, *next;
} BBOX_WING_RING;

/* A vertex is basically a point in 3D space. */
typedef struct BBOX_VERTEX {
  void *client_data;
  BBOX_WING_RING *wing_ring;		 /* ring of wings leaving at the vertex */
} BBOX_VERTEX;


typedef void *(*BBOX_CALLBACK_FUNC)(void *);

extern BBOX_CALLBACK_FUNC BBoxSetCreateSolidCallback(BBOX_CALLBACK_FUNC func);
extern BBOX_CALLBACK_FUNC BBoxSetCreateShellCallback(BBOX_CALLBACK_FUNC func);
extern BBOX_CALLBACK_FUNC BBoxSetCreateFaceCallback(BBOX_CALLBACK_FUNC func);
extern BBOX_CALLBACK_FUNC BBoxSetCreateContourCallback(BBOX_CALLBACK_FUNC func);
extern BBOX_CALLBACK_FUNC BBoxSetCreateEdgeCallback(BBOX_CALLBACK_FUNC func);
extern BBOX_CALLBACK_FUNC BBoxSetCreateVertexCallback(BBOX_CALLBACK_FUNC func);

extern BBOX_CALLBACK_FUNC BBoxSetCloseSolidCallback(BBOX_CALLBACK_FUNC func);
extern BBOX_CALLBACK_FUNC BBoxSetCloseShellCallback(BBOX_CALLBACK_FUNC func);
extern BBOX_CALLBACK_FUNC BBoxSetCloseFaceCallback(BBOX_CALLBACK_FUNC func);
extern BBOX_CALLBACK_FUNC BBoxSetCloseContourCallback(BBOX_CALLBACK_FUNC func);
extern BBOX_CALLBACK_FUNC BBoxSetCloseEdgeCallback(BBOX_CALLBACK_FUNC func);
extern BBOX_CALLBACK_FUNC BBoxSetCloseVertexCallback(BBOX_CALLBACK_FUNC func);

extern BBOX_CALLBACK_FUNC BBoxSetDestroySolidCallback(BBOX_CALLBACK_FUNC func);
extern BBOX_CALLBACK_FUNC BBoxSetDestroyShellCallback(BBOX_CALLBACK_FUNC func);
extern BBOX_CALLBACK_FUNC BBoxSetDestroyFaceCallback(BBOX_CALLBACK_FUNC func);
extern BBOX_CALLBACK_FUNC BBoxSetDestroyContourCallback(BBOX_CALLBACK_FUNC func);
extern BBOX_CALLBACK_FUNC BBoxSetDestroyEdgeCallback(BBOX_CALLBACK_FUNC func);
extern BBOX_CALLBACK_FUNC BBoxSetDestroyVertexCallback(BBOX_CALLBACK_FUNC func);


typedef void (*BBOX_MSG_CALLBACK_FUNC)(void *client_data, char *message);

/* No error message from the library is longer than this */
#define BBOX_MAX_MESSAGE_LENGTH 200

/* The SetCallback functions return the previously set callback function */
extern BBOX_MSG_CALLBACK_FUNC BBoxSetInfoCallback(BBOX_MSG_CALLBACK_FUNC f);
extern BBOX_MSG_CALLBACK_FUNC BBoxSetWarningCallback(BBOX_MSG_CALLBACK_FUNC f);
extern BBOX_MSG_CALLBACK_FUNC BBoxSetErrorCallback(BBOX_MSG_CALLBACK_FUNC f);
extern BBOX_MSG_CALLBACK_FUNC BBoxSetFatalCallback(BBOX_MSG_CALLBACK_FUNC f);


/* *********************** Contructors ***************************** */
/* creates an empty solid, client_data supplied */
extern BBOX_SOLID *BBoxCreateSolid(void *client_data);

/* creates a new empty shell within the solid if solid is not a NULL
 * pointer. You will need to call BBoxCloseShell() yourself to
 * close the shell and/or BBoxConnectShellToSolid() to connect it
 * to a solid if you don't specify a solid to contain the shell with
 * this function. */
extern BBOX_SHELL *BBoxCreateShell(BBOX_SOLID *solid, void *client_data);

/* creates a new empty face within the shell if shell is not a NULL
 * pointer. */
extern BBOX_FACE *BBoxCreateFace(BBOX_SHELL *shell, void *client_data);

/* creates a new empty contour within the face. 'face' cannot be a NULL
 * pointer. */
extern BBOX_CONTOUR *BBoxCreateContour(BBOX_FACE *face, void *client_data);

/* creates a new vertex. The new vertex is not installed it in a vertex 
 * octree as vertices do not have to be sorted in an octree, see below. */
extern BBOX_VERTEX *BBoxCreateVertex(void *client_data);

extern BBOX_WING *BBoxCreateWing(BBOX_VERTEX *vertex1, BBOX_VERTEX *vertex2, BBOX_CONTOUR *contour, void *client_data);

/* performs various actions to be done when the boundary representation of
 * the solid has been completely specified. Automatically calls 
 * BBoxCloseShell() for its contained shells and so on ... */
extern void BBoxCloseSolid(BBOX_SOLID *solid);

/* The following functions are only needed when creating "orphan"
 * shells and faces (= without specifying the parent solid/shell
 * when creating them). */

/* same for a shell. Only needed for shells that were created without
 * specifying the enclosing solid. */
extern void BBoxCloseShell(BBOX_SHELL *shell);

/* same for a face. Note that contours and wings/edges cannot be created
 * without a containing face, ... */
extern void BBoxCloseFace(BBOX_FACE *face);

/* connects the shell to the solid */
extern void BBoxConnectShellToSolid(BBOX_SHELL *shell, BBOX_SOLID *solid);

/* disconnect the shell from the solid */
extern void BBoxDisconnectShellFromSolid(BBOX_SHELL *shell);

/* connects the face to the shell */
extern void BBoxConnectFaceToShell(BBOX_FACE *face, BBOX_SHELL *shell);

/* disconnect the face from its containing shell */
extern void BBoxDisconnectFaceFromShell(BBOX_FACE *face);


/* ************************ Modifiers ******************************* */

/* Splits an edge in two at the specified vertex (at least: 
 * topologically; the new vertex doesn't have to be collinear
 * with the endpoints of the edge to be split, but you should make
 * sure that no contours are created that intersect in other
 * places than at vertices and along edges. The contours
 * to which the edge belongs are assumed to be closed loops. */
extern void BBoxSplitEdge(BBOX_EDGE *edge, BBOX_VERTEX *vertex);

/* Splits only one wing. If a full edge is to be split, also its
 * wing in the other contour (if any) needs to be split at the same vertex.
 * (BBoxSplitEdge does this).
 * Returns the wing leaving at the inserted vertex. */
extern BBOX_WING *BBoxSplitWing(BBOX_WING *wing, BBOX_VERTEX *vertex);

/* Join two edges at their common endpoint. The edges must be used
 * in the same contours and must share an endpoint. */
extern void BBoxJoinEdges(BBOX_EDGE *edge1, BBOX_EDGE *edge2);

extern BBOX_WING *BBoxJoinWings(BBOX_WING *wing1, BBOX_WING *wing2);

extern BBOX_WING *BBoxMakeEdgeSplitContour(BBOX_WING *wing1, BBOX_WING *wing2,
					   void *edge_data, void *contour_data);

extern BBOX_WING *BBoxMakeEdgeJoinContours(BBOX_WING *wing1, BBOX_WING *wing2, 
					   void *edge_data);

extern BBOX_WING *BBoxDeleteEdgeJoinContours(BBOX_WING *wing);

extern BBOX_WING *BBoxDeleteEdgeSplitContour(BBOX_WING *wing, void *contour_data);

extern BBOX_WING *BBoxMakeNotch(BBOX_WING *wing, BBOX_VERTEX *vertex, 
				void *edge_data);

extern BBOX_WING *BBoxMakeSlit(BBOX_FACE *face, BBOX_VERTEX *v1, BBOX_VERTEX *v2,
			       void *edge_data, void *contour_data);

extern BBOX_WING *BBoxWingReplaceVertex(BBOX_WING *wing, BBOX_VERTEX *newvertex);


extern void BBoxSolidIterateShells(BBOX_SOLID *solid, void (*func)(BBOX_SHELL *));
extern void BBoxSolidIterateShells1A(BBOX_SOLID *solid, void (*func)(BBOX_SHELL *, void *), void *parm);
extern void BBoxSolidIterateShells2A(BBOX_SOLID *solid, void (*func)(BBOX_SHELL *, void *, void *), void *parm1, void *parm2);

/* execute func for every face in the shell */
extern void BBoxShellIterateFaces(BBOX_SHELL *shell, void (*func)(BBOX_FACE *));
extern void BBoxShellIterateFaces1A(BBOX_SHELL *shell, void (*func)(BBOX_FACE *, void *), void *parm);
extern void BBoxShellIterateFaces2A(BBOX_SHELL *shell, void (*func)(BBOX_FACE *, void *, void *), void *parm1, void *parm2);

/* iterate over the contours of the face */
extern void BBoxFaceIterateContours(BBOX_FACE *face, void (*func)(BBOX_CONTOUR *));
extern void BBoxFaceIterateContours1A(BBOX_FACE *face, void (*func)(BBOX_CONTOUR *, void *), void *parm);
extern void BBoxFaceIterateContours2A(BBOX_FACE *face, void (*func)(BBOX_CONTOUR *, void *, void *), void *parm1, void *parm2);

/* Iterate over the wings in the contour. When properly built, the inside of 
 * the face containing the contour is always on the left side of an edge. */
extern void BBoxContourIterateWings(BBOX_CONTOUR *contour, void (*func)(BBOX_WING *));
extern void BBoxContourIterateWings1A(BBOX_CONTOUR *contour, void (*func)(BBOX_WING *, void *), void *parm);
extern void BBoxContourIterateWings2A(BBOX_CONTOUR *contour, void (*func)(BBOX_WING *, void *, void *), void *parm1, void *parm2);

/* Iterates over the vertices in a contour. Same order as when iterating 
 * over the edges. */
extern void BBoxContourIterateVertices(BBOX_CONTOUR *contour, void (*func)(BBOX_VERTEX *));
extern void BBoxContourIterateVertices1A(BBOX_CONTOUR *contour, void (*func)(BBOX_VERTEX *, void *), void *parm);

/* Iterate over the wings leaving the vertex, in arbitrary order. */
extern void BBoxVertexIterateWings(BBOX_VERTEX *vertex, void (*func)(BBOX_WING *));
extern void BBoxVertexIterateWings1A(BBOX_VERTEX *vertex, void (*func)(BBOX_WING *, void *), void *parm);

/* Iterator over all wings (included in a contour) connecting the two 
 * given vertices. */
extern void BBoxIterateWingsWithVertex(BBOX_VERTEX *vertex1, BBOX_VERTEX *vertex2, 
				       void (*func)(BBOX_WING *));

extern void BBoxIterateWingsWithVertex1A(BBOX_VERTEX *vertex1, BBOX_VERTEX *vertex2, 
					 void (*func)(BBOX_WING *, void *parm), 
					 void *parm);

/* You might prefer the following, more flexibale iterators however ... */
#define ForAllInRing(TYPE, elem, first, nextel) \
  TYPE *elem, *nextel; \
  for (elem=(first), nextel=elem->next; elem; elem = nextel==(first) ? (TYPE *)NULL : nextel, nextel=nextel->next)

#define ForAllShellsInSolid(shell, solid) { \
  if ((solid) && (solid)->shells) { \
    ForAllInRing(BBOX_SHELL, shell, (solid)->shells, next_shell) {

#define ForAllFacesInShell(face, shell) { \
  if ((shell) && (shell)->faces) { \
    ForAllInRing(BBOX_FACE, face, (shell)->faces, next_face) {

#define ForAllContoursInFace(contour, face) { \
  if ((face) && (face)->outer_contour) { \
    ForAllInRing(BBOX_CONTOUR, contour, (face)->outer_contour, next_contour) {

#define ForAllWingsInContour(wing, contour) { \
  if ((contour) && (contour)->wings) { \
    ForAllInRing(BBOX_WING, wing, (contour)->wings, next_wing) {

#define ForAllWingsLeavingVertex(_wing, vertex) { \
  if ((vertex) && (vertex)->wing_ring) { \
    ForAllInRing(BBOX_WING_RING, _r_, (vertex)->wing_ring, next_wingel) { \
      BBOX_WING *_wing = _r_->wing;

#ifndef EndForAll
#define EndForAll }}}
#endif

/* returns the other wing in the edge */
extern BBOX_WING *BBoxEdgeOtherWing(BBOX_WING *wing);

/* returns the next wing with same starting vertex as wing. The order is 
 * counterclockwise if the vertex is used in outer contours only and 
 * clockwise if used in inner contours only. Returns a null pointer
 * if the current wing is not properly connected to possible
 * other wings sharing the vertex, or the boundary of a partial
 * shell was reached. */
extern BBOX_WING *BBoxNextWingLeavingVertex(BBOX_WING *wing);

/* looks whether the wing is referenced in the wing ring of its
 * starting vertex. Returns a pointer to the BBOX_WING_RING element if
 * it is found and NULL if not */
extern BBOX_WING_RING *BBoxFindWingLeavingVertex(BBOX_WING *wing);

/* Looks whether an edge connecting the two vertices already
 * exists, Returns it if it exists. Returns NULL if not. */
extern BBOX_EDGE *BBoxFindEdge(BBOX_VERTEX *vertex1, BBOX_VERTEX *vertex2);


/* ************************ Destructors **************************** */
/* releases all memory associated with the solid, also its vertices if not used
 * in other solids as well. */
extern void BBoxDestroySolid(BBOX_SOLID *solid);

/* release all memory associated to a shell and its contained faces */
extern void BBoxDestroyShell(BBOX_SHELL *shell);

/* release all storage associated with a face and its contours, including
 * edges if not used in other faces as well */
extern void BBoxDestroyFace(BBOX_FACE *face);

/* release all storage associated with a contour and its wing/edges
 * if not used in other contours as well */
extern void BBoxDestroyContour(BBOX_CONTOUR *contour);

/* remove a wing from a contour, release the storage associated with the
 * edge if it is not used in other contours as well. This routine will 
 * in general create a hole in a contour: the endpoint of a previous edge
 * might not be the starting point of the next one where an edge has been
 * deleted. The vertices are not deleted. */
extern void BBoxDestroyWing(BBOX_WING *wing);

/* Remove a vertex from a (closed) contour, the wings containing the vertex are
 * removed as well (and their edges if not used in other contours as
 * well). If it is not the only vertex in the contour, a new wing connecting
 * the neighbooring vertices is created, so the contour remains
 * a loop. The vertex is not deleted. */
extern void BBoxContourRemoveVertex(BBOX_CONTOUR *contour, BBOX_VERTEX *vertex);

/* release all storage associated with the vertex if it is not used 
 * anymore in any edge. */
extern void BBoxDestroyVertex(BBOX_VERTEX *vertex);


/* ********************** vertex octrees *************************** */
/* vertices can also be sorted and stored into an octree in order to make 
 * searching for a vertex significantly faster. */
/* this struct must have the same layout as OCTREE in Octree.h */
typedef struct BBOX_VERTEX_OCTREE {
  BBOX_VERTEX *vertex;
  struct BBOX_VERTEX_OCTREE *child[8];
} BBOX_VERTEX_OCTREE;

/* Before trying to store vertices in an octree, first a routine should
 * be speciied to compare the client data of two vertices.
 * The routine specified should return a code with the following meaning:
 *
 * 	0:  x1 <= x2 , y1 <= y2, z1 <= z2 but not all are equal
 * 	1:  x1 >  x2 , y1 <= y2, z1 <= z2 
 * 	2:  x1 <= x2 , y1 >  y2, z1 <= z2 
 * 	3:  x1 >  x2 , y1 >  y2, z1 <= z2 
 * 	4:  x1 <= x2 , y1 <= y2, z1 >  z2 
 * 	5:  x1 >  x2 , y1 <= y2, z1 >  z2 
 * 	6:  x1 <= x2 , y1 >  y2, z1 >  z2 
 * 	7:  x1 >  x2 , y1 >  y2, z1 >  z2 
 * 	8:  x1 == x2 , y1 == y2, z1 == z2 
 *
 * in other words: 
 *
 *	code&1 == 1 if x1 > x2 and 0 otherwise
 *	code&2 == 1 if y1 > y2 and 0 otherwise
 *	code&4 == 1 if z1 > z2 and 0 otherwise
 *	code&8 == 1 if x1 == x2 and y1 == y2 and z1 == z2
 *
 * BBoxSetVertexCompareRoutine() returns the previously installed compare 
 * routine so it can be restored when necessary. */
typedef int (*BBOX_COMPARE_FUNC)(void *, void *);

extern BBOX_COMPARE_FUNC BBoxSetVertexCompareRoutine(BBOX_COMPARE_FUNC routine);

extern BBOX_COMPARE_FUNC BBoxSetVertexCompareLocationRoutine(BBOX_COMPARE_FUNC routine);

/* Looks up a vertex in the vertex octree, return NULL if not found */
extern BBOX_VERTEX *BBoxFindVertex(void *vertex_data, BBOX_VERTEX_OCTREE *vertices);

/* attaches a  BBOX_VERTEX to the vertex octree */
extern void BBoxAttachVertex(BBOX_VERTEX *vertex, BBOX_VERTEX_OCTREE **vertices);

/* removes a BBOX_VERTEX from the vertex octree */
extern void BBoxReleaseVertex(BBOX_VERTEX *vertex, BBOX_VERTEX_OCTREE **vertices);

/* Creates a vertex and installs it in the vertex octree, same as
 * BBoxCreateVertex() followed by BBoxAttachVertex() */
extern BBOX_VERTEX *BBoxInstallVertex(void *vertex_data, BBOX_VERTEX_OCTREE **vertices);

/* calls func for each BBOX_VERTEX in the octree */
extern void BBoxIterateVertices(BBOX_VERTEX_OCTREE *vertices, void (*func)(BBOX_VERTEX *));

/* For convenience, a vertex octree is created with each BBOX_SOLID.
 * The vertex octree bound to a BBOX_SOLID is also destroyed when
 * destroying the BBOX_SOLID. 
 * The routines below allow to create and destroy vertex octrees not
 * bound to a solid. */

/* creates a new vertex octree: currently only returns a NULL pointer */
extern BBOX_VERTEX_OCTREE *BBoxCreateVertexOctree(void);

/* Destroys a vertex octree, does not destroy the vertices referenced in
 * the octree. */
extern void BBoxDestroyVertexOctree(BBOX_VERTEX_OCTREE *vertices);

/* destroys all the vertices and the vertex octree, same as
 * BBoxIterateVertices(vertices, BBoxDestroyVertex) followed by
 * BBoxDestroyvertexOctree(). */
extern void BBoxDestroyVertices(BBOX_VERTEX_OCTREE *vertices);

/* Looks up the first vertex in the vertex octree at the location as specified in the 
 * given vertex data. There may be multiple vertices at the same location (e.g. having 
 * a different normal and/or a different name). These vertices should normally be
 * stored as a suboctree of the octree containing all vertices. A pointer to the
 * top of this suboctree is returned if there are vertices at the given location.
 * NULL is returned if there are no vertices at the given location. */
extern BBOX_VERTEX_OCTREE *BBoxFindVertexAtLocation(void *vertex_data, BBOX_VERTEX_OCTREE *vertices);

/* Iterators over all vertices in the given vertex octree that are at the same
 * location as specified in the vertex data. */
extern void BBoxIterateVerticesAtLocation(void *vertex_data, BBOX_VERTEX_OCTREE *vertices,
					  void (*routine)(BBOX_VERTEX *vertex));

/* 1 extra parameter */
extern void BBoxIterateVerticesAtLocation1A(void *vertex_data, BBOX_VERTEX_OCTREE *vertices,
					    void (*routine)(BBOX_VERTEX *vertex, void *parm), 
					    void *parm);

/* 2 extra parameters */
extern void BBoxIterateVerticesAtLocation2A(void *vertex_data, BBOX_VERTEX_OCTREE *vertices,
					    void (*routine)(BBOX_VERTEX *vertex, void *parm1, void *parm2),
					    void *parm1, void *parm2);

/* Iterator over all edge-wings between vertices at the locations specified by 
 * v1data and v2data. */
extern void BBoxIterateWingsBetweenLocations(void *v1data, void *v2data,
					     BBOX_VERTEX_OCTREE *vertices,
					     void (*func)(BBOX_WING *));

/* Iterator over all edge-wings between vertices at the locations specified by 
 * v1data and v2data. */
extern void BBoxIterateWingsBetweenLocations1A(void *v1data, void *v2data,
					       BBOX_VERTEX_OCTREE *vertices,
					       void (*func)(BBOX_WING *),
					       void *parm);

#ifdef __cplusplus
}
#endif

#endif /* _BBOX_H_ */
