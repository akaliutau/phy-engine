

#include "pools.h"
#include "scene.h"
#include "mcradP.h"

#ifndef NOPOOLS
static POOL *coeffPool[MAX_BASIS_SIZE+1];
#define NEWCOEFFICIENTS(n)	(COLOR *)NewPoolCell(n*sizeof(COLOR), 0, "MONTECARLO coefficients", &coeffPool[n])
#define DISPOSECOEFFICIENTS(coeff, n) Dispose((char *)coeff, &coeffPool[n])
#else
#define NEWCOEFFICIENTS(n)	(COLOR *)Alloc(n*sizeof(COLOR))
#define DISPOSECOEFFICIENTS(coeff, n) Free((char *)coeff, n*sizeof(COLOR))
#endif

static int coeffPoolsInited = FALSE;
static void InitCoeffPools(void)
{
#ifndef NOPOOLS
  int i;
  for (i=0; i<=MAX_BASIS_SIZE; i++)
    coeffPool[i] = (POOL *)NULL;
#endif
}

void InitCoefficients(ELEMENT *elem)
{
  if (!coeffPoolsInited) {
    InitCoeffPools();
    coeffPoolsInited = TRUE;
  }

  elem->rad = elem->unshot_rad = elem->received_rad = (COLOR *)NULL;
  elem->basis = &dummyBasis;
}

void DisposeCoefficients(ELEMENT *elem)
{
  if (elem->basis && elem->basis != &dummyBasis && elem->rad) {
    DISPOSECOEFFICIENTS(elem->rad, elem->basis->size);
    DISPOSECOEFFICIENTS(elem->unshot_rad, elem->basis->size);
    DISPOSECOEFFICIENTS(elem->received_rad, elem->basis->size);
  }
  InitCoefficients(elem);
}


static BASIS *ActualBasis(ELEMENT *elem)
{
  if (elem->iscluster)
    return &clusterBasis;
  else
    return &basis[NR_VERTICES(elem)==3 ? ET_TRIANGLE : ET_QUAD][mcr.approx_type];
}

void AllocCoefficients(ELEMENT *elem)
{
  DisposeCoefficients(elem);
  elem->basis = ActualBasis(elem);
  elem->rad = NEWCOEFFICIENTS(elem->basis->size);
  elem->unshot_rad = NEWCOEFFICIENTS(elem->basis->size);
  elem->received_rad = NEWCOEFFICIENTS(elem->basis->size);
}

void ReAllocCoefficients(ELEMENT *elem)
{
  if (elem->basis != ActualBasis(elem))
    AllocCoefficients(elem);
}
