

#include <string.h>

#include "config.h"

#include "raytracing.h"
#include "scene.h"
#include "geom.h"
#include "error.h"
#include "options.h"
#include "defaults.h"
#include "camera.h"
#include "statistics.h"


#ifdef RT_STOCHASTIC
#include "raytracing/rtstochastic.h"
#endif
#ifdef RT_BIDIR
#include "raytracing/bidirpath.h"
#endif
#ifdef RT_GENETIC
#include "GENETIC/genetic.h"
#endif
#include "raycasting.h"
#include "raymatting.h"


RAYTRACINGMETHOD *RayTracingMethods[] = {
#ifdef RT_STOCHASTIC
  &RT_StochasticMethod,
#endif
#ifdef RT_BIDIR
  &RT_BidirPathMethod,
#endif
#ifdef RT_GENETIC
  &RT_GeneticMethod,
#endif
  &RayCasting,
  &RayMatting,
  (RAYTRACINGMETHOD *)NULL
};

long rt_raycount=0, rt_pixcount=0;
double rt_total_time = 0.;



RAYTRACINGMETHOD *RayTracing = (RAYTRACINGMETHOD *)NULL;


void RayTrace(char *fname, FILE *fp, int ispipe)
{
  ImageOutputHandle *img = NULL;

  if (fp) {
    img = CreateRadianceImageOutputHandle(fname, fp, ispipe, 
					  Camera.hres, Camera.vres, reference_luminance/179.);
    if (!img) return;
  }

  if (RayTracing)
    RayTracing->Raytrace(img);

//  if (img)
//    DeleteImageOutputHandle(img);
}


void SetRayTracing(RAYTRACINGMETHOD *newmethod)
{
  if(RayTracing)
  {
    RayTracing->HideControlPanel();
    RayTracing->InterruptRayTracing();
    RayTracing->Terminate();
  }

  RayTracing = newmethod;
  if (RayTracing)
    RayTracing->Initialize();
}


static char raytracing_methods_string[1000];
static void make_raytracing_methods_string(void)
{
  char *str = raytracing_methods_string;
  int n;
  sprintf(str, "\
-raytracing-method <method>: Set pixel-based radiance computation method\n%n",
	  &n); str += n;
  sprintf(str, "\tmethods: %-20.20s %s%s\n%n", 
	  "none", "no pixel-based radiance computation", 
	  !RayTracing ? " (default)" : "", &n); str += n;
  ForAllRayTracingMethods(method) {
    sprintf(str, "\t         %-20.20s %s%s\n%n",
	    method->shortName, method->fullName, 
	    RayTracing==method ? " (default)" : "", &n); str += n;
  } EndForAll;
  *(str-1) = '\0';	
}

void RayTracingDefaults(void)
{
  ForAllRayTracingMethods(method) {
    method->Defaults();
    if (strncasecmp(DEFAULT_RAYTRACING_METHOD, method->shortName, method->nameAbbrev) == 0)
      SetRayTracing(method);
  } EndForAll;
  make_raytracing_methods_string();	
}

static void RayTracingOption(void *value)
{
  char *name = *(char **)value;

  ForAllRayTracingMethods(method) {
    if (strncasecmp(name, method->shortName, method->nameAbbrev) == 0) {
      SetRayTracing(method);
      return;
    }
  } EndForAll;

  if (strncasecmp(name, "none", 4) == 0)
    SetRayTracing((RAYTRACINGMETHOD *)NULL);
  else 
    Error(NULL, "Invalid raytracing method name '%s'", name);
}

static CMDLINEOPTDESC raytracingOptions[] = {
  {"-raytracing-method", 4, Tstring, NULL, RayTracingOption,
   raytracing_methods_string},
  {NULL	, 	0,	TYPELESS, 	NULL, 	DEFAULT_ACTION,
   NULL}
};

void ParseRayTracingOptions(int *argc, char **argv)
{
  ParseOptions(raytracingOptions, argc, argv);
  ForAllRayTracingMethods(method) {
    method->ParseOptions(argc, argv);
  } EndForAll;
}

void PrintRayTracingOptions(FILE *fp)
{
  fprintf(fp, "\nPixel-driven radiance computation options:\n");
  PrintOptions(fp, raytracingOptions);

  ForAllRayTracingMethods(method) {
    method->PrintOptions(fp);
  } EndForAll;
}
