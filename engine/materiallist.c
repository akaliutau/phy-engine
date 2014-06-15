

#include <string.h>
#include "materiallist.h"


MATERIAL *MaterialLookup(MATERIALLIST *MaterialLib, char *name)
{
  MATERIAL *m;

  while ((m = MaterialListNext(&MaterialLib))) 
    if (strcmp(m->name, name) == 0)
      return m;

  return (MATERIAL *)NULL;
}

