

#include "patchlist.h"
#include "monitor.h"
#include "error.h"

static PATCHLIST *monitoredPatches = (PATCHLIST *)NULL;


void MonitorInit(void)
{
  PatchListDestroy(monitoredPatches);
  monitoredPatches = (PATCHLIST *)NULL;
}


int Monitored(PATCH *patch)
{
  PATCHLIST *pl;
  for (pl = monitoredPatches; pl; pl = pl->next)
    if (pl->patch == patch)
      return TRUE;
  return FALSE;
}


void MonitorAdd(PATCH *patch)
{
  if (!Monitored(patch)) 
    monitoredPatches = PatchListAdd(monitoredPatches, patch);
  else
    Warning(NULL, "Patch %d is already being monitored", patch->id);
}


void MonitorRemove(PATCH *patch)
{
  if (Monitored(patch))
    monitoredPatches = PatchListRemove(monitoredPatches, patch);
  else
    Warning(NULL, "Patch %d was not being monitored", patch->id);
}

