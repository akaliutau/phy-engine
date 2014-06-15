

#include "bbox.h"
#include "private.h"
#include "pools.h"

#ifndef NOPOOLS
static POOL *shellPool = (POOL *)NULL;
#define NEWSHELL()  	(BBOX_SHELL *)NewPoolCell(sizeof(BBOX_SHELL), 0, "brep shells", &shellPool)
#define DISPOSESHELL(ptr) Dispose((unsigned char *)(ptr), &shellPool)
#else 
#define NEWSHELL()	(BBOX_SHELL *)Alloc(sizeof(BBOX_SHELL))
#define DISPOSESHELL(ptr) Free((char *)ptr, sizeof(BBOX_SHELL))
#endif 


static BBOX_CALLBACK_FUNC brep_close_shell_callback = (BBOX_CALLBACK_FUNC)NULL,
                          brep_destroy_shell_callback = (BBOX_CALLBACK_FUNC)NULL,
                          brep_create_shell_callback = (BBOX_CALLBACK_FUNC)NULL;


BBOX_CALLBACK_FUNC BBoxSetCreateShellCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_create_shell_callback;
  brep_create_shell_callback = func;
  return oldfunc;
}


BBOX_CALLBACK_FUNC BBoxSetCloseShellCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_close_shell_callback;
  brep_close_shell_callback = func;
  return oldfunc;
}


BBOX_CALLBACK_FUNC BBoxSetDestroyShellCallback(BBOX_CALLBACK_FUNC func)
{
  BBOX_CALLBACK_FUNC oldfunc = brep_destroy_shell_callback;
  brep_destroy_shell_callback = func;
  return oldfunc;
}


void BBoxConnectShellToSolid(BBOX_SHELL *shell, BBOX_SOLID *solid)
{
  shell->solid = solid;
  if (!solid)
    return;

  if (!solid->shells) { 
    solid->shells = shell;
    shell->next = shell->prev = shell;
  } else {		
    shell->next = solid->shells;
    shell->prev = solid->shells->prev;
    shell->next->prev = shell->prev->next = shell;
  }
}


BBOX_SHELL *BBoxCreateShell(BBOX_SOLID *solid, void *client_data)
{
  BBOX_SHELL *shell;

  shell = NEWSHELL();
  shell->faces = (BBOX_FACE *)NULL;
  shell->client_data = client_data;

  BBoxConnectShellToSolid(shell, solid);

  
  shell->client_data = client_data;
  if (brep_create_shell_callback)
    shell->client_data = brep_create_shell_callback(shell);

  return shell;
}


void BBoxCloseShell(BBOX_SHELL *shell)
{
  
  BBoxShellIterateFaces(shell, BBoxCloseFace);

  
  if (brep_close_shell_callback)
    shell->client_data = brep_close_shell_callback(shell);
}


void BBoxShellIterateFaces(BBOX_SHELL *shell, void (*func)(BBOX_FACE *))
{
  BBoxIterate((BBOX_RING *)shell->faces, (void (*)(BBOX_RING *))func);
}

void BBoxShellIterateFaces1A(BBOX_SHELL *shell, void (*func)(BBOX_FACE *, void *), void *parm)
{
  BBoxIterate1A((BBOX_RING *)shell->faces, (void (*)(BBOX_RING *, void *))func, parm);
}

void BBoxShellIterateFaces2A(BBOX_SHELL *shell, void (*func)(BBOX_FACE *, void *, void *), void *parm1, void *parm2)
{
  BBoxIterate2A((BBOX_RING *)shell->faces, (void (*)(BBOX_RING *, void *, void *))func, parm1, parm2);
}


void BBoxDisconnectShellFromSolid(BBOX_SHELL *shell)
{
  BBOX_SOLID *solid = shell->solid;
  
  if (!solid)			
    return;

  if (solid->shells == shell) {	
    if (shell->next == shell)	
      solid->shells = (BBOX_SHELL *)NULL;

    else			
      solid->shells = shell->next;
  } 

  shell->next->prev = shell->prev;
  shell->prev->next = shell->next;

  shell->solid = (BBOX_SOLID *)NULL;
}


static void BBoxShellDestroyFaces(BBOX_FACE *first)
{
  BBOX_FACE *face, *prev;

  if (first) {
    for (face = first->prev; face != first; face = prev) {
      prev = face->prev;
      BBoxDestroyFace(face);
    }
    BBoxDestroyFace(first);
  }
}


void BBoxDestroyShell(BBOX_SHELL *shell)
{
  

  
  BBoxDisconnectShellFromSolid(shell);
  
  
  if (brep_destroy_shell_callback)
    brep_destroy_shell_callback(shell);

  
  BBoxShellDestroyFaces(shell->faces);

  
  DISPOSESHELL(shell);
}
