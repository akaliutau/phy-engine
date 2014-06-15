/* render.h: interface to the graphics library for rendering.
 * graphics library dependent routines are in render.c. Graphics library independent routines
 * are in rendercommon.c. */

#ifndef _RENDER_H_
#define _RENDER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <Xm/Xm.h>
#include "color.h"
#include "camera.h"
#include "vectortype.h"
#include "patch_type.h"

/* This function determines whether or not a non-default X visual and/or
 * colormap is needed to render in the canvas window on the given screen. If so,
 * the function returns True and fills in the XVIsualInfo and Colormap. If not,
 * the function doesn't fill in the XVisualInfo and Colormap and returns False. */
extern Boolean RenderGetVisualInfoAndColormap(Screen *screen, 
					      XVisualInfo *visual_info,
					      Colormap *colormap);

/* create a window for rendering into. Returns the created widget. */
extern Widget RenderCreateWindow(Widget parent);

/* prepares for rendering in the given window on the given display with given
 * visual information */
extern void RenderInitWindow(Display *display, Window window, XVisualInfo *visinfo);


/* creates an offscreen window for rendering */
extern void RenderCreateOffscreenWindow(int hres, int vres);

/* returns FALSE until rendering is fully initialized. Do not attempt to
 * render something until this function returns TRUE. */
extern int RenderInitialized(void);

/* renders the whole scene */
extern void RenderScene(void);

/* Patch ID rendering. Returns an array of size (*x)*(*y) containing the IDs of
 * the patches visible through each pixel or 0 if the background is visible through 
 * the pixel. x is normally the width and y the height of the canvas window. */
extern unsigned long *RenderIds(long *x, long *y);

/* Renders an image of m lines of n pixels at column x on row y (= lower
 * left corner of image, relative to the lower left corner of the window) */
extern void RenderPixels(int x, int y, int n, int m, RGB *rgb);

/* sets the current color for line or outline drawing */
extern void RenderSetColor(RGB *rgb);

/* renders a convex polygon flat shaded in the current color */
extern void RenderPolygonFlat(int nrverts, POINT *verts);

/* renders a convex polygon with Gouraud shading */
extern void RenderPolygonGouraud(int nrverts, POINT *verts, RGB *vertcols);

/* renders the outline of the given patch in the current color */
extern void RenderPatchOutline(PATCH *patch);

/* renders the all the patches using default colors */
extern void RenderPatch(PATCH *patch);

/* renders normals of patch vertices (rendercommon.c) */
  void RenderPatchNormals(PATCH *patch);

/* renders a line from point p to point q, for eg debugging */
extern void RenderLine(VECTOR *p, VECTOR *q);

/* renders an anti aliased line from p to q. If
   anti-aliasing is not supported, a normal line is drawn */
extern void RenderAALine(VECTOR *p, VECTOR *q);

/* renders an arrow in q for a line from point p, the arrow
   size is always about the same size on screen, independent
   of the distance. The arrow is drawn using AA lines */
extern void RenderArrow(VECTOR *p, VECTOR *q);

/* renders a point (small circle, at the specified location */
extern void RenderPoint(VECTOR *p);

  /* triangle strip rendering */

  /* Start a strip */
extern void RenderBeginTriangleStrip(void);

  /* Supply the next point (one at a time) */
extern void RenderNextTrianglePoint(POINT *point, RGB *col);

  /* Supply N points (array), still Begin and End must be called */
extern void RenderTriangleStrip(int N,POINT *point, RGB *col);

  /* End a strip */
extern void RenderEndTriangleStrip(void);

/* renders a bounding box. */
extern void RenderBounds(BOUNDINGBOX bounds);

/* renders the bounding boxes of all objects in the scene */
extern void RenderBoundingBoxHierarchy(void);

/* renders the cluster hierarchy bounding boxes */
extern void RenderClusterHierarchy(void);

/* saves a RGB image in the front buffer */
extern void SaveScreen(char *filename, FILE *fp, int ispipe);
  
/* renders alternate camera, virtual screen etc ... for didactical pictures etc.. */
extern void RenderCameras(void);

/* rerenders last raytraced image if any, Returns TRUE if there is one,
 * and FALSE if not. */
extern int RenderRayTraced(void);

/* sets line width for outlines etc... */
extern void RenderSetLineWidth(float width);

/* traverses the patches in the scene in such a way to obtain
 * hierarchical view frustum culling + sorted (large patches first +
 * near to far) rendering. For every patch that is not culled,
 * render_patch is called. */
extern void RenderWorldOctree(void (*render_patch)(PATCH *));

/* display background, no Z-buffer, fill whole screen */
extern void RenderBackground(CAMERA *cam);
extern int InitBackground(FILE *fp);

/* rendering options */
typedef struct RENDEROPTIONS {
	RGB 	outline_color,	  /* color in which to draw outlines */
	        bounding_box_color, /* color in which to draw bounding boxes */
                cluster_color,	  /* color in which to show cluster bounding boxes */
	        camera_color;	  /* color for drawing alternate cameras */
	float   camsize,	  /* determines how large alternate cameras are drawn */
	        linewidth;	  /* linewidth */
	char	draw_outlines,    /* True for drawing facet outlines */
                no_shading,       /* False for using any kind of shading */ 
	        smooth_shading,	  /* True for rendering with Gouraud interpolation */
	        backface_culling,	/* true for backface culling */
	        draw_bounding_boxes,	/* true for showing bounding boxes */
                draw_clusters,		/* true for showing cluster
                                         * hierarchy */
	        use_display_lists,	/* true for using display
                                         * lists for faster display */
	        frustum_culling,        /* frustum culling accelerates 
					 * rendering of large scenes. */
	        draw_cameras,		/* true for drawing alternate
                                         * viewpoints */
	        render_raytraced_image, /* for freezing raytraced image on
		                         * the screen when approriate */
		trace,			/* high-dynamic range
                                         * raytraced tiff */
                use_background;         /* use background image when rendering */       
} RENDEROPTIONS;

extern RENDEROPTIONS renderopts;

/* switches backface culling ... on when the argument is nonzero and off
 * if the argument is zero */
extern void RenderSetBackfaceCulling(char truefalse);
extern void RenderSetSmoothShading(char truefalse);
extern void RenderSetOutlineDrawing(char truefalse);
extern void RenderSetBoundingBoxDrawing(char truefalse);
extern void RenderSetClusterDrawing(char truefalse);
extern void RenderUseDisplayLists(char truefalse);
extern void RenderUseFrustumCulling(char truefalse);
extern void RenderSetNoShading(char truefalse);
extern void RenderSetUseBackgroundShading(char truefalse);

/* color for drawing outlines ... */
extern void RenderSetOutlineColor(RGB *outline_color);
extern void RenderSetBoundingBoxColor(RGB *outline_color);
extern void RenderSetClusterColor(RGB *outline_color);

/* computes front- and backclipping plane distance for the current World and 
 * Camera */
extern void RenderGetNearFar(float *near, float *far);

/* indicates that the scene has modified, so a new display list should be
 * compiled and rendered from now on. Only relevant when using display lists. */
extern void RenderNewDisplayList(void);

/* returns only after all issued graphics commands have been processed.
 * RenderScene() already does so, but this function is needed in other
 * circumstances, such as when selecting a patch. */
extern void RenderFinish(void);

/* the following routines are for debugging */
extern void RenderNormals(void);
extern void RenderCoordinateFrame(VECTOR *p, VECTOR *X, VECTOR *Y, VECTOR *Z);


/* command line options, defaults ,.... */
extern void RenderingDefaults(void);
extern void ParseRenderingOptions(int *argc, char **argv);
extern void PrintRenderingOptions(FILE *fp);

#ifdef __cplusplus
}
#endif

#endif /*_RENDER_H_*/
