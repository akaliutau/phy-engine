

#include "private.h"

void BBoxIterate(BBOX_RING *first, void (*func)(BBOX_RING *))
{
  BBOX_RING *ring, *next;

  if (first) {
    next = first;
    do {
      ring = next;
      next = ring->next;
      func(ring);
    } while (next != first);
  }  
}

void BBoxIterate1A(BBOX_RING *first, void (*func)(BBOX_RING *, void *), void *parm)
{
  BBOX_RING *ring, *next;

  if (first) {
    next = first;
    do {
      ring = next;
      next = ring->next;
      func(ring, parm);
    } while (next != first);
  }  
}

void BBoxIterate2A(BBOX_RING *first, void (*func)(BBOX_RING *, void *, void *), void *parm1, void *parm2)
{
  BBOX_RING *ring, *next;

  if (first) {
    next = first;
    do {
      ring = next;
      next = ring->next;
      func(ring, parm1, parm2);
    } while (next != first);
  }  
}

