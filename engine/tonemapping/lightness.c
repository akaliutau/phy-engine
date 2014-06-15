

#include <stdlib.h>
#include <unistd.h>

#include "lightness.h"
#include "error.h"
#include "cie.h"
#include "statistics.h" 

static void Defaults(void)
{
}

static void Init(void)
{
}

static void Terminate(void)
{
}

static float Lightness(float luminance)
{
  float relative_luminance;

  if (reference_luminance == 0.0)
    return 0.0;

  relative_luminance = luminance / reference_luminance;
  if (relative_luminance > 0.008856)
    return (1.16 * pow(relative_luminance, 0.33) - 0.16);
  else
    return 9.033 * relative_luminance;
}

static COLOR ScaleForComputations(COLOR radiance)
{
  Fatal(-1, "ScaleForComputations", "%s %d not yet implemented", __FILE__, __LINE__);
  return radiance;
}

static COLOR ScaleForDisplay(COLOR radiance)
{
  float max, scale_factor;

  max = COLORMAXCOMPONENT(radiance);
  if (max < 1e-32) return radiance;

  
  scale_factor = Lightness(WHITE_EFFICACY * max);
  if (scale_factor == 0.) return radiance;

  COLORSCALE((scale_factor/max), radiance, radiance);
  return radiance;
}

static float ReverseScaleForComputations(float dl)
{
  Fatal(-1, "ReverseScaleForComputations", "%s %d not yet implemented", __FILE__, __LINE__);
  return -1.0;
}

TONEMAP TM_Lightness = {
  "Lightness Mapping", "Lightness", "tmoLightnessButton", 3,
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
