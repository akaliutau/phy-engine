

#include "dummy.h"

static void Defaults(void)
{
}

static void Init(void)
{
}

static void Terminate(void)
{
}

static COLOR ScaleForComputations(COLOR radiance)
{
  return radiance;
}

static COLOR ScaleForDisplay(COLOR radiance)
{
  return radiance;
}

static float ReverseScaleForComputations(float dl)
{
  return -1.0;
}

TONEMAP TM_Dummy = {
  "Dummy", "Dummy", "dummyButton", 3,
  Defaults,
  (void (*)(int *, char **))NULL,
  (void (*)(FILE *))NULL,
  Init,
  Terminate,
  ScaleForComputations,
  ScaleForDisplay,
  ReverseScaleForComputations,
  (void (*)(void *))NULL,
  (void (*)(void *))NULL,
  (void (*)(void))NULL,
  (void (*)(void))NULL
};
