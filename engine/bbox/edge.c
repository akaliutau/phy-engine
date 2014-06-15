

#include "bbox.h"
#include "private.h"
#include "pools.h"

#ifndef NOPOOLS
static POOL *edgePool = (POOL *)NULL;
#define NEWEDGE()  	(BBOX_EDGE *)NewPoolCell(sizeof(BBOX_EDGE), 0, "brep edges", &edgePool)
#define DISPOSEEDGE(ptr) Dispose((unsigned char *)(ptr), &edgePool)
#else 
#define NEWEDGE()	(BBOX_EDGE *)Alloc(sizeof(BBOX_EDGE))
#define DISPOSEEDGE(ptr) Free((char *)ptr, sizeof(BBOX_EDGE))
#endif 


static BBOX_CALLBACK_FUNC brep_create_edge_callback = (BBOX_CALLBACK_FUNC)NULL,
                          brep_close_edge_callback = (BBOX_CALLBACK_FUNC)NULL,
                          brep_destroy_edge_callback = (BBOX_CALLBACK_FUNC)NULL;


BBOX_CALLBACK_FUNC BBoxSetCreateEdgeCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_create_edge_callback;
  brep_create_edge_callback = func;
  return oldfunc;
}


BBOX_CALLBACK_FUNC BBoxSetCloseEdgeCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_close_edge_callback;
  brep_close_edge_callback = func;
  return oldfunc;
}


BBOX_CALLBACK_FUNC BBoxSetDestroyEdgeCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_destroy_edge_callback;
  brep_destroy_edge_callback = func;
  return oldfunc;
}


BBOX_EDGE *BBoxCreateEdge(BBOX_VERTEX *vertex1, BBOX_VERTEX *vertex2, void *client_data)
{
  BBOX_EDGE *edge;

  edge = NEWEDGE();
  edge->wing[0].edge = edge->wing[1].edge = edge;

  edge->wing[0].vertex = vertex1;
  edge->wing[1].vertex = vertex2;

  
  edge->wing[0].contour = edge->wing[1].contour = (BBOX_CONTOUR *)NULL;
  edge->wing[0].prev = edge->wing[0].next = 
  edge->wing[1].prev = edge->wing[1].next = (BBOX_WING *)NULL;
  
  
  edge->client_data = client_data;
  if (brep_create_edge_callback)
    edge->client_data = brep_create_edge_callback(edge);

  return edge;
}


BBOX_WING *BBoxCreateWingWithoutContour(BBOX_VERTEX *vertex1, BBOX_VERTEX *vertex2, void *client_data)
{
  BBOX_EDGE *edge;
  BBOX_WING *wing;

  
  edge = BBoxFindEdge(vertex1, vertex2);
  if (!edge) {	
    
    edge = BBoxCreateEdge(vertex1, vertex2, client_data);

    
    wing = &edge->wing[0];
  } else {	
    if (edge->wing[0].vertex == vertex1)	
      wing = &(edge->wing[0]);
    else if (edge->wing[1].vertex == vertex1)	
      wing = &(edge->wing[1]);
    else {					
      BBoxFatal(edge->client_data, "BBoxCreateWingWithoutContour", "something impossible wrong here");
      return (BBOX_WING *)NULL;  
    }
      
    
    if (wing->contour) {
      BBoxInfo(edge->client_data, "BBoxCreateWing", "attempt to use an edge two times in the same sense - creating a duplicate edge");
      edge = BBoxCreateEdge(vertex1, vertex2, client_data);
      wing = &edge->wing[0];
    }
  }

  return wing;
}


void BBoxConnectWingToContour(BBOX_WING *wing, BBOX_CONTOUR *contour)
{
  wing->contour = contour;

  if (!contour->wings) {	
    wing->next = wing->prev = wing;
    contour->wings = wing;
  } else {			
    wing->next = contour->wings;
    wing->prev = contour->wings->prev;
    wing->prev->next = wing->next->prev = wing;
  }
}


void BBoxCloseEdge(BBOX_EDGE *edge)
{
  if (brep_close_edge_callback)
    edge->client_data = brep_close_edge_callback(edge);
}


BBOX_WING *BBoxCreateWing(BBOX_VERTEX *vertex1, BBOX_VERTEX *vertex2, BBOX_CONTOUR *contour, void *client_data)
{
  BBOX_WING *wing;

  if (contour->wings && BBoxEdgeOtherWing(contour->wings->prev)->vertex != vertex1) {
    BBoxError(contour->client_data, "BBoxCreateWing", "the starting point of a new wing should be the endpoint of the previous wing");
    return (BBOX_WING *)NULL;
  }

  
  wing = BBoxCreateWingWithoutContour(vertex1, vertex2, client_data);

  
  if (wing->vertex != vertex1) 
      BBoxError(wing->edge->client_data, "BBoxCreateWing", "edge should be used in opposite sense by the contours sharing it");

  
  BBoxConnectWingToContour(wing, contour);

  
  BBoxConnectWingToVertex(wing);

  
  BBoxCloseEdge(wing->edge);

  return wing;
}


BBOX_WING *BBoxSplitWing(BBOX_WING *wing, BBOX_VERTEX *vertex)
{
  BBOX_WING *wing1, *wing2;

  
  wing1 = BBoxCreateWingWithoutContour(wing->vertex, vertex, NULL);
  wing2 = BBoxCreateWingWithoutContour(vertex, wing->next->vertex, NULL);
  
      
  wing1->contour = wing2->contour = wing->contour;
  
  wing1->next = wing2;
  wing2->prev = wing1;

  if (wing->prev != wing) {	
    wing1->prev = wing->prev;
    wing2->next = wing->next;
  } else {			
    wing1->prev = wing2;
    wing2->next = wing1;
  }

  
  BBoxDestroyWing(wing);
  
  
  wing1->prev->next = wing1;
  wing2->next->prev = wing2;

  
  if (!wing1->contour->wings)
    wing1->contour->wings = wing1;

  
  BBoxConnectWingToVertex(wing1);
  BBoxConnectWingToVertex(wing2);  
  
  
  BBoxCloseEdge(wing1->edge);
  BBoxCloseEdge(wing2->edge);

  return wing2;
}


void BBoxSplitEdge(BBOX_EDGE *edge, BBOX_VERTEX *vertex)
{
  int do_wing0, do_wing1;

  do_wing0 = (edge->wing[0].contour != (BBOX_CONTOUR *)NULL);
  do_wing1 = (edge->wing[1].contour != (BBOX_CONTOUR *)NULL);
  if (do_wing0)
    BBoxSplitWing(&(edge->wing[0]), vertex);
  if (do_wing1)
    BBoxSplitWing(&(edge->wing[1]), vertex);
}


BBOX_WING *BBoxJoinWings(BBOX_WING *wing1, BBOX_WING *wing2)
{
  BBOX_WING *wing;

  
  wing = BBoxCreateWingWithoutContour(wing1->vertex, wing2->next->vertex, NULL);
    
  
  wing->contour = wing1->contour;

  if (wing1->prev != wing2) {	
    wing->prev = wing1->prev;
    wing->next = wing2->next;
  } else {			
    wing->prev = wing->next = wing;      
  }

  
  BBoxDestroyWing(wing1);
  BBoxDestroyWing(wing2);
    
  
  wing->prev->next = wing;
  wing->next->prev = wing;
  
  
  if (!wing->contour->wings)
    wing->contour->wings = wing;

  
  BBoxConnectWingToVertex(wing);

  
  BBoxCloseEdge(wing->edge);

  return wing;
}


void BBoxJoinEdges(BBOX_EDGE *edge1, BBOX_EDGE *edge2)
{
  int do_wing0, do_wing1;

  do_wing0 = (edge1->wing[0].contour != (BBOX_CONTOUR *)NULL);
  do_wing1 = (edge1->wing[1].contour != (BBOX_CONTOUR *)NULL);

  
  if (do_wing0) {
    if (edge1->wing[0].next == &(edge2->wing[0]))
      BBoxJoinWings(&(edge1->wing[0]), &(edge2->wing[0]));
    else if (edge1->wing[0].next == &(edge2->wing[1]))
      BBoxJoinWings(&(edge1->wing[0]), &(edge2->wing[1]));
    else if (edge1->wing[0].prev == &(edge2->wing[0]))
      BBoxJoinWings(&(edge2->wing[0]), &(edge1->wing[0]));
    else if (edge1->wing[0].prev == &(edge2->wing[1]))
      BBoxJoinWings(&(edge2->wing[1]), &(edge1->wing[0]));
    else
      BBoxError(edge1->client_data, "BBoxJoinEdges", "the two edges don't fit on the first side");
  }

  
  if (do_wing1) {
    if (edge1->wing[1].next == &(edge2->wing[0]))
      BBoxJoinWings(&(edge1->wing[1]), &(edge2->wing[0]));
    else if (edge1->wing[1].next == &(edge2->wing[1]))
      BBoxJoinWings(&(edge1->wing[1]), &(edge2->wing[1]));
    else if (edge1->wing[1].prev == &(edge2->wing[0]))
      BBoxJoinWings(&(edge2->wing[0]), &(edge1->wing[1]));
    else if (edge1->wing[1].prev == &(edge2->wing[1]))
      BBoxJoinWings(&(edge2->wing[1]), &(edge1->wing[1]));
    else
      BBoxError(edge1->client_data, "BBoxJoinEdges", "the two edges don't fit on the second side");
  }
}


BBOX_WING *BBoxWingReplaceVertex(BBOX_WING *wing, BBOX_VERTEX *newvertex)
{
  BBOX_WING *wing1, *wing2;

  if (!wing->prev) {
    BBoxError(wing->edge->client_data, "BBoxWingReplaceVertex", "wing should be properly connected to a contour");
    return (BBOX_WING *)NULL;
  }

  if (wing->prev == wing) {
    BBoxError(wing->edge->client_data, "BBoxWingReplaceVertex", "wing should not be the only wing in the contour");
    return (BBOX_WING *)NULL;
  }

  
  wing1 = BBoxCreateWingWithoutContour(wing->prev->vertex, newvertex, NULL);
  wing2 = BBoxCreateWingWithoutContour(newvertex, wing->next->vertex, NULL);

  
  wing1->contour = wing2->contour = wing->contour;

  wing1->next = wing2;
  wing2->prev = wing1;

  if (wing->prev->prev != wing) {
    
    wing1->prev = wing->prev->prev;
    wing2->next = wing->next;
  } else {
    
    wing1->prev = wing2;
    wing2->next = wing1;
  }

  
  BBoxDestroyWing(wing->prev);
  BBoxDestroyWing(wing);

  
  wing1->prev->next = wing1;
  wing2->next->prev = wing2;

  
  if (!wing1->contour->wings)
    wing1->contour->wings = wing1;

  
  BBoxConnectWingToVertex(wing1);
  BBoxConnectWingToVertex(wing2);  
  
  
  BBoxCloseEdge(wing1->edge);
  BBoxCloseEdge(wing2->edge);

  return wing2;  
}


BBOX_WING *BBoxEdgeOtherWing(BBOX_WING *wing)
{
  return (wing == &(wing->edge->wing[0]) ? 
	  &(wing->edge->wing[1]) : 
	  &(wing->edge->wing[0])  );
}


void BBoxIterateWings(BBOX_WING *wings, void (*func)(BBOX_WING *))
{
  BBOX_WING *first_wing, *wing, *next;

  if (wings) {
    next = first_wing = wings;
    do {
      wing = next;
      next = wing->next;
      func(wing);
    } while (next != first_wing);
  }
}


void BBoxDestroyEdge(BBOX_EDGE *edge)
{
  
  if (edge->wing[0].contour || edge->wing[1].contour)
    return;

  
  if (brep_destroy_edge_callback)
    brep_destroy_edge_callback(edge);

  
  DISPOSEEDGE(edge);  
}


void BBoxDisconnectWingFromContour(BBOX_WING *wing)
{
  BBOX_CONTOUR *contour = wing->contour;

  if (!contour || !wing->next || !wing->prev) 
    BBoxError(wing->edge->client_data, "BBoxDisconnectWingFromContour", "wing improperly connected to contour");

  if (contour && contour->wings == wing) {	
    if (wing->next == wing)			
      contour->wings = (BBOX_WING *)NULL;
    else 		
      contour->wings = wing->next;
  }

  
  if (wing->next)
    wing->next->prev = wing->prev;
  if (wing->prev)
    wing->prev->next = wing->next;  

  wing->contour = (BBOX_CONTOUR *)NULL;
  wing->next = wing->prev = (BBOX_WING *)NULL;
}


void BBoxDestroyWing(BBOX_WING *wing)
{
  
  BBoxDisconnectWingFromVertex(wing);

  
  BBoxDisconnectWingFromContour(wing);

  
  BBoxDestroyEdge(wing->edge);
}

