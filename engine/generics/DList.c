

#include "DList.h"
#include "private.h"
#include "pools.h"

#ifndef NOPOOLS
static POOL *dlistPool = (POOL *)NULL;
#define NEWDLIST()  	(DLIST *)NewPoolCell(sizeof(DLIST), 0, "generics DList cells", &dlistPool)
#define DISPOSEDLIST(ptr) Dispose((unsigned char *)(ptr), &dlistPool)
#else 
#define NEWDLIST()	(DLIST *)Alloc(sizeof(DLIST))
#define DISPOSEDLIST(ptr) Free((char *)ptr, sizeof(DLIST))
#endif 


#ifndef NODLISTMACROS
void *__pdlistel__;	
#endif 

#ifdef NODLISTMACROS

DLIST *DListCreate(void)
{
  return (DLIST *)NULL;
}
#endif 


DLIST *DListAdd(DLIST *dlist, void *pelement)
{
  DLIST *newdlist;

  
  if (pelement == (void *)NULL)
    return dlist;

  newdlist = NEWDLIST();
  newdlist->pelement = pelement;
  newdlist->next = dlist;
  newdlist->prev = (DLIST *)NULL;
  
  if (dlist)
    dlist->prev = newdlist;
  
  return newdlist;
}


int DListCount(DLIST *dlist)
{
  int count = 0;
  
  while (dlist) {
    count++;
    dlist = dlist->next;
  }
  
  return count;
}


void *DListGet(DLIST *dlist, int index)
{
  while (dlist && index > 0) {
    dlist = dlist->next;
    index--;
  }
  
  if (!dlist)
    return (void *)NULL;
  else
    return dlist->pelement;
}

#ifdef NODLISTMACROS
void *DListNext(DLIST **dlist)
{
  void *pelement = (void *)NULL;
  
  if (*dlist) {
    pelement = (*dlist)->pelement;
    *dlist = (*dlist)->next;
  } else
    pelement = (void *)NULL;

  return pelement;
}
#endif 


DLIST *DListMerge(DLIST *dlist1, DLIST *dlist2)
{
  void *pelement = (void *)NULL;
  
  while ((pelement = DListNext(&dlist2)))
    dlist1 = DListAdd(dlist1, pelement);
  
  return dlist1;
}


DLIST *DListDuplicate(DLIST *dlist)
{
  DLIST *newdlist = (DLIST *)NULL;
  void *pelement = (void *)NULL;
  
  while ((pelement = DListNext(&dlist)))
    newdlist = DListAdd(newdlist, pelement);
  
  return newdlist;
}


DLIST *DListRemoveCell(DLIST *dlist, DLIST *p)
{
  if (p == dlist) {
    dlist = dlist->next;
    dlist->prev = (DLIST *)NULL;
    DISPOSEDLIST(p);
  } else {
    if (p->prev) p->prev->next = p->next;
    if (p->next) p->next->prev = p->prev;
    DISPOSEDLIST(p);
  }

  return dlist;
}


DLIST *DListRemove(DLIST *dlist, void *pelement)
{
  DLIST *p;

  
  if (!dlist) {
    GdtError("DlistRemove", "attempt to remove an element from an empty dlist");
    return dlist;	
  }
  
  
  for (p = dlist; p && p->pelement != pelement; p = p->next) {}

  
  if (!p) {
    GdtError("DlistRemove", "attempt to remove a nonexisting element from a dlist");
    return dlist;
  }

  
  return DListRemoveCell(dlist, p);
}


DLIST *DListSort(DLIST *list, int (*compare)(void *, void *))
{
  DLIST *p, *next, *q;

  if (!list)
    return list;

  p = list->next;	
  while (p) {
    next = p->next;

    
    q=list;
    while (q!=p && compare(p->pelement, q->pelement) >= 0) {
      q=q->next;
    }

    if (q!=p) {
      
      p->prev->next = p->next;
      if (p->next) p->next->prev = p->prev;

      
      p->prev = q->prev;
      p->next = q;
      if (p->prev) p->prev->next = p; else list = p;
      p->next->prev = p;
    }

    p = next;
  }

  return list;
}



#ifdef NODLISTMACROS
void DListIterate(DLIST *dlist, void (*proc)(void *))
{
  void *pelement;
  
  while (dlist) {
    pelement = dlist->pelement;
    dlist = dlist->next;
    proc(pelement);
  }		
}
#endif 

#ifdef NODLISTMACROS
void DListIterate1A(DLIST *dlist, void (*proc)(void *, void *), void *extra)
{
  void *pelement;
  
  while (dlist) {
    pelement = dlist->pelement;
    dlist = dlist->next;
    proc(pelement, extra);
  }		
}
#endif 

#ifdef NODLISTMACROS
void DListIterate1B(DLIST *dlist, void (*proc)(void *, void *), void *extra)
{
  void *pelement;
  
  while (dlist) {
    pelement = dlist->pelement;
    dlist = dlist->next;
    proc(extra, pelement);
  }		
}
#endif 


void DListDestroy(DLIST *dlist)
{
  DLIST *p;
  
  while (dlist) {
    p = dlist->next;
    DISPOSEDLIST(dlist);
    dlist = p;
  }
}




