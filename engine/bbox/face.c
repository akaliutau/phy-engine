

#include "bbox.h"
#include "private.h"
#include "pools.h"

#ifndef NOPOOLS
static POOL *facePool = (POOL *)NULL;
#define NEWFACE()  	(BBOX_FACE *)NewPoolCell(sizeof(BBOX_FACE), 0, "brep faces", &facePool)
#define DISPOSEFACE(ptr) Dispose((unsigned char *)(ptr), &facePool)
#else 
#define NEWFACE()	(BBOX_FACE *)Alloc(sizeof(BBOX_FACE))
#define DISPOSEFACE(ptr) Free((char *)ptr, sizeof(BBOX_FACE))
#endif 


static BBOX_CALLBACK_FUNC brep_close_face_callback = (BBOX_CALLBACK_FUNC)NULL,
                          brep_destroy_face_callback = (BBOX_CALLBACK_FUNC)NULL,
                          brep_create_face_callback = (BBOX_CALLBACK_FUNC)NULL;


BBOX_CALLBACK_FUNC BBoxSetCreateFaceCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_create_face_callback;
  brep_create_face_callback = func;
  return oldfunc;
}


BBOX_CALLBACK_FUNC BBoxSetCloseFaceCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_close_face_callback;
  brep_close_face_callback = func;
  return oldfunc;
}


BBOX_CALLBACK_FUNC BBoxSetDestroyFaceCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_destroy_face_callback;
  brep_destroy_face_callback = func;
  return oldfunc;
}


void BBoxConnectFaceToShell(BBOX_FACE *face, BBOX_SHELL *shell)
{
  face->shell = shell;
  if (!shell)
    return;

  if (!shell->faces) { 
    shell->faces = face;
    face->next = face->prev = face;
  } else {		
    face->next = shell->faces;
    face->prev = shell->faces->prev;
    face->next->prev = face->prev->next = face;
  }
}


BBOX_FACE *BBoxCreateFace(BBOX_SHELL *shell, void *client_data)
{
  BBOX_FACE *face;

  face = NEWFACE();
  face->outer_contour = (BBOX_CONTOUR *)NULL;

  BBoxConnectFaceToShell(face, shell);

  
  face->client_data = client_data;
  if (brep_create_face_callback)
    face->client_data = brep_create_face_callback(face);

  return face;
}


void BBoxCloseFace(BBOX_FACE *face)
{
  
  BBoxFaceIterateContours(face, BBoxCloseContour);

  
  if (brep_close_face_callback)
    face->client_data = brep_close_face_callback(face);
}


void BBoxFaceIterateContours(BBOX_FACE *face, void (*func)(BBOX_CONTOUR *))
{
  BBoxIterate((BBOX_RING *)face->outer_contour, (void (*)(BBOX_RING *))func);
}

void BBoxFaceIterateContours1A(BBOX_FACE *face, void (*func)(BBOX_CONTOUR *, void *), void *parm)
{
  BBoxIterate1A((BBOX_RING *)face->outer_contour, (void (*)(BBOX_RING *, void *))func, parm);
}

void BBoxFaceIterateContours2A(BBOX_FACE *face, void (*func)(BBOX_CONTOUR *, void *, void *), void *parm1, void *parm2)
{
  BBoxIterate2A((BBOX_RING *)face->outer_contour, (void (*)(BBOX_RING *, void *, void *))func, parm1, parm2);
}


void BBoxDisconnectFaceFromShell(BBOX_FACE *face)
{
  BBOX_SHELL *shell = face->shell;

  if (!shell)			
    return;

  if (shell->faces == face) {	
    if (face->next == face)	
      shell->faces = (BBOX_FACE *)NULL;

    else			
      shell->faces = face->next;
  } 

  face->next->prev = face->prev;
  face->prev->next = face->next;

  face->shell = (BBOX_SHELL *)NULL;
}


static void BBoxFaceDestroyContours(BBOX_CONTOUR *first)
{
  BBOX_CONTOUR *contour, *prev;

  if (first) {
    for (contour = first->prev; contour != first; contour = prev) {
      prev = contour->prev;
      BBoxDestroyContour(contour);
    }
    BBoxDestroyContour(first);
  }
}


void BBoxDestroyFace(BBOX_FACE *face)
{
  

  
  BBoxDisconnectFaceFromShell(face);
  
  
  if (brep_destroy_face_callback)
    brep_destroy_face_callback(face);

  
  BBoxFaceDestroyContours(face->outer_contour);

  
  DISPOSEFACE(face);
}
