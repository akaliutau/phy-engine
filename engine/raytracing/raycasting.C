

#include <time.h>

#include "raycasting.h"
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
#include "../graphicutils/draw.h"

#include "../photonmapping/pmap.h"

class Hard_ID_Renderer {
protected:
  unsigned long *ids;	// patch IDs visible in each pixel of the screen
  long idh, idw;	// width and height of the screen
  PATCH **id2patch;	// converts patch ID to PATCH *
  unsigned long nrpatchids;	// size of id2patch[] table
  int wgiv;

public:
  Hard_ID_Renderer() {	// also performs the actual ID rendering.
    ids = 0; id2patch = 0; idw = idh = 0; nrpatchids=0;
    wgiv = FALSE;
    init();
  }

  ~Hard_ID_Renderer(void) {
    if (ids) Free((char *)ids, (int)(idw*idh)*sizeof(unsigned long));
    if (id2patch) Free((char *)id2patch, (int)nrpatchids * sizeof(PATCH *));
  }

  void testids(void)
  {
    int i;
    int wgiv = FALSE;
    for (i=0; i<idw*idh; i++) {
      if (ids[i] >= nrpatchids && !wgiv) {
	Error("Hard_ID_Renderer::testids", "Rubbish in id buffer. You'd better recompile with SOFT_ID_RENDERING defined.\n");
	wgiv = TRUE;
      }
    }
  }

  void init(void)
  {
    unsigned long i;
    nrpatchids = PatchGetNextID(); 
    id2patch = (PATCH **)Alloc((int)nrpatchids * sizeof(PATCH *));
    for (i=0; i<nrpatchids; i++)
      id2patch[i] = 0;
    ForAllPatches(P, Patches) {
      id2patch[P->id] = P;
    } EndForAll;

    ids = RenderIds(&idw, &idh);
    
  }

  inline void get_size(long *width, long *height)
  {
    *width = idw;
    *height = idh;
  }

  inline PATCH *get_patch_at_pixel(int x, int y)
  {
    unsigned long id = ids[x + idw*(idh-y-1)];
    if (id<nrpatchids)
      return id2patch[id];
    else {
      if (!wgiv) {
	Error("Hard_ID_Renderer::get_patch_at_pixel", "Rubbish in id buffer. You'd better recompile with SOFT_ID_RENDERING defined.\n");
	wgiv = TRUE;
      }
      return (PATCH *)NULL;
    }
  }
};


#include "../SoftIds.H"

//#ifdef SOFT_ID_RENDERING
//#define ID_Renderer Soft_ID_Renderer
//#else
#define ID_Renderer Hard_ID_Renderer
//#endif

static void ClipUV(int nrvertices, double *u, double *v)
{
  if (*u > 1.-EPSILON) *u = 1.-EPSILON;
  if (*v > 1.-EPSILON) *v = 1.-EPSILON;
  if (nrvertices == 3 && (*u+*v) > 1.-EPSILON) {
    if (*u>*v)
      *u = 1. - *v - EPSILON;
    else
      *v = 1. - *u - EPSILON;
  }
  if (*u < EPSILON) *u = EPSILON;
  if (*v < EPSILON) *v = EPSILON;
}

class RayCaster {
protected:
  CScreenBuffer *scrn;
  bool interrupt_requested;
  bool doDeleteScreen;

  // determines the radiance of the nearest patch visible through the pixel
  // (x,y). P shall be the nearest patch visible in the pixel.
  COLOR get_radiance_at_pixel(int x, int y, PATCH *P, GETRADIANCE_FT getrad)
  {
    COLOR rad;
    COLORCLEAR(rad);
    if (P && getrad) {
      // ray pointing from the eye through the center of the pixel.
      RAY ray;
      ray.pos = Camera.eyep;
      ray.dir = scrn->GetPixelVector(x,y);
      VECTORNORMALIZE(ray.dir);
 
      // find intersection point of ray with patch P
      VECTOR point;
      float dist = VECTORDOTPRODUCT(P->normal, ray.dir);
      dist = - (VECTORDOTPRODUCT(P->normal, ray.pos) + P->plane_constant) / dist;
      VECTORADDSCALED(ray.pos, dist, ray.dir, point);
      
      // find surface coordinates of hit point on patch
      double u, v;	
      PatchUV(P, &point, &u, &v);
      // boundary check is necessary because Z-buffer algorithm does
      // not yield exactly the same result as ray tracing at patch 
      // boundaries.
      ClipUV(P->nrvertices, &u, &v);
      
      // reverse ray direction and get radiance emited at hit point towards the eye.
      VECTOR dir(-ray.dir.x, -ray.dir.y, -ray.dir.z);
      rad = getrad(P, u, v, dir);
    }
    return rad;
  }

public:
  RayCaster(CScreenBuffer *inscrn = NULL)
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
    
    interrupt_requested = false;
  }

  ~RayCaster(void)
  {
    if(doDeleteScreen)
      delete scrn;
  }

  void render(GETRADIANCE_FT getrad = NULL)
  {
    clock_t t = clock();
    interrupt_requested = false;

    if(getrad == NULL)
    {
      if(Radiance)
	getrad = Radiance->GetRadiance;
    }

    long width, height, x, y;
    ID_Renderer *id_renderer = new ID_Renderer;
    id_renderer->get_size(&width, &height);
    if (width != scrn->GetHRes() || 
	height != scrn->GetVRes()) {

      Fatal(-1, "RayCaster::render", "ID buffer size doesn't match screen size");
    }

    for (y=0; y<height; y++) {
      for (x=0; x<width; x++) {
	PATCH *P = id_renderer->get_patch_at_pixel(x, y);
	COLOR rad = get_radiance_at_pixel(x, y, P, getrad);
	scrn->Add(x, y, rad);
      }

      scrn->RenderScanline(y);
      if (y%10 == 0)
	ProcessWaitingEvents();
      if (interrupt_requested) break;
    }

    delete id_renderer;

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

// A static raycaster used in the raycasting method
static RayCaster *s_rc = 0;


// A "backward" approach would be much better than this naive "foreward" approach!
void CScreenBuffer::Reproject(void)
{
  CAMERA newcam = Camera;
  CAMERA oldcam = m_cam;

  Camera = oldcam;		// ID-rendering for the old camera.
  ID_Renderer *oldidr = new ID_Renderer;

  Camera = newcam;              // ID-rendering for the new camera
  ID_Renderer *newidr = new ID_Renderer;

  CScreenBuffer newscrn;	// for the new Camera!!
  newscrn.m_AddFactor = (1./100.);

  int x, y;
  for (y=0; y<m_cam.vres; y++) {
    for (x=0; x<m_cam.hres; x++) {
      PATCH *P = oldidr->get_patch_at_pixel(x,y);
      if (!P) continue;

      float xoff, yoff;
      for (xoff=(1./20.); xoff<1.; xoff+=(1./10.)) {
	for (yoff=(1./20.); yoff<1.; yoff+=(1./10.)) {
	  VECTOR pos = m_cam.eyep;
	  VECTOR dir = GetPixelVector(x,y,xoff,yoff);

	  // compute 3D point visible through the pixel in the (old) view.
	  VECTOR point;
	  float dist = VECTORDOTPRODUCT(P->normal, dir);
	  dist = - (VECTORDOTPRODUCT(P->normal, pos) + P->plane_constant) / dist;
	  VECTORADDSCALED(pos, dist, dir, point);
      
	  // compute pixel in the new view into which this point is projected
	  int nx, ny;
	  VECTOR newdir;
	  VECTORSUBTRACT(point, newscrn.m_cam.eyep, newdir);
	  if (newscrn.GetDirectionPixel(newdir, &nx, &ny) &&
	      newidr->get_patch_at_pixel(nx,ny) == P) {
	    COLOR rad = Get(x,y);
	    // distance and viewing direction correction
	    float s = (dist * dist) / VECTORNORM2(newdir);
	    VECTORNORMALISE(newdir);
	    VECTORNORMALISE(dir);
	    s *= VECTORDOTPRODUCT(P->normal, newdir) / VECTORDOTPRODUCT(P->normal, dir);
	    COLORSCALE(s, rad, rad);
	    newscrn.Add(nx, ny, rad);
	  }
	}
      }
    }
    if (y % (m_cam.vres / 100) == 0) {
      putchar('.');   // progress indicator
      fflush(stdout);
    }
  }
  putchar('\n');

  delete oldidr;
  delete newidr;

  Copy(&newscrn);
  Render();
}


void RayCast(char *fname, FILE *fp, int ispipe)
{
  ImageOutputHandle *img = NULL;

  if (fp) {
    img = CreateRadianceImageOutputHandle(fname, fp, ispipe, 
					  Camera.hres, Camera.vres, reference_luminance/179.);
    if (!img) return;
  }

  RayCaster *rc = new RayCaster;
  rc->render();
  if (img) rc->save(img);
  delete rc;

//  if (img)
//    DeleteImageOutputHandle(img);
}


void RayCast(GETRADIANCE_FT cb, CScreenBuffer *screen)
{
  if(s_rc) delete s_rc;
  s_rc = new RayCaster(screen);
  s_rc->render(cb);
  delete s_rc;
  s_rc = NULL; // -- Delete ??
}


double RayCastAdaptationLuminance(float scalefactor)
{
  double lum;

  RayCaster *rc = new RayCaster;
  rc->render();
  lum = rc->adaptation_luminance(scalefactor);
  delete rc;

  return lum;
}

static void Defaults(void)
{
  
}

static void ParseHWRCastOptions(int *argc, char **argv)
{
  
}

static void PrintHWRCastOptions(FILE *fp)
{
  
}

static void Initialize(void)
{
	
}

static void IRayCast(ImageOutputHandle *ip)
{
  if (s_rc) delete s_rc;
  s_rc = new RayCaster();
  s_rc->render();
  if (ip) s_rc->save(ip);
}


static void RecomputeDisplayColors(void)
{
  if (s_rc) s_rc->recompute_display_colors();
}


static int Redisplay(void)
{
  if (!s_rc)
    return FALSE;

  s_rc->display();    
  return TRUE;
}

static int Reproject(void)
{
  if (!s_rc)
    return FALSE;

  s_rc->reproject();    
  return TRUE;
}

static int SaveImage(ImageOutputHandle *ip)
{
  if (!s_rc)
    return FALSE;

  s_rc->save(ip);
  return TRUE;
}

static void Interrupt(void)
{
  if (s_rc) s_rc->interrupt();
}

static double AdaptationLuminance(float scale)
{
  if (s_rc) 
    return s_rc->adaptation_luminance(scale);
  else
    return 1.0;
}

static void CreateControlPanel(void *parent_widget)
{
  
}

static void ShowControlPanel(void)
{
  
  Warning(NULL, "Ray Casting is fully automatic: there is no control panel");
}

static void HideControlPanel(void)
{
  
}

static void Terminate(void)
{
  if (s_rc) delete s_rc;
  s_rc = 0;
}

RAYTRACINGMETHOD RayCasting = {
  "RayCasting", 4,
  "Ray Casting",
  "rcastButton",
  Defaults,
  CreateControlPanel,
  ParseHWRCastOptions,
  PrintHWRCastOptions,
  Initialize,
  IRayCast,
  AdaptationLuminance,
  RecomputeDisplayColors,
  Redisplay,
  Reproject,
  SaveImage,
  Interrupt,
  ShowControlPanel,
  HideControlPanel,
  Terminate
};

