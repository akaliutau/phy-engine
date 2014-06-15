

#include <math.h>
#include "render.h"
#include "bounds.h"
#include "vector.h"
#include "scene.h"
#include "camera.h"
#include "error.h"
#include "geom.h"
#include "options.h"
#include "defaults.h"
#include "raytracing.h"
#include "spherical.h"

RENDEROPTIONS renderopts;

void RenderSetUseBackground(char yesno)
{
  renderopts.use_background = yesno;
}

void RenderSetSmoothShading(char yesno)
{
  renderopts.smooth_shading = yesno;
}

void RenderSetNoShading(char yesno)
{
  renderopts.no_shading = yesno;
}

void RenderSetBackfaceCulling(char backface_culling)
{
  renderopts.backface_culling = backface_culling;
}

void RenderSetOutlineDrawing(char draw_outlines)
{
  renderopts.draw_outlines = draw_outlines;
}

void RenderSetBoundingBoxDrawing(char yesno)
{
  renderopts.draw_bounding_boxes = yesno;
}

void RenderSetClusterDrawing(char yesno)
{
  renderopts.draw_clusters = yesno;
}

void RenderUseDisplayLists(char truefalse)
{
  renderopts.use_display_lists = truefalse;
}

void RenderUseFrustumCulling(char truefalse)
{
  renderopts.frustum_culling = truefalse;
}

void RenderSetOutlineColor(RGB *outline_color)
{
  renderopts.outline_color = *outline_color;
}

void RenderSetBoundingBoxColor(RGB *outline_color)
{
  renderopts.bounding_box_color = *outline_color;
}

void RenderSetClusterColor(RGB *cluster_color)
{
  renderopts.cluster_color = *cluster_color;
}

static void DisplayListsOption(void *value)
{
  renderopts.use_display_lists = TRUE;
}

static void FlatOption(void *value)
{
  renderopts.smooth_shading = FALSE;
}

static void NoCullingOption(void *value)
{
  renderopts.backface_culling = FALSE;
}

static void OutlinesOption(void *value)
{
  renderopts.draw_outlines = TRUE;
}

static void TraceOption(void *value)
{
  renderopts.trace = TRUE;
}

static CMDLINEOPTDESC renderingOptions[] = {
  {"-display-lists",               10, TYPELESS,
   NULL,                           DisplayListsOption,
   "-display-lists\t\t"
   ": use display lists for faster hardware-assisted rendering"},
  {"-flat-shading",                5,  TYPELESS,
   NULL,                           FlatOption,
   "-flat-shading\t\t: render without Gouraud (color) interpolation"},
  {"-raycast",                5,  TYPELESS,
   NULL,                           TraceOption,
   "-raycast\t\t: save raycasted scene view as a high dynamic range image"},
  {"-no-culling",                  5,  TYPELESS,
   NULL,                           NoCullingOption,
   "-no-culling\t\t: don't use backface culling"},
  {"-outlines",                    5,  TYPELESS,
   NULL,                           OutlinesOption,
   "-outlines\t\t: draw polygon outlines"},
  {"-outline-color",               10, TRGB,
   &renderopts.outline_color,      DEFAULT_ACTION,
   "-outline-color <rgb> \t: color for polygon outlines"},
  {NULL,                           0, TYPELESS,
   NULL,                           DEFAULT_ACTION,
   NULL }
};

void RenderingDefaults(void)
{
  RGB    outlinecolor = DEFAULT_OUTLINE_COLOR,
         bbcolor = DEFAULT_BOUNDING_BOX_COLOR,
         cluscolor = DEFAULT_CLUSTER_COLOR;

  RenderUseDisplayLists(DEFAULT_DISPLAY_LISTS);
  RenderSetSmoothShading(DEFAULT_SMOOTH_SHADING);
  RenderSetBackfaceCulling(DEFAULT_BACKFACE_CULLING);
  RenderSetOutlineDrawing(DEFAULT_OUTLINE_DRAWING);
  RenderSetBoundingBoxDrawing(DEFAULT_BOUNDING_BOX_DRAWING);
  RenderSetClusterDrawing(DEFAULT_CLUSTER_DRAWING);
  RenderSetOutlineColor(&outlinecolor);
  RenderSetBoundingBoxColor(&bbcolor);
  RenderSetClusterColor(&cluscolor);
  RenderUseFrustumCulling(TRUE);

  RenderSetNoShading(FALSE);

  renderopts.draw_cameras = FALSE;
  renderopts.camsize = 0.25;
  renderopts.linewidth = 1.0;
  renderopts.camera_color = Yellow;

  renderopts.render_raytraced_image = FALSE;
  renderopts.use_background = TRUE;
}

void ParseRenderingOptions(int *argc, char **argv)
{
  ParseOptions(renderingOptions, argc, argv);
}

void PrintRenderingOptions(FILE *fp)
{
  fprintf(fp, "\nRendering options:\n");
  PrintOptions(fp, renderingOptions);
}

extern CAMERA AlternateCamera;


void RenderGetNearFar(float *near, float *far)
{
  BOUNDINGBOX bounds;
  VECTOR b[2], d;
  int i, j, k;
  float z;

  if (!World) {
    *far = 10.; *near = 0.1;	
    return;
  }
  GeomListBounds(World, bounds);

  VECTORSET(b[0], bounds[MIN_X], bounds[MIN_Y], bounds[MIN_Z]);
  VECTORSET(b[1], bounds[MAX_X], bounds[MAX_Y], bounds[MAX_Z]);
  
  *far = -HUGE; *near = HUGE;
  for (i=0; i<=1; i++)
    for (j=0; j<=1; j++)
      for (k=0; k<=1; k++) {
	VECTORSET(d, b[i].x, b[j].y, b[k].z);
	VECTORSUBTRACT(d, Camera.eyep, d);
	z = VECTORDOTPRODUCT(d, Camera.Z);
	
	if (z > *far) *far = z;
	if (z < *near) *near = z;
      }

  if (renderopts.draw_cameras) {
    CAMERA *cam = &AlternateCamera;
    float camlen = renderopts.camsize,
      hsiz = camlen * cam->viewdist * cam->tanhfov, 
      vsiz = camlen * cam->viewdist * cam->tanvfov;
    POINT c, P[5];

    VECTORCOMB2(1., cam->eyep, camlen * cam->viewdist, cam->Z, c);
    VECTORCOMB3(c,  hsiz, cam->X,  vsiz, cam->Y, P[0]);
    VECTORCOMB3(c,  hsiz, cam->X, -vsiz, cam->Y, P[1]);
    VECTORCOMB3(c, -hsiz, cam->X, -vsiz, cam->Y, P[2]);
    VECTORCOMB3(c, -hsiz, cam->X,  vsiz, cam->Y, P[3]);
    P[4] = cam->eyep;

    for (i=0; i<5; i++) {
      VECTORSUBTRACT(P[i], Camera.eyep, d);
      z = VECTORDOTPRODUCT(d, Camera.Z);    
      if (z > *far) *far = z;
      if (z < *near) *near = z;    
    }
  }

  
  *far += 0.02 * (*far); *near -= 0.02 * (*near);
  if (*far < EPSILON) *far = Camera.viewdist;
  if (*near < EPSILON) *near = Camera.viewdist/100.;
}

void RenderBounds(BOUNDINGBOX bounds)
{
  POINT p[8];

  VECTORSET(p[0], bounds[MIN_X], bounds[MIN_Y], bounds[MIN_Z]);
  VECTORSET(p[1], bounds[MAX_X], bounds[MIN_Y], bounds[MIN_Z]);
  VECTORSET(p[2], bounds[MIN_X], bounds[MAX_Y], bounds[MIN_Z]);
  VECTORSET(p[3], bounds[MAX_X], bounds[MAX_Y], bounds[MIN_Z]);
  VECTORSET(p[4], bounds[MIN_X], bounds[MIN_Y], bounds[MAX_Z]);
  VECTORSET(p[5], bounds[MAX_X], bounds[MIN_Y], bounds[MAX_Z]);
  VECTORSET(p[6], bounds[MIN_X], bounds[MAX_Y], bounds[MAX_Z]);
  VECTORSET(p[7], bounds[MAX_X], bounds[MAX_Y], bounds[MAX_Z]);

  RenderLine(&p[0], &p[1]);
  RenderLine(&p[1], &p[3]);
  RenderLine(&p[3], &p[2]);
  RenderLine(&p[2], &p[0]);
  RenderLine(&p[4], &p[5]);
  RenderLine(&p[5], &p[7]);
  RenderLine(&p[7], &p[6]);
  RenderLine(&p[6], &p[4]);
  RenderLine(&p[0], &p[4]);
  RenderLine(&p[1], &p[5]);
  RenderLine(&p[2], &p[6]);
  RenderLine(&p[3], &p[7]);
}

void RenderGeomBounds(GEOM *geom)
{
  float *geombounds = GeomBounds(geom);

  if (geom->bounded && geombounds)
    RenderBounds(geombounds);

  if (GeomIsAggregate(geom)) {
    GeomListIterate(GeomPrimList(geom), RenderGeomBounds);
  }
}

void RenderBoundingBoxHierarchy(void)
{
  RenderSetColor(&renderopts.bounding_box_color);
  GeomListIterate(World, RenderGeomBounds);
}

void RenderClusterHierarchy(void)
{
  RenderSetColor(&renderopts.cluster_color);
  GeomListIterate(ClusteredWorld, RenderGeomBounds);
}

int RenderRayTraced(void)
{
  if (!RayTracing || !RayTracing->Redisplay) {
    return FALSE;
  } else {
    return RayTracing->Redisplay();
  }
}

static void RenderNormal(PATCH *p, VERTEX *v, float len)
{
  POINT x;

  if (v && v->normal) {

    RenderSetColor(&Yellow);
    VECTORADDSCALED(*v->point, len, *v->normal, x);
    RenderLine(v->point, &x);

#ifdef NEVER
    HITREC hit;
    VECTOR X,Y,Z; 

    InitHit(&hit, p, NULL, v->point, &p->normal, p->surface->material, 0.);

    if(!HitUV(&hit, &hit.uv))
    {
      fprintf(stderr, "NO UV\n");
    }

    // Draw a shading frame
    HitShadingFrame(&hit, &X, &Y, &Z);
    
    
    
    RenderCoordinateFrame(v->point, &X, &Y, &Z);
#endif
  }
}


void RenderPatchNormals(PATCH *patch)
{
  float length;
  float *bbx;
  VECTOR diagonal;
  
  if(patch == NULL) return;

  if(WorldGrid)
  {
	bbx = WorldGrid->bounds;
	VECTORSET(diagonal,
			  bbx[MAX_X] -  bbx[MIN_X],   
			  bbx[MAX_Y] -  bbx[MIN_Y],
			  bbx[MAX_Z] -  bbx[MIN_Z] );
	length = VECTORNORM(diagonal) / 20.;
	
	RenderSetColor(&Yellow);

	RenderNormal(patch, patch->vertex[0], length);
	RenderNormal(patch, patch->vertex[1], length);
	RenderNormal(patch, patch->vertex[2], length);
	if(patch->nrvertices == 4)
	  RenderNormal(patch, patch->vertex[3], length);
  }
}

void RenderNormals(void)
{
  ForAllPatches(P, Patches) {
	RenderPatchNormals(P);
  } EndForAll;
}

void RenderCoordinateFrame(VECTOR *p, VECTOR *X, VECTOR *Y, VECTOR *Z)
{
  VECTOR q;
  VECTORADD(*p, *X, q);
  RenderSetColor(&Red);
  RenderLine(p, &q);
  VECTORADD(*p, *Y, q);
  RenderSetColor(&Green);
  RenderLine(p, &q);
  VECTORADD(*p, *Z, q);
  RenderSetColor(&Blue);
  RenderLine(p, &q);
  RenderFinish();
}


#define ARROW_FACTOR 50.0

void RenderArrow(VECTOR *p, VECTOR *q)
{
  VECTOR d, dn, p1, p2;
  float dist, deltaD, deltaDN, sine;
  float Zdist;

  VECTORSUBTRACT(*q, Camera.eyep, d);
  Zdist = VECTORDOTPRODUCT(d, Camera.Z);

  VECTORSUBTRACT(*q,*p,d);
  dist = VECTORNORM(d);

  VECTORSCALE(1/dist, d, d);
  
  VECTORCROSSPRODUCT(d, Camera.Z, dn); 

  sine = VECTORNORM(dn);

  if(sine < EPSILON)
  {
    dn = Camera.X;
  }
  else
  {
    VECTORSCALE(1/sine, dn, dn);
  }

  deltaD = Zdist / ARROW_FACTOR;
  deltaDN = deltaD / 4.0;

  VECTORDIFFSCALED(*q, deltaD, d, p1);

  VECTORDIFFSCALED(p1, deltaDN, dn, p2);
  VECTORADDSCALED(p1, deltaDN, dn, p1);

  RenderAALine(q, &p1);
  RenderAALine(q, &p2);
}
