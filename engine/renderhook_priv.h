#ifndef _RENDERHOOK_PRIV_H_
#define _RENDERHOOK_PRIV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "List.h"
#include "renderhook.h"

/* RenderHook List */  

typedef struct
{
  RENDERHOOKFUNCTION func;
  void *data;
} RENDERHOOK;

/* same layout as LIST in generics/List.h in order to be able to use
 * the generic list procedures defined in generics/List.c */
typedef struct RENDERHOOKLIST {
	struct RENDERHOOK *renderhook;
	struct RENDERHOOKLIST *next;
} RENDERHOOKLIST;

#define RenderHookListCreate	(RENDERHOOKLIST *)ListCreate

#define RenderHookListAdd(renderHookList, hook)	\
        (RENDERHOOKLIST *)ListAdd((LIST *)renderHookList, (void *)hook)

#define RenderHookListCount(renderHookList) \
        ListCount((LIST *)renderHookList)

#define RenderHookListGet(renderHookList, index) \
        (RENDERHOOK *)ListGet((LIST *)renderHookList, index)

#define RenderHookListNext(prenderHookList) \
        (RENDERHOOK *)ListNext((LIST **)prenderHookList)

#define RenderHookListRemove(renderHookList, hook) \
        (RENDERHOOKLIST *)ListRemove((LIST *)renderHookList, (void *)hook)

#define RenderHookListIterate(renderHookList, proc) \
        ListIterate((LIST *)renderHookList, (void (*)(void *))proc)

#define RenderHookListDestroy(renderHookList) \
        ListDestroy((LIST *)renderHookList)

#define ForAllHooks(h, hooklist) ForAllInList(RENDERHOOK, h, hooklist)

#ifdef __cplusplus
}
#endif

#endif
