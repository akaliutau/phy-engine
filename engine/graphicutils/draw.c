

#include <string.h>
#include "../graphicutils/poly.h"	
#include "../graphicutils/line.h"
#include "draw.h"
#include "pools.h"
#include "error.h"
#include "extmath.h"

Poly_vert *poly_dummy;		

DRAW_CONTEXT *current_draw_context = (DRAW_CONTEXT *)NULL;

DRAW_CONTEXT *drawOpen(int width, int height)
{
  DRAW_CONTEXT *context = (DRAW_CONTEXT *)Alloc(sizeof(DRAW_CONTEXT));

  current_draw_context = context;

  
  context->width = width;
  context->height = height;
  context->fbuf = (DRAW_PIXEL *)Alloc(width * height * sizeof(DRAW_PIXEL));

  
  context->zbuf = (DRAW_ZVAL *)NULL;

  
  context->curtrans = context->transform_stack;
  *context->curtrans = IdentityTransform;

  context->curpixel = 0;

  context->clipping = TRUE;

  
  context->vp_x = 0;
  context->vp_y = 0;
  context->vp_width = width;
  context->vp_height = height;
  context->near = 0.;
  context->far = 1.;

  return context;
}

void drawClose(DRAW_CONTEXT *context)
{
  Free((char *)context->fbuf, context->width * context->height * sizeof(DRAW_PIXEL));
  if (context->zbuf)
    Free((char *)context->zbuf, context->width * context->height * sizeof(DRAW_ZVAL));
  
  Free((char *)context, sizeof(DRAW_CONTEXT));

  if (context == current_draw_context)
    current_draw_context = (DRAW_CONTEXT *)NULL;
}

DRAW_CONTEXT *drawMakeCurrent(DRAW_CONTEXT *context)
{
  DRAW_CONTEXT *old_context = current_draw_context;
  current_draw_context = context;
  return old_context;
}

void drawClearFrameBuffer(DRAW_PIXEL backgroundcol)
{
  DRAW_PIXEL *pix, *lpix;
  int i, j;

  lpix = current_draw_context->fbuf + current_draw_context->vp_y * current_draw_context->width + current_draw_context->vp_x;
  for (j=0; j<current_draw_context->vp_height; j++, lpix += current_draw_context->width) {
    for (pix = lpix, i=0; i<current_draw_context->vp_width; i++)
      *pix++ = backgroundcol;
  }
}

void drawClearZBuffer(DRAW_ZVAL defzval)
{
  DRAW_ZVAL *zval, *lzval;
  int i, j;

  lzval = current_draw_context->zbuf + current_draw_context->vp_y * current_draw_context->width + current_draw_context->vp_x;
  for (j=0; j<current_draw_context->vp_height; j++, lzval += current_draw_context->width) {
    for (zval = lzval, i=0; i<current_draw_context->vp_width; i++) 
      *zval++ = defzval;
  }
}

void drawClear(DRAW_PIXEL backgroundcol, DRAW_ZVAL defzval)
{
  drawClearFrameBuffer(backgroundcol);
  drawClearZBuffer(defzval);
}

void drawDepthTesting(DRAW_BOOLEAN on)
{
  if (on) {
    if (current_draw_context->zbuf)
      return;
    else {
      current_draw_context->zbuf = (DRAW_ZVAL *)Alloc(current_draw_context->width * current_draw_context->height * sizeof(DRAW_ZVAL));
    }
  } else {
    if (current_draw_context->zbuf) {
      Free((char *)current_draw_context->zbuf, current_draw_context->width * current_draw_context->height * sizeof(DRAW_ZVAL));
      current_draw_context->zbuf = (DRAW_ZVAL *)NULL;
    } else
      return;
  }
}

void drawClipping(DRAW_BOOLEAN on)
{
  current_draw_context->clipping = on;
}

void drawPushMatrix(void)
{
  TRANSFORM *oldtrans;

  if (current_draw_context->curtrans - current_draw_context->transform_stack >= DRAW_TRANSFORM_STACK_SIZE-1) {
    Error("drawPushMatrix", "Matrix stack overflow");
    return;
  }

  oldtrans = current_draw_context->curtrans;
  current_draw_context->curtrans++;
  *current_draw_context->curtrans = *oldtrans;
}

void drawPopMatrix(void)
{
  if (current_draw_context->curtrans <= current_draw_context->transform_stack) {
    Error("drawPopMatrix", "Matrix stack underflow");
    return;
  }

  current_draw_context->curtrans--;
}

void drawLoadMatrix(TRANSFORM xf)
{
  *current_draw_context->curtrans = xf;
}

void drawMultMatrix(TRANSFORM xf)
{
  *current_draw_context->curtrans = TransCompose(*current_draw_context->curtrans, xf);
}

void drawSetColor(DRAW_PIXEL col)
{
  current_draw_context->curpixel = col;
}

void drawViewport(int x, int y, int width, int height)
{
  current_draw_context->vp_x = x;
  current_draw_context->vp_y = y;
  current_draw_context->vp_width = width;
  current_draw_context->vp_height = height;
}

void drawDepthRange(double near, double far)
{
  current_draw_context->near = near;
  current_draw_context->far = far;
}

void drawPolygon(int nrverts, VECTOR *verts)
{
  Poly pol;
  Poly_vert *pv;
  Window win;
  Poly_box clip_box = {-1., 1., -1., 1., -1., 1.};
  int i;

  if (nrverts > (current_draw_context->clipping ? (POLY_NMAX-6) : POLY_NMAX)) {
    Error("drawPolygon", "Too many vertices (max. %d)", POLY_NMAX);
    return;
  }

  
  for (i=0, pv=&pol.vert[0]; i<nrverts; i++, pv++) {
    POINT4D v;
    v.x = verts[i].x; v.y = verts[i].y; v.z = verts[i].z; v.w = 1.; 
    TRANSFORM_POINT_4D(*current_draw_context->curtrans, v, v);
    if (v.w > -EPSILON && v.w < EPSILON) 
      return;
    pv->sx = v.x;
    pv->sy = v.y;
    pv->sz = v.z;
    pv->sw = v.w;
  }
  pol.n = nrverts;
  pol.mask = 0;

  if (current_draw_context->clipping) {
    pol.mask = POLY_MASK(sx) | POLY_MASK(sy) | POLY_MASK(sz) | POLY_MASK(sw);
    if (poly_clip_to_box(&pol, &clip_box) == POLY_CLIP_OUT)
      return;
  }

  
  for (i=0, pv=&pol.vert[0]; i<pol.n; i++, pv++) {
    pv->sx = (double)current_draw_context->vp_x + (pv->sx/pv->sw + 1.) * (double)current_draw_context->vp_width * 0.5;
    pv->sy = (double)current_draw_context->vp_y + (pv->sy/pv->sw + 1.) * (double)current_draw_context->vp_height * 0.5;
    pv->sz = (current_draw_context->near + (pv->sz/pv->sw + 1.) * current_draw_context->far * 0.5) * (double)DRAW_ZMAX;
  }

  
  win.x0 = current_draw_context->vp_x;
  win.y0 = current_draw_context->vp_y;
  win.x1 = current_draw_context->vp_x+current_draw_context->vp_width-1;
  win.y1 = current_draw_context->vp_y+current_draw_context->vp_height-1;

  
  if (!current_draw_context->zbuf)
    poly_scan_flat(&pol, &win);
  else
    poly_scan_z(&pol, &win);
}
