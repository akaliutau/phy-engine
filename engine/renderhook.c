

#include "stdio.h"
#include "renderhook.h"
#include "renderhook_priv.h"
#include "pools.h"
#include "error.h"

static RENDERHOOKLIST *oRenderHookList = NULL;

void RenderHooks(void)
{
  if(oRenderHookList != NULL)
  {
    ForAllHooks(h, oRenderHookList) 
      {
	h->func(h->data);
      }
    EndForAll
  }
}

void AddRenderHook(RENDERHOOKFUNCTION func, void *data)
{
  RENDERHOOK *hook;

  if(oRenderHookList == NULL)
  {
    oRenderHookList = RenderHookListCreate();
  }

  hook = (RENDERHOOK *)Alloc(sizeof(RENDERHOOK));

  hook->func = func;
  hook->data = data;

  oRenderHookList = RenderHookListAdd(oRenderHookList, hook);
}

void RemoveRenderHook(RENDERHOOKFUNCTION func, void *data)
{
  RENDERHOOKLIST *list = oRenderHookList;
  RENDERHOOK *hook;

  

  do
  {
    hook = RenderHookListNext(&list);
  } while (hook != NULL && (hook->func != func || hook->data != data));

  if(hook == NULL)
  {
    Warning("RemoveRenderHook", "Hook to remove not found");
  }
  else
  {
    oRenderHookList = RenderHookListRemove(oRenderHookList, hook);
  }
}


void RemoveAllRenderHooks(void)
{
  RenderHookListDestroy(oRenderHookList);
  oRenderHookList = NULL;
}
