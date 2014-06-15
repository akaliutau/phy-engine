

#include "bbox.h"
#include "private.h"
#include "pools.h"

#ifndef NOPOOLS
static POOL *contourPool = (POOL *)NULL;
#define NEWCONTOUR()  	(BBOX_CONTOUR *)NewPoolCell(sizeof(BBOX_CONTOUR), 0, "brep contours", &contourPool)
#define DISPOSECONTOUR(ptr) Dispose((unsigned char *)(ptr), &contourPool)
#else 
#define NEWCONTOUR()	(BBOX_CONTOUR *)Alloc(sizeof(BBOX_CONTOUR))
#define DISPOSECONTOUR(ptr) Free((char *)ptr, sizeof(BBOX_CONTOUR))
#endif 


static BBOX_CALLBACK_FUNC brep_close_contour_callback = (BBOX_CALLBACK_FUNC)NULL,
                          brep_destroy_contour_callback = (BBOX_CALLBACK_FUNC)NULL,
                          brep_create_contour_callback = (BBOX_CALLBACK_FUNC)NULL,
                          brep_update_contour_callback = (BBOX_CALLBACK_FUNC)NULL;


BBOX_CALLBACK_FUNC BBoxSetCreateContourCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_create_contour_callback;
  brep_create_contour_callback = func;
  return oldfunc;
}


BBOX_CALLBACK_FUNC BBoxSetCloseContourCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_close_contour_callback;
  brep_close_contour_callback = func;
  return oldfunc;
}


BBOX_CALLBACK_FUNC BBoxSetUpdateContourCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_update_contour_callback;
  brep_update_contour_callback = func;
  return oldfunc;
}


BBOX_CALLBACK_FUNC BBoxSetDestroyContourCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_destroy_contour_callback;
  brep_destroy_contour_callback = func;
  return oldfunc;
}


void BBoxConnectContourToFace(BBOX_CONTOUR *contour, BBOX_FACE *face)
{
  contour->face = face;
  if (!face->outer_contour) { 	
    face->outer_contour = contour;
    contour->next = contour->prev = contour;
  } else {			
    contour->next = face->outer_contour;
    contour->prev = face->outer_contour->prev;
    contour->next->prev = contour->prev->next = contour;
  }
}


BBOX_CONTOUR *BBoxCreateContour(BBOX_FACE *face, void *client_data)
{
  BBOX_CONTOUR *contour;

  contour = NEWCONTOUR();
  contour->wings = (BBOX_WING *)NULL;

  BBoxConnectContourToFace(contour, face);

  
  contour->client_data = client_data;
  if (brep_create_contour_callback)
    contour->client_data = brep_create_contour_callback(contour);

  return contour;
}

void BBoxCloseContour(BBOX_CONTOUR *contour)
{
  
  if (brep_close_contour_callback)
    contour->client_data = brep_close_contour_callback(contour);
}


void BBoxUpdateContour(BBOX_CONTOUR *contour)
{
  
  if (brep_update_contour_callback)
    contour->client_data = brep_update_contour_callback(contour);

  
}


void BBoxContourIterateWings(BBOX_CONTOUR *contour, void (*func)(BBOX_WING *))
{
  BBoxIterate((BBOX_RING *)contour->wings, (void (*)(BBOX_RING *))func);
}

void BBoxContourIterateWings1A(BBOX_CONTOUR *contour, void (*func)(BBOX_WING *, void *), void *parm)
{
  BBoxIterate1A((BBOX_RING *)contour->wings, (void (*)(BBOX_RING *, void *))func, parm);
}

void BBoxContourIterateWings2A(BBOX_CONTOUR *contour, void (*func)(BBOX_WING *, void *, void *), void *parm1, void *parm2)
{
  BBoxIterate2A((BBOX_RING *)contour->wings, (void (*)(BBOX_RING *, void *, void *))func, parm1, parm2);
}


static void do_vertex_func(BBOX_WING *wing, void (*func)(BBOX_VERTEX *))
{
  func(wing->vertex);
}

void BBoxContourIterateVertices(BBOX_CONTOUR *contour, void (*func)(BBOX_VERTEX *))
{
  BBoxContourIterateWings1A(contour, (void (*)(BBOX_WING *, void *))do_vertex_func, (void *)func);
}

static void do_vertex_func_parm(BBOX_WING *wing, void (*func)(BBOX_VERTEX *, void *), void *parm)
{
  func(wing->vertex, parm);
}

void BBoxContourIterateVertices1A(BBOX_CONTOUR *contour, void (*func)(BBOX_VERTEX *, void *), void *parm)
{
  BBoxContourIterateWings2A(contour, (void (*)(BBOX_WING *, void *, void *))do_vertex_func_parm, (void *)func, parm);
}


void BBoxContourRemoveVertex(BBOX_CONTOUR *contour, BBOX_VERTEX *vertex)
{
  BBOX_WING *wing;

  
  if (!contour->wings) {
    BBoxError(contour->client_data, "BBoxContourRemoveVertex", "empty contour");
    return;
  }

  
  if (contour->wings->next == contour->wings) {
    if (contour->wings->vertex != vertex) {
      BBoxError(contour->client_data, "BBoxContourRemoveVertex", "vertex not belonging to contour or contour is not closed");
      return;
    } else
      BBoxDestroyWing(contour->wings);
  }

  

  
  wing = contour->wings->prev;
  while (wing->vertex != vertex && wing != contour->wings)
    wing = wing->prev;
  
  if (wing->vertex != vertex) 
    BBoxError(contour->client_data, "BBoxContourRemoveVertex", "vertex doesn't occur in contour");
  else
    BBoxJoinWings(wing->prev, wing);
}


void BBoxDisconnectContourFromFace(BBOX_CONTOUR *contour)
{
  BBOX_FACE *face = contour->face;

  if (face->outer_contour == contour) {	
    if (contour->next == contour)	
      face->outer_contour = (BBOX_CONTOUR *)NULL;

    else {				
      
      face->outer_contour = contour->next;
    }
  } 

  contour->next->prev = contour->prev;
  contour->prev->next = contour->next;

  contour->face = (BBOX_FACE *)NULL;
}


static void BBoxContourDestroyWings(BBOX_WING *first)
{
  BBOX_WING *wing, *prev;

  if (first) {
    for (wing = first->prev; wing != first; wing = prev) {
      prev = wing->prev;
      BBoxDestroyWing(wing);
    }
    BBoxDestroyWing(first);
  }
}


void BBoxDestroyContour(BBOX_CONTOUR *contour)
{
  

  
  BBoxDisconnectContourFromFace(contour);
  
  
  if (brep_destroy_contour_callback)
    brep_destroy_contour_callback(contour);

  
  BBoxContourDestroyWings(contour->wings);

  
  DISPOSECONTOUR(contour);  
}


BBOX_WING *BBoxMakeEdgeSplitContour(BBOX_WING *wing1, BBOX_WING *wing2,
				    void *edge_data, void *contour_data)
{
  BBOX_EDGE *edge;
  BBOX_WING *winga, *wingb, *wing;

  
  if (wing1->contour != wing2->contour) {
    BBoxError(wing1->edge->client_data, "BBoxMakeEdgeSplitContour", "wings must belong to the same contour");
    return (BBOX_WING *)NULL;
  }

  
  edge = BBoxCreateEdge(wing1->vertex, wing2->vertex, edge_data);
  winga = &(edge->wing[0]);
  wingb = &(edge->wing[1]);
  
  
  winga->prev = wing1->prev;
  winga->next = wing2;
  wingb->prev = wing2->prev;
  wingb->next = wing1;
  winga->prev->next = winga->next->prev = winga;
  wingb->prev->next = wingb->next->prev = wingb;
  winga->contour = wingb->contour = wing1->contour;

  
  wing1->contour->wings = wing1;

  
  wing2->contour = BBoxCreateContour(wing1->contour->face, contour_data);
  wing2->contour->wings = wing2;

  
  wing = wing2;
  do {
    wing->contour = wing2->contour;
    wing = wing->next;
  } while (wing != wing2);

  
  BBoxConnectWingToVertex(winga);
  BBoxConnectWingToVertex(wingb);

  
  BBoxCloseEdge(edge);

  
  
  BBoxCloseContour(wing2->contour);
  BBoxCloseContour(wing1->contour);

  
  return winga;
}


BBOX_WING *BBoxMakeEdgeJoinContours(BBOX_WING *wing1, BBOX_WING *wing2, void *edge_data)
{
  BBOX_EDGE *edge;
  BBOX_WING *winga, *wingb, *wing;

  
  if (wing1->contour == wing2->contour) {
    BBoxError(wing1->edge->client_data, "BBoxMakeEdgeJoinContours", "wings must belong to a different contour");
    return (BBOX_WING *)NULL;
  }

  
  if (wing1->contour->face != wing2->contour->face) {
    BBoxWarning(wing1->edge->client_data, "BBoxMakeEdgeJoinContours", "joining contours from different faces");
    
  }

  
  edge = BBoxCreateEdge(wing1->vertex, wing2->vertex, edge_data);
  winga = &(edge->wing[0]);
  wingb = &(edge->wing[1]);
  
  
  winga->prev = wing1->prev;
  winga->next = wing2;
  wingb->prev = wing2->prev;
  wingb->next = wing1;
  winga->prev->next = winga->next->prev = winga;
  wingb->prev->next = wingb->next->prev = wingb;

  
  if (wing2->contour != wing2->contour->face->outer_contour) {
    winga->contour = wingb->contour = wing1->contour;

    
    wing2->contour->wings = (BBOX_WING *)NULL;
    BBoxDestroyContour(wing2->contour);

    
    for (wing = wing2; wing->contour != wing1->contour; wing = wing->next)
      wing->contour = wing1->contour;
  } else {
    winga->contour = wingb->contour = wing2->contour;

    
    wing1->contour->wings = (BBOX_WING *)NULL;
    BBoxDestroyContour(wing1->contour);

    
    for (wing = wing1; wing->contour != wing2->contour; wing = wing->next)
      wing->contour = wing2->contour;
  }

  
  BBoxConnectWingToVertex(winga);
  BBoxConnectWingToVertex(wingb);

  
  BBoxCloseEdge(edge);

  
  
  BBoxCloseContour(winga->contour);

  
  return winga;
}


BBOX_WING *BBoxDeleteEdgeJoinContours(BBOX_WING *wing)
{
  BBOX_WING *owing = BBoxEdgeOtherWing(wing);
  BBOX_WING *prev_wing = wing->prev, *next_wing = wing->next;
  BBOX_CONTOUR *contour = wing->contour, *ocontour = owing->contour;

  if (!ocontour || ocontour == wing->contour) {
    BBoxWarning(wing->edge->client_data, "BBoxDeleteEdgeJoinContours", "Edge not enclosed in two different contours");
    return wing;
  }

  if (!prev_wing || !next_wing || !contour) {
    BBoxError(wing->edge->client_data, "BBoxDeleteEdgeJoinContours", "wing not properly connected in contour");
    return wing;
  }

  if (wing->prev == wing) {
    
    if (owing->next == owing)
      BBoxDestroyContour(ocontour);
    else
      BBoxDestroyWing(owing);

    
    BBoxDestroyContour(contour);

    return (BBOX_WING *)NULL;
  }

  
  BBoxDestroyWing(wing);

  if (owing->next != owing) {
    
    BBOX_WING *wing1 = owing->next, *wing2 = owing->prev;
    owing->next = owing->prev = owing;
    ocontour->wings = owing;	

    prev_wing->next = wing1;
    wing1->prev = prev_wing;

    next_wing->prev = wing2;
    wing2->next = next_wing;

    for (wing=wing1; wing!=wing2; wing=wing->next)
      wing->contour = contour;
    wing2->contour = contour;
  }

  
  BBoxDestroyContour(ocontour);

  
  BBoxCloseContour(contour);

  
  return prev_wing;
}


BBOX_WING *BBoxDeleteEdgeSplitContour(BBOX_WING *wing, void *contour_data)
{
  BBOX_WING *owing;

  if (!wing || !wing->contour || BBoxEdgeOtherWing(wing)->contour != wing->contour) {
    BBoxWarning(wing->edge->client_data, "BBoxDeleteEdgeSplitContour", "argument should be a seam");
    return wing;
  }

  owing = BBoxEdgeOtherWing(wing);
  if (owing == wing->next || owing == wing->prev) {
    
    if (wing->next->next == wing) {
      
      BBoxDestroyContour(wing->contour);
      return (BBOX_WING *)NULL;
    } else {
      
      BBOX_WING *prevwing = (owing->next == wing) ? owing->prev : wing->prev;
      BBoxDestroyWing(wing);
      BBoxDestroyWing(owing);
      BBoxCloseContour(prevwing->contour);
      return prevwing;
    }
  } else {
    BBOX_CONTOUR *contour = wing->contour, *split_contour;
    BBOX_WING *first1 = owing->next, *last1 = wing->prev,
              *first2 = wing->next, *last2 = owing->prev;

    
    BBoxDestroyWing(wing);
    BBoxDestroyWing(owing);

    
    first1->prev = last1;
    last1->next = first1;
    contour->wings = last1;
    BBoxCloseContour(contour);

    
    split_contour = BBoxCreateContour(contour->face, contour_data);
    first2->prev = last2;
    last2->next = first2;
    split_contour->wings = first2;
    ForAllWingsInContour(w, split_contour) {
      w->contour = split_contour;
    } EndForAll;
    BBoxCloseContour(split_contour);

    return last1;
  }

  return wing;	
}


BBOX_WING *BBoxMakeSlit(BBOX_FACE *face, BBOX_VERTEX *v1, BBOX_VERTEX *v2,
			void *edge_data, void *contour_data)
{
  BBOX_EDGE *edge;
  BBOX_WING *winga, *wingb;
  BBOX_CONTOUR *contour;

  
  contour = BBoxCreateContour(face, contour_data);

  
  edge = BBoxCreateEdge(v1, v2, edge_data);
  winga = &(edge->wing[0]);
  wingb = &(edge->wing[1]);
  
  
  winga->prev = winga->next = wingb;
  wingb->prev = wingb->next = winga;
  winga->contour = wingb->contour = contour;
  contour->wings = winga;

  
  BBoxConnectWingToVertex(winga);
  BBoxConnectWingToVertex(wingb);

  
  BBoxCloseEdge(edge);

  
  BBoxCloseContour(contour);

  
  return winga;
}


BBOX_WING *BBoxMakeNotch(BBOX_WING *wing, BBOX_VERTEX *vertex, void *edge_data)
{  
  BBOX_EDGE *edge;
  BBOX_WING *winga, *wingb;

  
  edge = BBoxCreateEdge(wing->vertex, vertex, edge_data);
  winga = &(edge->wing[0]);
  wingb = &(edge->wing[1]);
  
  
  winga->prev = wing->prev;
  winga->next = wingb;
  wingb->prev = winga;
  wingb->next = wing;
  winga->prev->next = winga->next->prev = winga;
  wingb->prev->next = wingb->next->prev = wingb;
  winga->contour = wingb->contour = wing->contour;

  
  BBoxConnectWingToVertex(winga);
  BBoxConnectWingToVertex(wingb);

  
  BBoxCloseEdge(edge);

  
  

  
  return winga;
}

