

#include "vectorlist.h"


VECTOR *VectorListFind(VECTORLIST *vl, VECTOR *vector)
{
  VECTOR *v;

  while ((v = VectorListNext(&vl))) {
    float eps = VECTORTOLERANCE(*v) + VECTORTOLERANCE(*vector);
    if (VECTOREQUAL(*v, *vector, eps))
      return v;
  }

  return (VECTOR *)NULL;
}
