





#include <time.h>

#include "raymatting.h"
#include "options.h"
#include "render.h"
#include "raytracing/screenbuffer.H"
#include "pools.h"
#include "scene.h"
#include "radiance.h"
#include "error.h"
#include "camera.h"
#include "statistics.h"
#include "ui.h"
#include "raytools.H"
#include "rmoptions.H"
#include "raymatting_priv.h"
#include "../graphicutils/draw.h"
#include "pixelfilter.H"

RM_State rms;

class RayMatter {
protected:
  CScreenBuffer *scrn;
  bool interrupt_requested;
  bool doDeleteScreen;
  pixelFilter *Filter;

public:
  RayMatter(CScreenBuffer *inscrn = NULL)
  {
    if(inscrn == NULL)
    {
      scrn = new CScreenBuffer;
      doDeleteScreen = false;
    }
    else
    {
      scrn = inscrn;
      doDeleteScreen = false;
    }

    Filter = NULL;
    interrupt_requested = false;
    scrn->SetRGBImage(true);
  }

  ~RayMatter(void)
  {
    if(doDeleteScreen)
      delete scrn;
    if(Filter)
      delete Filter;
  }

  void CheckFilter(void)
  {
    if(Filter) delete Filter;
      
    if(rms.filter == RM_BOX_FILTER) Filter = new BoxFilter;
    if(rms.filter == RM_TENT_FILTER) Filter = new TentFilter;
    if(rms.filter == RM_GAUSS_FILTER) Filter = new NormalFilter;
    if(rms.filter == RM_GAUSS2_FILTER) Filter = new NormalFilter(.5, 1.5);
  }

  void Matting(void)
  {
    clock_t t = clock();
    interrupt_requested = false;
    COLOR matte;

    CheckFilter();

    long width = Camera.hres;
    long height = Camera.vres;

    // for every pixel
    for (long y=0; y<height; y++) 
    {
      for (long x=0; x<width; x++) 
      {
	float hits = 0;
	  
	for(int i = 0; i<rms.samplesPerPixel; i++) 
	{
	  // uniform random var
	  double xi1 = drand48();
	  double xi2 = drand48();
	  
	  // insert non-uniform sampling here
	  Filter->sample(&xi1, &xi2);

	  // generate ray
	  RAY ray;
	  ray.pos = Camera.eyep;
	  ray.dir = scrn->GetPixelVector(x,y, xi1, xi2);
	  VECTORNORMALIZE(ray.dir);
	    
	  // check if hit
	  if(FindRayIntersection(&ray, NULL, NULL, NULL) != NULL)
	    hits++;
	}

	// add matte value to screenbuffer
	float value = (hits / rms.samplesPerPixel);
	if(value > 1.) value = 1.;

	COLORSET(matte, value, value, value);
	scrn->Add(x, y, matte);
      }

      scrn->RenderScanline(y);
      if (y%10 == 0)
	ProcessWaitingEvents();
      if (interrupt_requested) break;
    }

    rt_total_time = (float)(clock() - t) / (float)CLOCKS_PER_SEC;
    rt_raycount = rt_pixcount = 0;
  }

  double adaptation_luminance(float scalefactor)
  {
    return scrn->AdaptationLuminance(scalefactor);
  }

  void recompute_display_colors(void) 
  {
    scrn->Sync();
  }

  void display(void)
  {
    scrn->Render();
  }

  void reproject(void)
  {
    scrn->Reproject();
  }

  void save(ImageOutputHandle *ip)
  {
    scrn->WriteFile(ip);
  }

  void interrupt(void)
  {
    interrupt_requested = true;
  }
};



void RayMatte(char *fname, FILE *fp, int ispipe)
{
  ImageOutputHandle *img = NULL;

  if (fp) {
    img = CreateRadianceImageOutputHandle(fname, fp, ispipe, 
					  Camera.hres, Camera.vres, reference_luminance/179.);
    if (!img) return;
  }

  RayMatter *rm = new RayMatter;
  rm->Matting();
  if (img) rm->save(img);
  delete rm;

//  if (img)
//    DeleteImageOutputHandle(img);
}


static RayMatter *rm = NULL;

static void Initialize(void)
{
  // do nothing
}

static void IRayMatte(ImageOutputHandle *ip)
{
  if (rm) delete rm;
  rm = new RayMatter();
  rm->Matting();
  if (ip) rm->save(ip);
}


static void RecomputeDisplayColors(void)
{
  if (rm) rm->recompute_display_colors();
}


static int Redisplay(void)
{
  if (!rm)
    return FALSE;

  rm->display();    
  return TRUE;
}

static int Reproject(void)
{
  if (!rm)
    return FALSE;

  rm->reproject();    
  return TRUE;
}

static int SaveImage(ImageOutputHandle *ip)
{
  if (!rm)
    return FALSE;

  rm->save(ip);
  return TRUE;
}

static void Interrupt(void)
{
  if (rm) rm->interrupt();
}

static double AdaptationLuminance(float scale)
{
  if (rm) 
    return rm->adaptation_luminance(scale);
  else
    return 1.0;
}

static void Terminate(void)
{
  if (rm) delete rm;
  rm = NULL;
}

RAYTRACINGMETHOD RayMatting = {
  "RayMatting", 4,
  "Ray Matting",
  "rmatteButton",
  RM_Defaults,
  CreateRMControlPanel,
  RM_ParseOptions,
  RM_PrintOptions,
  Initialize,
  IRayMatte,
  AdaptationLuminance,
  RecomputeDisplayColors,
  Redisplay,
  Reproject,
  SaveImage,
  Interrupt,
  RM_ShowControlPanel,
  RM_HideControlPanel,
  Terminate
};

