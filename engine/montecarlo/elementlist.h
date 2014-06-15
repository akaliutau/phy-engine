/* elementlist.h: linear lists of ELEMENTs */

#ifndef _ELEMENTLIST_H_
#define _ELEMENTLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "elementtype.h"
#include "List.h"

/* same layout as LIST in generics/List.h in order to be able to use
 * the generic list procedures defined in generics/List.c */
typedef struct ELEMENTLIST {
	struct ELEMENT *element;
	struct ELEMENTLIST *next;
} ELEMENTLIST;

#define ElementListCreate	(ELEMENTLIST *)ListCreate

#define ElementListAdd(elementlist, element)	\
        (ELEMENTLIST *)ListAdd((LIST *)elementlist, (void *)element)

#define ElementListCount(elementlist) \
        ListCount((LIST *)elementlist)

#define ElementListGet(elementlist, index) \
        (ELEMENT *)ListGet((LIST *)elementlist, index)

#define ElementListNext(pelementlist) \
        (ELEMENT *)ListNext((LIST **)pelementlist)

#define ElementListRemove(elementlist, element) \
        (ELEMENTLIST *)ListRemove((LIST *)elementlist, (void *)element)

#define ElementListIterate(elementlist, proc) \
        ListIterate((LIST *)elementlist, (void (*)(void *))proc)

#define ElementListDestroy(elementlist) \
        ListDestroy((LIST *)elementlist)

#define ForAllElements(elem, elemlist) ForAllInList(ELEMENT, elem, elemlist)

#ifdef __cplusplus
}
#endif

#endif /* _ELEMENTLIST_H_ */

