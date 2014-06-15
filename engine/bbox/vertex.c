

#include "bbox.h"
#include "private.h"
#include "pools.h"

#ifndef NOPOOLS

static POOL *vertexPool = (POOL *)NULL;
#define NEWVERTEX()  	(BBOX_VERTEX *)NewPoolCell(sizeof(BBOX_VERTEX), 0, "brep vertices", &vertexPool)
#define DISPOSEVERTEX(ptr) Dispose((unsigned char *)(ptr), &vertexPool)

static POOL *ringelPool = (POOL *)NULL;
#define NEWRINGEL()  	(BBOX_WING_RING *)NewPoolCell(sizeof(BBOX_WING_RING), 0, "brep vertex wing rings", &ringelPool)
#define DISPOSERINGEL(ptr) Dispose((unsigned char *)(ptr), &ringelPool)

#else 

#define NEWVERTEX()	(BBOX_VERTEX *)Alloc(sizeof(BBOX_VERTEX))
#define DISPOSEVERTEX(ptr) Free((char *)ptr, sizeof(BBOX_VERTEX))

#define NEWRINGEL()	(BBOX_WING_RING *)Alloc(sizeof(BBOX_WING_RING))
#define DISPOSERINGEL(ptr) Free((char *)ptr, sizeof(BBOX_WING_RING))

#endif 


static BBOX_CALLBACK_FUNC brep_create_vertex_callback = (BBOX_CALLBACK_FUNC)NULL,
                          brep_close_vertex_callback = (BBOX_CALLBACK_FUNC)NULL,
                          brep_destroy_vertex_callback = (BBOX_CALLBACK_FUNC)NULL;


BBOX_CALLBACK_FUNC BBoxSetCreateVertexCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_create_vertex_callback;
  brep_create_vertex_callback = func;
  return oldfunc;
}


BBOX_CALLBACK_FUNC BBoxSetCloseVertexCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_close_vertex_callback;
  brep_close_vertex_callback = func;
  return oldfunc;
}


BBOX_CALLBACK_FUNC BBoxSetDestroyVertexCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_destroy_vertex_callback;
  brep_destroy_vertex_callback = func;
  return oldfunc;
}


BBOX_VERTEX *BBoxCreateVertex(void *client_data)
{
  BBOX_VERTEX *vertex;

  vertex = NEWVERTEX();
  vertex->wing_ring = (BBOX_WING_RING *)NULL;

  vertex->client_data = client_data;  
  if (brep_create_vertex_callback)
    vertex->client_data = brep_create_vertex_callback(vertex);

  
  if (brep_close_vertex_callback)
    vertex->client_data = brep_close_vertex_callback(vertex);

  return vertex;
}


static void BBoxConnectWingRingToVertex(BBOX_WING_RING *ringel, BBOX_VERTEX *vertex)
{
  if (!vertex->wing_ring) {	
    ringel->next = ringel->prev = ringel;
    vertex->wing_ring = ringel;
  } else {			  
    ringel->next = vertex->wing_ring;
    ringel->prev = vertex->wing_ring->prev;
    ringel->next->prev = ringel->prev->next = ringel;
  }
}


void BBoxConnectWingToVertex(BBOX_WING *wing)
{
  BBOX_VERTEX *vertex = wing->vertex;
  BBOX_WING_RING *ringel;

  ringel = NEWRINGEL();
  ringel->wing = wing;

  BBoxConnectWingRingToVertex(ringel, vertex);
}


BBOX_WING_RING *BBoxFindWingLeavingVertex(BBOX_WING *wing)
{
  BBOX_VERTEX *vertex = wing->vertex;
  BBOX_WING_RING *ringel, *next;
  
  if (vertex->wing_ring) {
    next = vertex->wing_ring;
    do {
      ringel = next;
      next = ringel->next;
      if (ringel->wing == wing)
	return ringel;
    } while (next && next != vertex->wing_ring);
  }

  return (BBOX_WING_RING *)NULL;
}


static void BBoxDisconnectWingRingFromVertex(BBOX_WING_RING *ringel)
{
  BBOX_VERTEX *vertex = ringel->wing->vertex;

  if (vertex->wing_ring == ringel) {
    if (ringel->next == ringel)		
      vertex->wing_ring = (BBOX_WING_RING *)NULL;
    else
      vertex->wing_ring = ringel->next;
  }

  ringel->next->prev = ringel->prev;
  ringel->prev->next = ringel->next;
}


void BBoxDisconnectWingFromVertex(BBOX_WING *wing)
{
  BBOX_WING_RING *ringel;

  
  ringel = BBoxFindWingLeavingVertex(wing);
  if (!ringel) {
    BBoxError(wing->edge->client_data, "BBoxDisconnectWingFromVertex", "wing not connected to the vertex");
    return;
  }

  
  BBoxDisconnectWingRingFromVertex(ringel);

  
  DISPOSERINGEL(ringel);
}


BBOX_EDGE *BBoxFindEdge(BBOX_VERTEX *vertex1, BBOX_VERTEX *vertex2)
{
  BBOX_WING_RING *ringel, *next;

  
  if (vertex2->wing_ring) {
    next = vertex2->wing_ring;
    do {
      ringel = next;
      next = ringel->next;
      if (BBoxEdgeOtherWing(ringel->wing)->vertex == vertex1)
	return ringel->wing->edge;
    } while (next && next != vertex2->wing_ring);
  } 

  
  if (vertex1->wing_ring) {
    next = vertex1->wing_ring;
    do {
      ringel = next;
      next = ringel->next;
      if (BBoxEdgeOtherWing(ringel->wing)->vertex == vertex2)
	return ringel->wing->edge;
    } while (next && next != vertex1->wing_ring);
  } 

  
  return (BBOX_EDGE *)NULL;
}


void BBoxIterateWingsWithVertex(BBOX_VERTEX *vertex1, BBOX_VERTEX *vertex2, 
				void (*func)(BBOX_WING *))
{
  BBOX_WING_RING *ringel, *next;

  if (vertex2->wing_ring) {
    next = vertex2->wing_ring;
    do {
      ringel = next;
      next = ringel->next;
      if (ringel->wing->contour && ringel->wing->next->vertex == vertex1)
	func(ringel->wing);
    } while (next && next != vertex2->wing_ring);
  }

  if (vertex1->wing_ring) {
    next = vertex1->wing_ring;
    do {
      ringel = next;
      next = ringel->next;
      if (ringel->wing->contour && ringel->wing->next->vertex == vertex2)
	func(ringel->wing);
    } while (next && next != vertex1->wing_ring);
  }
}


void BBoxIterateWingsWithVertex1A(BBOX_VERTEX *vertex1, BBOX_VERTEX *vertex2, 
				  void (*func)(BBOX_WING *, void *parm), 
				  void *parm)
{
  BBOX_WING_RING *ringel, *next;

  if (vertex2->wing_ring) {
    next = vertex2->wing_ring;
    do {
      ringel = next;
      next = ringel->next;
      if (ringel->wing->contour && ringel->wing->next->vertex == vertex1)
	func(ringel->wing, parm);
    } while (next && next != vertex2->wing_ring);
  }

  if (vertex1->wing_ring) {
    next = vertex1->wing_ring;
    do {
      ringel = next;
      next = ringel->next;
      if (ringel->wing->contour && ringel->wing->next->vertex == vertex2)
	func(ringel->wing, parm);
    } while (next && next != vertex1->wing_ring);
  }
}


BBOX_WING *BBoxNextWingLeavingVertex(BBOX_WING *wing)
{
  BBOX_WING *next_wing;

  if (!wing->prev)  
    return (BBOX_WING *)NULL;

  next_wing = BBoxEdgeOtherWing(wing->prev);

  if (next_wing->contour)
    return next_wing;
  else
    return (BBOX_WING *)NULL;
}


static void do_ringel_wing(BBOX_WING_RING *ringel, void (*func)(BBOX_WING *))
{
  func(ringel->wing);
}

void BBoxVertexIterateWings(BBOX_VERTEX *vertex, void (*func)(BBOX_WING *))
{
  BBoxIterate1A((BBOX_RING *)vertex->wing_ring, (void (*)(BBOX_RING *, void *))do_ringel_wing, (void *)func);
}

#ifdef NEVER
static void do_ringel_wing_parm(BBOX_WING_RING *ringel, void (*func)(BBOX_WING *, void *), void *parm)
{
  func(ringel->wing, parm);
}
#endif

void BBoxVertexIterateWings1A(BBOX_VERTEX *vertex, void (*func)(BBOX_WING *, void *), void *parm)
{
  BBoxIterate2A((BBOX_RING *)vertex->wing_ring, (void (*)(BBOX_RING *, void *, void *))do_ringel_wing, (void *)func, parm);
}


void BBoxDestroyVertex(BBOX_VERTEX *vertex)
{
  
  if (vertex->wing_ring) {
    BBoxInfo(vertex->client_data, "BBoxDestroyVertex", "vertex still being used in edges, will not be destroyed"); 
    return;
  }

  
  if (brep_destroy_vertex_callback)
    brep_destroy_vertex_callback(vertex);

  
  DISPOSEVERTEX(vertex);
}


void BBoxMoveWingRingToVertex(BBOX_WING_RING *ringel, BBOX_VERTEX *newvertex)
{
  
  BBoxDisconnectWingRingFromVertex(ringel);

  
  BBoxConnectWingRingToVertex(ringel, newvertex);
}

