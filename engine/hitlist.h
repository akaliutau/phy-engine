/* hitlist.h: doubly linked list of HITRECs */

#ifndef _HITLIST_H_
#define _HITLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "hitlist_type.h"

#define HitListCreate()	(HITLIST *)DListCreate()

#define ForAllHits(hitp, hitlist) ForAllInDList(HITREC, hitp, hitlist)

#define HitListAdd(hitlist, hitp) (HITLIST *)DListAdd((DLIST *)hitlist, (void *)hitp)

#define HitListCount(hitlist) DListCount((DLIST *)hitlist)

#define HitListRemove(hitlist, hitp) (HITLIST *)DListRemove((DLIST *)hitlist, (void *)hitp)

#define HitListDestroy(hitlist) DListDestroy((DLIST *)hitlist)

#define HitListSort(hitlist, hitcompare)	(HITLIST *)DListSort((DLIST *)hitlist, (int (*)(void *, void *))hitcompare)

/* creates a duplicate of the given hit record */
extern HITREC *DuplicateHit(HITREC *hit);

/* disposes of a hitlist (the hits themselves + the list) */
extern void DestroyHitlist(HITLIST *hitlist);

/* print an individual hit */
extern void PrintHit(FILE *out, HITREC *hit);

/* print the hits in the hit list */
extern void PrintHits(FILE *out, HITLIST *hits);

#ifdef __cplusplus
}
#endif

#endif /*_HITLIST_H_*/
