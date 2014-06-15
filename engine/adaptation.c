

#include "adaptation.h"
#include "tonemapping.h"
#include "statistics.h"
#include "error.h"
#include "scene.h"
#include "radiance.h"
#include "raytracing.h"
#include "raycasting.h"
#include "render.h"



typedef struct
{
  float luminance;
  float area;
} LUMAREA;


static int _lumAreaComp(const void *la1, const void *la2)
{
  float l1 = ((LUMAREA*)la1)->luminance;
  float l2 = ((LUMAREA*)la2)->luminance;
  return l1 > l2 ? 1 : l1 == l2 ? 0 : -1;
}


float MeanAreaWeightedLuminance(LUMAREA *pairs, int numPairs)
{
  extern double total_area;
  float areaMax = total_area/2.0;
  float areaCnt = 0.0;

  qsort((void*)pairs, numPairs, sizeof(LUMAREA), _lumAreaComp);

  while (areaCnt < areaMax)
  {
    areaCnt += pairs->area;
    pairs++;
  }

  pairs--;

  return pairs->luminance;
}


static int _numEntries;
static double _logAreaLum, _totalLogArea;
static LUMAREA * _lumArea; 
static float _lumMin = HUGE;
static float _lumMax = 0.0;


static COLOR InitRadianceEstimate(PATCH *patch)
{
  COLOR E = PatchAverageEmittance(patch, ALL_COMPONENTS),
        R = PatchAverageNormalAlbedo(patch, BSDF_ALL_COMPONENTS),
        radiance;
  
  COLORPROD(R,estimated_average_radiance,radiance);
  COLORADDSCALED(radiance,(1./M_PI),E,radiance);
  return radiance;
}

static COLOR (*PatchRadianceEstimate)(PATCH *P) = InitRadianceEstimate;

static float PatchBrightnessEstimate(PATCH *patch)
{
  COLOR radiance = PatchRadianceEstimate(patch);
  float brightness = ColorLuminance(radiance);
  if (brightness < EPSILON) brightness = EPSILON;
  return brightness;
}

static void PatchComputeLogAreaLum(PATCH *patch)
{
  float brightness = PatchBrightnessEstimate(patch);
  _logAreaLum += patch->area*log(brightness);
}

static void PatchFillLumArea(PATCH *patch)
{
  float brightness = PatchBrightnessEstimate(patch);

  _lumArea->luminance = brightness;
  _lumArea->area      = patch->area;

  _lumMin = MIN(_lumMin, _lumArea->luminance);
  _lumMax = MAX(_lumMax, _lumArea->luminance);

  _lumArea++;
  _numEntries++;
}


static int last_is_scene = TRUE;

void EstimateSceneAdaptation(COLOR (*patch_radiance)(PATCH *))
{
  PatchRadianceEstimate = patch_radiance;
  last_is_scene = TRUE;

  switch (tmopts.statadapt) {
  case TMA_NONE:
    break;
  case TMA_AVERAGE:
    {
      
      _logAreaLum = 0.0;
      PatchListIterate(Patches, PatchComputeLogAreaLum);
      tmopts.lwa = exp(_logAreaLum/total_area+0.84);
      break;
    }
  case TMA_MEDIAN:
    {
      
      LUMAREA  *la = malloc(nrpatches*sizeof(LUMAREA));
      
      _lumArea = la;
      PatchListIterate(Patches, PatchFillLumArea);
      tmopts.lwa = MeanAreaWeightedLuminance(la, nrpatches);
      
      free(la);
      break;
    }
  default:
    Error("ComputeSomeSceneStats", "unknown static adaptation method %d", tmopts.statadapt);
  }
}

void InitSceneAdaptation(void)
{
  EstimateSceneAdaptation(InitRadianceEstimate);
}

void EstimateViewAdaptation(void)
{
  last_is_scene = FALSE;

  if(tmopts.statadapt != TMA_NONE)
  {
    if(renderopts.render_raytraced_image && RayTracing)
      tmopts.lwa = RayTracing->AdaptationLuminance(tmopts.pow_bright_adjust);
    else if(Radiance)
      tmopts.lwa = RayCastAdaptationLuminance(tmopts.pow_bright_adjust);
    else
      EstimateSceneAdaptation(PatchRadianceEstimate);
  }
}

void ReEstimateAdaptation(void)
{
  if (last_is_scene)
    EstimateSceneAdaptation(PatchRadianceEstimate);
  else
    EstimateViewAdaptation();

  fprintf(stderr, "old ref. luminance = %g\n", reference_luminance);
  reference_luminance = 5.42 * tmopts.lwa;
  fprintf(stderr, "new ref. luminance = %g\n", reference_luminance);
}
