

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>

#include "config.h"

#include "ui.h"
#include "defaults.h"
#include "options.h"
#include "statistics.h"
#include "camera.h"
#include "render.h"
#include "error.h"
#include "scene.h"
#include "geom.h"
#include "patch_type.h"
#include "patch_flags.h"
#include "material.h"
#include "edf.h"
#include "surface.h"
#include "readmgf.h"
#include "pools.h"
#include "patch.h"
#include "volume.h"
#include "cluster.h"
#include "radiance.h"
#include "raytracing.h"
#include "monitor.h"
#include "vertex.h"
#include "bbox.h"
#include "compound.h"
#include "grid.h"
#include "ipc.h"
#include "renderhook.h"
#include "tonemapping.h"
#include "fileopts.h"
#include "ui_pathdebug.h"
#include "background_edf.H"



int    nrpatches, nrlightsources;
double total_area;
double average_direct_potential, total_direct_potential;
double max_direct_potential, max_direct_importance; 
COLOR  total_emitted_power, estimated_average_radiance;
COLOR  average_reflectivity, max_selfemitted_radiance, max_selfemitted_power;
double reference_luminance;


char *progname;


static void default_alarm_handler(int sig)
{
  
  time_t t = time(NULL);
fprintf(stderr, "%s%s line %d: You're lucky that *I* catched that alarm signal ...\n",
	asctime(localtime(&t)),
	__FILE__, __LINE__);
}

static void PatchAccumulateStats(PATCH *patch)
{
  COLOR 
    E = PatchAverageEmittance(patch, ALL_COMPONENTS),
    R = PatchAverageNormalAlbedo(patch, BSDF_ALL_COMPONENTS),
    power;

  total_area += patch->area;
  COLORSCALE(patch->area, E, power);
  COLORADD(total_emitted_power, power, total_emitted_power);
  COLORADDSCALED(average_reflectivity, patch->area, R, average_reflectivity);
  
  COLORSCALE((1./M_PI), E, E);
  COLORMAX(E, max_selfemitted_radiance, max_selfemitted_radiance);
  COLORMAX(power, max_selfemitted_power, max_selfemitted_power);
}

static void ComputeSomeSceneStats(void)
{
  VECTOR zero;
  COLOR one, average_absorption, BP;

  COLORSETMONOCHROME(one, 1.);
  VECTORSET(zero, 0, 0, 0);

  
  COLORCLEAR(total_emitted_power);
  COLORCLEAR(average_reflectivity);
  COLORCLEAR(max_selfemitted_radiance);
  COLORCLEAR(max_selfemitted_power);
  total_area = 0.;

  
  PatchListIterate(Patches, PatchAccumulateStats);

  
  COLORSCALEINVERSE(total_area, average_reflectivity, average_reflectivity);
  COLORSUBTRACT(one, average_reflectivity, average_absorption);
  COLORSCALEINVERSE(M_PI * total_area, total_emitted_power, estimated_average_radiance);

  
  BP = BackgroundPower(Background, &zero);
  COLORSCALE(1. / (4 * M_PI), BP, BP);
  COLORADD(total_emitted_power, BP, total_emitted_power);
  COLORADD(estimated_average_radiance, BP, estimated_average_radiance);

  COLORDIV(estimated_average_radiance, average_absorption, estimated_average_radiance);

  total_direct_potential = max_direct_potential = average_direct_potential = max_direct_importance = 0.;
}


void AddBackgroundToLightSourceList(void)
{
  if (Background)
    {
      // add to list
      LightSourcePatches = PatchListAdd(LightSourcePatches, Background->bkgPatch);
    }
}



static void AddPatchToLightSourceListIfLightSource(PATCH *patch)
{
  if (patch->surface->material->edf) {
    LightSourcePatches = PatchListAdd(LightSourcePatches, patch);
    nrlightsources++;
  }
}


static void BuildLightSourcePatchList(void)
{
  LightSourcePatches = PatchListCreate();
  nrlightsources = 0;
  PatchListIterate(Patches, AddPatchToLightSourceListIfLightSource);

  // add background if present
  AddBackgroundToLightSourceList();
  nrlightsources++;
}


Boolean ReadFile(char *filename)
{
  char *dot, *slash, *extension;
  FILE *input;
  GEOMLIST *oWorld, *oClusteredWorld;
  GEOM *oClusteredWorldGeom;
  MATERIALLIST *oMaterialLib;
  PATCHLIST *oPatches, *oLightSourcePatches;
  GRID *oWorldGrid;
  RADIANCEMETHOD *oRadiance;
  RAYTRACINGMETHOD *oRayTracing;
  BACKGROUND *oBackground;
  int opatchid, onrpatches;
  clock_t t, last;

  
  if (filename[0] != '#') {
    if ((input = fopen(filename, "r")) == (FILE *)NULL ||
	fgetc(input) == EOF) {
      if (input) fclose(input);
      Error(NULL, "Can't open file '%s' for reading", filename);
      return False;
    }
    fclose(input);
  }

  
  current_directory = (char *)Alloc(strlen(filename)+1);
  sprintf(current_directory, "%s", filename);
  if ((slash = strrchr(current_directory, '/')) != NULL)
    *slash = '\0';
  else
    *current_directory = '\0';

  ErrorReset();

  
  fprintf(stderr, "Terminating current radiance/raytracing method ... \n");
  oRadiance = Radiance;
  SetRadianceMethod((RADIANCEMETHOD *)NULL);
  oRayTracing = RayTracing;
  SetRayTracing((RAYTRACINGMETHOD *)NULL);

  
  fprintf(stderr, "Saving current scene ... \n");
  oWorld = World; World = GeomListCreate();
  oMaterialLib = MaterialLib; MaterialLib = MaterialListCreate();
  oPatches = Patches; Patches = PatchListCreate();
  opatchid = PatchGetNextID(); PatchSetNextID(1);
  onrpatches = nrpatches;
  oClusteredWorld = ClusteredWorld; ClusteredWorld = GeomListCreate();
  oClusteredWorldGeom = ClusteredWorldGeom;
  oLightSourcePatches = LightSourcePatches;
  oWorldGrid = WorldGrid;
  oBackground = Background; Background = (BACKGROUND *)NULL;

  
  fprintf(stderr, "Reading the scene from file '%s' ... \n", filename);
  last = clock();

  if ((dot = strrchr(filename, '.')) != NULL)
    extension = dot+1;
  else
    extension = "mgf";

  if (strcmp(extension, "gz")  == 0 ||
      strcmp(extension, "Z")   == 0 ||
      strcmp(extension, "bz")  == 0 ||
      strcmp(extension, "bz2") == 0) 
  {
    
    do {
      dot--;
    } while (dot>filename && *dot!='.');
    if (*dot == '.')
      extension = dot+1;
  }

  if (strncmp(extension, "mgf", 3) == 0)	ReadMgf(filename);
  else {
    Error(NULL, "Unsupported scene format (no MGF)");
  }

  t = clock();
  fprintf(stderr, "Reading took %g secs.\n", (float)(t-last)/(float)CLOCKS_PER_SEC);
  last = t;

  Free(current_directory, strlen(filename)+1);
  current_directory = NULL;

  
  if (!World) {
    
    fprintf(stderr, "Restoring old scene ... ");
    fflush(stderr);
    GeomListIterate(World, GeomDestroy);
    GeomListDestroy(World);
    World = oWorld;

    MaterialListIterate(MaterialLib, MaterialDestroy);
    MaterialListDestroy(MaterialLib);
    MaterialLib = oMaterialLib;

    Patches = oPatches;
    PatchSetNextID(opatchid);
    nrpatches = onrpatches;

    ClusteredWorld = oClusteredWorld;
    ClusteredWorldGeom = oClusteredWorldGeom;
    LightSourcePatches = oLightSourcePatches;

    WorldGrid = oWorldGrid;
    Background = oBackground;

    SetRadianceMethod(oRadiance);
    SetRayTracing(oRayTracing);

    t = clock();
    fprintf(stderr, "%g secs.\n", (float)(t-last)/(float)CLOCKS_PER_SEC);
    last = t;
    fprintf(stderr, "Done.\n");

    if (!ErrorOccurred()) 
      Error(NULL, "Empty world");

    return False;	
  }

  
  MonitorInit();

  
  fprintf(stderr, "Disposing of the old scene ... "); fflush(stderr);
  if (Radiance) Radiance->Terminate();
  if (RayTracing) RayTracing->Terminate();

  PatchListDestroy(oPatches);
  PatchListDestroy(oLightSourcePatches);

  GeomListIterate(oWorld, GeomDestroy);
  GeomListDestroy(oWorld);

  if (oClusteredWorldGeom) GeomDestroy(oClusteredWorldGeom);
  if (oBackground) oBackground->methods->Destroy(oBackground->data);

  DestroyGrid(oWorldGrid);

  MaterialListIterate(oMaterialLib, MaterialDestroy);
  MaterialListDestroy(oMaterialLib);

  t = clock();
  fprintf(stderr, "%g secs.\n", (float)(t-last)/(float)CLOCKS_PER_SEC);
  last = t;

  
  fprintf(stderr, "Building patch list ... "); fflush(stderr);

  Patches = BuildPatchList(World, PatchListCreate());

  t = clock();
  fprintf(stderr, "%g secs.\n", (float)(t-last)/(float)CLOCKS_PER_SEC);
  last = t;  

  
  fprintf(stderr, "Building light source patch list ... "); fflush(stderr);

  BuildLightSourcePatchList();

  t = clock();
  fprintf(stderr, "%g secs.\n", (float)(t-last)/(float)CLOCKS_PER_SEC);
  last = t;

  
  fprintf(stderr, "Building cluster hierarchy ... "); fflush(stderr);

  ClusteredWorldGeom = CreateClusterHierarchy(Patches);
  if (GeomIsCompound(ClusteredWorldGeom))
    ClusteredWorld = (GEOMLIST *)(ClusteredWorldGeom->obj);
  else {  
    ClusteredWorld = GeomListAdd(GeomListCreate(), ClusteredWorldGeom);
    Warning(NULL, "Strange clusters for this world ...");
  }

  t = clock();
  fprintf(stderr, "%g secs.\n", (float)(t-last)/(float)CLOCKS_PER_SEC);
  last = t;

  
  WorldGrid = CreateGrid(ClusteredWorldGeom);

  t = clock();
  fprintf(stderr, "Engridding took %g secs.\n", (float)(t-last)/(float)CLOCKS_PER_SEC);
  last = t;

  
  fprintf(stderr, "Computing some scene statistics ... "); fflush(stderr);

  nrpatches = nrelements;
  ComputeSomeSceneStats();
  reference_luminance = 5.42 * ((1.-ColorGray(average_reflectivity)) * 
				ColorLuminance(estimated_average_radiance));

  t = clock();
  fprintf(stderr, "%g secs.\n", (float)(t-last)/(float)CLOCKS_PER_SEC);
  last = t;

  
  fprintf(stderr, "Initializing tone mapping ... "); fflush(stderr);

  InitToneMapping();

  t = clock();
  fprintf(stderr, "%g secs.\n", (float)(t-last)/(float)CLOCKS_PER_SEC);
  last = t;

  printf("Stats: total_emitted_power .............: %f W\n"
	 "       estimated_average_illuminance ...: %f W/sr\n"
	 "       average_reflectivity ............: %f\n"
	 "       max_selfemitted_radiance ........: %f W/sr\n"
	 "       max_selfemitted_power ...........: %f W\n"
	 "       adaptation_luminance ............: %f cd/m2\n"
	 "       total_area ......................: %f m2\n",
	 ColorGray(total_emitted_power),
	 ColorGray(estimated_average_radiance),
	 ColorGray(average_reflectivity),
	 ColorGray(max_selfemitted_radiance),
	 ColorGray(max_selfemitted_power),
	 tmopts.lwa,
	 total_area);

  
  if (oRadiance) {
    fprintf(stderr, "Initializing radiance computations ... "); fflush(stderr);

    SetRadianceMethod(oRadiance);

    t = clock();
    fprintf(stderr, "%g secs.\n", (float)(t-last)/(float)CLOCKS_PER_SEC);
    last = t;  
  }

  if (oRayTracing)
  {
    fprintf(stderr, "Initializing raytracing computations ... \n");

    SetRayTracing(oRayTracing);

    t = clock();
    fprintf(stderr, "%g secs.\n", (float)(t-last)/(float)CLOCKS_PER_SEC);
    last = t;  
  }

  
  RemoveAllRenderHooks();

  
  InitPathDebug();

  fprintf(stderr, "Initialisations done.\n");
  UpdateFileStats();

  
#ifndef LONG_FNTITLE
  {
    char *slash = strrchr(filename, '/');
    SetWindowTitle(slash ? slash+1  : filename);
  }
#else
  SetWindowTitle(filename);
#endif

  return True;	
}


void Init(void)
{
  signal(SIGALRM, default_alarm_handler);

  
  FixCubatureRules();

  monochrome = DEFAULT_MONOCHROME;
  force_onesided_surfaces = DEFAULT_FORCE_ONESIDEDNESS;
  nqcdivs = DEFAULT_NQCDIVS;

  MgfDefaults();
  RenderingDefaults();
  ToneMapDefaults();
  CameraDefaults();
  RadianceDefaults();
  RayTracingDefaults();
  InterfaceDefaults();
  IpcDefaults();

  
  VertexSetCompareFlags(VCMP_LOCATION | VCMP_NORMAL);

  
  BBoxSetVertexCompareRoutine((BBOX_COMPARE_FUNC)VertexCompare);
  BBoxSetVertexCompareLocationRoutine((BBOX_COMPARE_FUNC)VertexCompareLocation);
}

static void SetSeed(void *pseed)
{
  int seedval = *(int *)pseed;
  srand48(seedval);
}

static void ReadMGFFromStdin(void *bla)
{
  ReadFile("#.mgf");
}

static void ShowUsage(void *bla);

static int yes=1, no=0;

static void ForceOnesidedOption(void *value)
{
  force_onesided_surfaces = *(int *)value;
}

static void MonochromeOption(void *value)
{
  monochrome = *(int *)value;
}

static CMDLINEOPTDESC globalOptions[] = {
  {"-mgf", 	3,	TYPELESS, 	NULL, 	ReadMGFFromStdin,
   "-mgf           \t\t: read MGF file from standard input"},
  {"-nqcdivs",  3, 	Tint,		&nqcdivs, DEFAULT_ACTION,
   "-nqcdivs <integer>\t: number of quarter circle divisions"},
  {"-force-onesided", 10, TYPELESS,	&yes,   ForceOnesidedOption,
   "-force-onesided\t\t: force one-sided surfaces"},
  {"-dont-force-onesided", 14, TYPELESS,  &no,   ForceOnesidedOption,
   "-dont-force-onesided\t: allow two-sided surfaces"},
  {"-monochromatic", 5,	TYPELESS,	&yes,   MonochromeOption,
   "-monochromatic \t\t: convert colors to shades of grey"},
  {"-seed", 	2,	Tint, 		NULL, 	SetSeed, 
   "-seed <integer>\t\t: set seed for random number generator"},
  {"-help", 	2,	TYPELESS,	NULL,	ShowUsage,
   "-help          \t\t: show program usage and command line options"},
  {NULL	, 	0,	TYPELESS, 	NULL, 	DEFAULT_ACTION,
   NULL }
};


static void ShowUsage(void *bla)
{
  printf("Usage: %s [options] [filename]\n", progname);

  printf("\nGeneral options:\n");
  PrintOptions(stdout, globalOptions);
  PrintMgfOptions(stdout);
  PrintCameraOptions(stdout);
  PrintRenderingOptions(stdout);
  PrintToneMapOptions(stdout);
  PrintRadianceOptions(stdout);
  PrintRayTracingOptions(stdout);
  PrintInterfaceOptions(stdout);
  PrintIpcOptions(stdout);

  exit(0);
}


static void ParseGlobalOptions(int *argc, char **argv)
{
  
  progname = strdup(argv[0]);

  ParseMgfOptions(argc, argv);
  ParseRenderingOptions(argc, argv);
  ParseToneMapOptions(argc, argv);
  ParseCameraOptions(argc, argv);
  ParseRadianceOptions(argc, argv);
  ParseRayTracingOptions(argc, argv);
  ParseInterfaceOptions(argc, argv);
  ParseIpcOptions(argc, argv);

  ParseOptions(globalOptions, argc, argv); 	
}

int main(int argc, char **argv)
{
  
  Init();

  
  ParseGlobalOptions(&argc, argv);

  StartUserInterface(&argc, argv);	
  return 0;
}

