

#include "bbox.h"
#include "private.h"
#include "pools.h"

#ifndef NOPOOLS
static POOL *solidPool = (POOL *)NULL;
#define NEWSOLID()  	(BBOX_SOLID *)NewPoolCell(sizeof(BBOX_SOLID), 0, "brep solids", &solidPool)
#define DISPOSESOLID(ptr) Dispose((unsigned char *)(ptr), &solidPool)
#else 
#define NEWSOLID()	(BBOX_SOLID *)Alloc(sizeof(BBOX_SOLID))
#define DISPOSESOLID(ptr) Free((char *)ptr, sizeof(BBOX_SOLID))
#endif 


static BBOX_CALLBACK_FUNC brep_close_solid_callback = (BBOX_CALLBACK_FUNC)NULL,
                          brep_destroy_solid_callback = (BBOX_CALLBACK_FUNC)NULL,
                          brep_create_solid_callback = (BBOX_CALLBACK_FUNC)NULL;


BBOX_CALLBACK_FUNC BBoxSetCreateSolidCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_create_solid_callback;
  brep_create_solid_callback = func;
  return oldfunc;
}


BBOX_CALLBACK_FUNC BBoxSetCloseSolidCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_close_solid_callback;
  brep_close_solid_callback = func;
  return oldfunc;
}


BBOX_CALLBACK_FUNC BBoxSetDestroySolidCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_destroy_solid_callback;
  brep_destroy_solid_callback = func;
  return oldfunc;
}


BBOX_SOLID *BBoxCreateSolid(void *client_data)
{
  BBOX_SOLID *solid;

  solid = NEWSOLID();
  solid->vertices = BBoxCreateVertexOctree();
  solid->shells = (BBOX_SHELL *)NULL;

  
  solid->client_data = client_data;
  if (brep_create_solid_callback)
    solid->client_data = brep_create_solid_callback(solid);

  return solid;
}


void BBoxCloseSolid(BBOX_SOLID *solid)
{
  
  BBoxSolidIterateShells(solid, BBoxCloseShell);

  
  if (brep_close_solid_callback)
    solid->client_data = brep_close_solid_callback(solid);
}


void BBoxSolidIterateShells(BBOX_SOLID *solid, void (*func)(BBOX_SHELL *))
{
  BBoxIterate((BBOX_RING *)solid->shells, (void (*)(BBOX_RING *))func);
}

void BBoxSolidIterateShells1A(BBOX_SOLID *solid, void (*func)(BBOX_SHELL *, void *), void *parm)
{
  BBoxIterate1A((BBOX_RING *)solid->shells, (void (*)(BBOX_RING *, void *))func, parm);
}

void BBoxSolidIterateShells2A(BBOX_SOLID *solid, void (*func)(BBOX_SHELL *, void *, void *), void *parm1, void *parm2)
{
  BBoxIterate2A((BBOX_RING *)solid->shells, (void (*)(BBOX_RING *, void *, void *))func, parm1, parm2);
}


static void BBoxSolidDestroyShells(BBOX_SHELL *first)
{
  BBOX_SHELL *shell, *prev;

  if (first) {
    for (shell = first->prev; shell != first; shell = prev) {
      prev = shell->prev;
      BBoxDestroyShell(shell);
    }
    BBoxDestroyShell(first);
  }
}

#ifdef NEVER

static void BBoxSolidDestroyVertices(BBOX_SOLID *solid)
{
  BBoxIterateVertices(solid->vertices, BBoxDestroyVertex);
}
#endif 


void BBoxDestroySolid(BBOX_SOLID *solid)
{
  

  
  if (brep_destroy_solid_callback) 
    brep_destroy_solid_callback(solid);

  
  BBoxSolidDestroyShells(solid->shells);

  
  BBoxDestroyVertices(solid->vertices);

  
  DISPOSESOLID(solid);
}

