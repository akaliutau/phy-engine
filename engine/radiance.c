

#include <string.h>

#include "config.h"

#include "radiance.h"
#include "scene.h"
#include "geom.h"
#include "error.h"
#include "options.h"
#include "defaults.h"

#ifdef STOCHRAY
#include "montecarlo/stochray.h"
#endif
#ifdef WDRS
#include "montecarlo/wdrs.h"
#endif
#ifdef NEVER
#ifdef RWRAD
#include "montecarlo/rwrad.h"
#endif
#endif

#ifdef STOCHRELAX
#include "montecarlo/mcrad.h"
#endif
#ifdef RWRAD
#include "montecarlo/mcrad.h"
#endif

#ifdef PHOTONMAP
#include "photonmapping/pmap.h"
#endif



RADIANCEMETHOD *RadianceMethods[] = {

#ifdef WDRS
  &WDRSRadiosity,
#endif
#ifdef STOCHRAY
  &StochasticRayRadiosity,
#endif

#ifdef STOCHRELAX
  &StochasticRelaxationRadiosity,
#endif
#ifdef RWRAD
  &RandomWalkRadiosity,
#endif

#ifdef PHOTONMAP
  &Pmap,
#endif
#ifdef RDRAD
  &RDRad,
#endif
  (RADIANCEMETHOD *)NULL
};


RADIANCEMETHOD *Radiance = (RADIANCEMETHOD *)NULL;


void SetRadianceMethod(RADIANCEMETHOD *newmethod)
{
  if (Radiance) {
    Radiance->HideControlPanel();
    Radiance->Terminate();
    
    if (Radiance->DestroyPatchData)
      PatchListIterate(Patches, Radiance->DestroyPatchData);
  }
  Radiance = newmethod;
  if (Radiance) {
    if (Radiance->CreatePatchData)
      PatchListIterate(Patches, Radiance->CreatePatchData);
    Radiance->Initialize();
  }
}


static char radiance_methods_string[1000];
static void make_radiance_methods_string(void)
{
  char *str = radiance_methods_string;
  int n;
  sprintf(str, "\
-radiance-method <method>: Set world-space radiance computation method\n%n",
	  &n); str += n;
  sprintf(str, "\tmethods: %-20.20s %s%s\n%n", 
	  "none", "no world-space radiance computation", 
	  !Radiance ? " (default)" : "", &n); str += n;
  ForAllRadianceMethods(method) {
    sprintf(str, "\t         %-20.20s %s%s\n%n",
	    method->shortName, method->fullName, 
	    Radiance==method ? " (default)" : "", &n); str += n;
  } EndForAll;
  *(str-1) = '\0';	
}

void RadianceDefaults(void)
{
  ForAllRadianceMethods(method) {
    method->Defaults();
    if (strncasecmp(DEFAULT_RADIANCE_METHOD, method->shortName, method->nameAbbrev) == 0)
      SetRadianceMethod(method);
  } EndForAll;
  make_radiance_methods_string();
}

static void RadianceMethodOption(void *value)
{
  char *name = *(char **)value;

  ForAllRadianceMethods(method) {
    if (strncasecmp(name, method->shortName, method->nameAbbrev) == 0) {
      SetRadianceMethod(method);
      return;
    }
  } EndForAll;

  if (strncasecmp(name, "none", 4) == 0)
    SetRadianceMethod(NULL);
  else
    Error(NULL, "Invalid world-space radiance method name '%s'", name);
}

static CMDLINEOPTDESC radianceOptions[] = {
  {"-radiance-method", 4, Tstring, 	NULL,	RadianceMethodOption,
   radiance_methods_string },
  {NULL	, 	0,	TYPELESS, 	NULL, 	DEFAULT_ACTION,
   NULL }
};

void ParseRadianceOptions(int *argc, char **argv)
{
  ParseOptions(radianceOptions, argc, argv);

  ForAllRadianceMethods(method) {
    method->ParseOptions(argc, argv);
  } EndForAll;
}

void PrintRadianceOptions(FILE *fp)
{
  fprintf(fp, "\nObject-space radiance computation options:\n");
  PrintOptions(fp, radianceOptions);

  ForAllRadianceMethods(method) {
    method->PrintOptions(fp);
  } EndForAll;
}

