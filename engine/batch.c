

#include <time.h>

#include "batch.h"
#include "options.h"
#include "statistics.h"
#include "radiance.h"
#include "raytracing.h"
#include "raycasting.h"
#include "canvas.h"
#include "camera.h"
#include "render.h"
#include "scene.h"
#include "ui.h"
#include "file.h"
#include "error.h"
#include "imageutil/image.h"

static int iterations = 1;	
static int save_modulo = 10;	
static int timings = FALSE;

static char *radiance_image_filename_format = "";
static char *radiance_model_filename_format = "";
static char *raytracing_image_filename = "";

static CMDLINEOPTDESC batchOptions[] = {
  {"-iterations",			3,	Tint,
   &iterations,				DEFAULT_ACTION,
   "-iterations <integer>\t: world-space radiance iterations"},
  {"-radiance-image-savefile",		12,	Tstring,
   &radiance_image_filename_format,	DEFAULT_ACTION,
   "-radiance-image-savefile <filename>\t: radiance PPM/LOGLUV savefile name,"
   "\n\tfirst '%%d' will be substituted by iteration number"},
  {"-radiance-model-savefile",		12,	Tstring,
   &radiance_model_filename_format,	DEFAULT_ACTION,
   "-radiance-model-savefile <filename>\t: radiance VRML model savefile name,"
   "\n\tfirst '%%d' will be substituted by iteration number"},
  {"-save-modulo",			8,	Tint,
   &save_modulo,			DEFAULT_ACTION,
   "-save-modulo <integer>\t: save every n-th iteration"},
  {"-raytracing-image-savefile",	14,	Tstring,
   &raytracing_image_filename,		DEFAULT_ACTION,
   "-raytracing-image-savefile <filename>\t: raytracing PPM savefile name"},
  {"-timings",			3,	Tsettrue,
   &timings,				DEFAULT_ACTION,
   "-timings\t: print timings for world-space radiance and raytracing methods"},
  {NULL	,				0,	TYPELESS,
   NULL,	DEFAULT_ACTION,
   NULL }
};

void ParseBatchOptions(int *argc, char **argv)
{
  ParseOptions(batchOptions, argc, argv);
}

void PrintBatchOptions(FILE *fp)
{
  fprintf(fp, "\nBatch processing options:\n");
  PrintOptions(fp, batchOptions);
}


static void
BatchProcessFile(char *filename, char *open_mode, 
		 void (*process_file)(char *filename, FILE *fp, int ispipe))
{
  int ispipe;
  FILE *fp = OpenFile(filename, open_mode, &ispipe);

  
  process_file(filename, fp, ispipe);

  CloseFile(fp, ispipe);
}

static void BatchSaveRadianceImage(char *fname, FILE *fp, int ispipe)
{
  clock_t   t;
  char    * extension;

  if (!fp) return;

  CanvasPushMode(CANVASMODE_WORKING);

  extension = ImageFileExtension(fname);
  if (IS_TIFF_LOGLUV_EXT(extension))
  {
    fprintf(stdout, "Saving LOGLUV image to file '%s' ....... ", fname);
  }
  else
  {
    fprintf(stdout, "Saving RGB image to file '%s' .......... ", fname);
  }
  fflush(stdout);

  t = clock();

  SaveScreen(fname, fp, ispipe);

  fprintf(stdout, "%g secs.\n", (float)(clock() - t) / (float)CLOCKS_PER_SEC);
  CanvasPullMode();
}

static void BatchSaveRadianceModel(char *fname, FILE *fp, int ispipe)
{
  clock_t t;

  if (!fp) return;

  CanvasPushMode(CANVASMODE_WORKING);
  fprintf(stdout, "Saving VRML model to file '%s' ... ", fname); fflush(stdout);
  t = clock();


  fprintf(stdout, "%g secs.\n", (float)(clock() - t) / (float)CLOCKS_PER_SEC);
  CanvasPullMode();
}



static void BatchSaveRaytracingImage(char *fname, FILE *fp, int ispipe)
{
  ImageOutputHandle *img = NULL;
  clock_t t;

  if (!fp) return;

  t = clock();

  if (fp) {
    img = CreateRadianceImageOutputHandle(fname, fp, ispipe, 
					  Camera.hres, Camera.vres, reference_luminance/179.);
    if (!img) return;
  }

  if (!RayTracing)
    Warning(NULL, "No ray tracing method active");
  else if (!RayTracing->SaveImage || !RayTracing->SaveImage(img))
    Warning(NULL, "No previous %s image available", RayTracing->fullName);

// if (img)
//    delete img;

  fprintf(stdout, "Raytrace save image: %g secs.\n", (float)(clock() - t) / (float)CLOCKS_PER_SEC);

  return;
}

static void BatchRayTrace(char *filename, FILE *fp, int ispipe)
{
  renderopts.render_raytraced_image = TRUE;
  Camera.changed = FALSE;

  CanvasPushMode(CANVASMODE_RENDER);
  RayTrace(filename, fp, ispipe);
  CanvasPullMode();
}

void Batch(void)
{
  clock_t start_time, wasted_start;
  float wasted_secs;

  if (!World) {
    printf("Empty world??\n");
    return;
  }

  start_time = clock();
  wasted_secs = 0.0;

  if (Radiance) {
    
    int it=0, done=FALSE;

    printf("Doing %s ...\n", Radiance->fullName);
    
    fflush(stdout);
    fflush(stderr);
    
    while (!done) {
      printf("-----------------------------------\n"
	     "World-space radiance iteration %04d\n"
	     "-----------------------------------\n\n", it);
      
      CanvasPushMode(CANVASMODE_WORKING);
      done = Radiance->DoStep();
      CanvasPullMode();
      
      fflush(stdout);
      fflush(stderr);
      
      printf(Radiance->GetStats());
      
      fflush(stdout);
      fflush(stderr);

      RenderScene();
      
      fflush(stdout);
      fflush(stderr);

      wasted_start = clock();
      
      if ((! (it%save_modulo)) && *radiance_image_filename_format)
      {
	char *fname = Alloc(strlen(radiance_image_filename_format)+1);
	sprintf(fname, radiance_image_filename_format, it);
	if (renderopts.trace)
	{
	  char * dot;
	  char * tmpName;
	  char * tiffExt = "tif";
	  
	  BatchProcessFile(fname, "w", BatchSaveRadianceImage);
	  
	  
	  
	  tmpName = malloc(strlen(fname)+strlen(tiffExt)+1);
	  strcpy(tmpName, fname);
	  dot = ImageFileExtension(tmpName);
	  if (dot) *dot = '\0';
	  strcat(tmpName,tiffExt);
	  BatchProcessFile(tmpName, "w", BatchSaveRadianceImage);
	  free(tmpName);
	}
	else
	{
	  BatchProcessFile(fname, "w", BatchSaveRadianceImage);
	}
      }
      
      if (*radiance_model_filename_format)
      {
	char *fname = Alloc(strlen(radiance_model_filename_format)+1);
	sprintf(fname, radiance_model_filename_format, it);
	BatchProcessFile(fname, "w", BatchSaveRadianceModel);
      }

      wasted_secs += (float)(wasted_start - clock()) / (float)CLOCKS_PER_SEC;
      
      fflush(stdout);
      fflush(stderr);
      
      it++;
      if (iterations > 0 && it >= iterations)
	done = TRUE;
    }
  } else
    printf("(No world-space radiance computations are being done)\n");
  
  
  if(timings) {
    fprintf(stdout, "Radiance total time %g secs.\n", 
	    ((float)(clock() - start_time) / (float)CLOCKS_PER_SEC) - wasted_secs);
  }


  if (RayTracing) {
    printf("Doing %s ...\n", RayTracing->fullName);

    start_time = clock();
    BatchRayTrace(NULL, NULL, FALSE);
    if(timings) {
      fprintf(stdout, "Raytracing total time %g secs.\n", 
	      (float)(clock() - start_time) / (float)CLOCKS_PER_SEC);
    }

    BatchProcessFile(raytracing_image_filename, "w", BatchSaveRaytracingImage);
  } else
    printf("(No pixel-based radiance computations are being done)\n");


  printf("Computations finished.\n");
}

char *
ImageFileExtension(char *fname)
{
  char *ext = fname + strlen(fname)-1;	

  while (ext>=fname && *ext!='.')
    ext--;

  if ((!strcmp(ext, ".Z"))   || 
      (!strcmp(ext, ".gz"))  ||
      (!strcmp(ext, ".bz"))  ||
      (!strcmp(ext, ".bz2")))
  { 
    ext--;				
    while (ext>=fname && *ext!='.')
      ext--;				
  }

  return ext+1;				
}


