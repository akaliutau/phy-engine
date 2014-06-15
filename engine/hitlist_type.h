/* hitlist_type.h: HITLIST type declaration: separate include file because 
 * it is needed so often. Putting all hitlist stuff in one include file would
 * cause almost the entire program to be recompiled whenever e.g. a
 * routine to manipulate hitlists is added. */

#ifndef _HITLIST_TYPE_H_
#define _HITLIST_TYPE_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "ray.h"	/* contains HITREC struct def. */
#include "DList.h"	/* Doubly Linked List generics */

/* same layout as DLIST in DList.h */
typedef struct HITLIST {
  struct HITREC *hit;
  struct HITLIST *prev, *next;
} HITLIST;

#ifdef __cplusplus
}
#endif
#endif /*_HITLIST_TYPE_H_*/
