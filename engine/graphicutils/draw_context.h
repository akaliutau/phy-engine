/* draw_context.h: DRAW context type declaration */

#ifndef _DRAW_CONTEXT_H_
#define _DRAW_CONTEXT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "transform.h"

typedef unsigned long DRAW_PIXEL;
typedef unsigned long DRAW_ZVAL;
typedef int DRAW_BOOLEAN;

/* minimum and maximum Z values possible */
#define DRAW_ZMAX 4294967295U
#define DRAW_ZMIN 0

#define DRAW_TRANSFORM_STACK_SIZE 4

typedef struct DRAW_CONTEXT {
  DRAW_PIXEL *fbuf;		/* framebuffer pointer */
  DRAW_ZVAL *zbuf;		/* Z buffer */
  int width, height;		/* canvas size. */
  TRANSFORM transform_stack[DRAW_TRANSFORM_STACK_SIZE];	/* transform stack */
  TRANSFORM *curtrans;		/* current transform */
  DRAW_PIXEL curpixel;		/* current pixel value */
  DRAW_BOOLEAN clipping;		/* whether to do clipping or not */
  int vp_x, vp_y, vp_width, vp_height; /* viewport */
  double near, far;		/* depth range */
} DRAW_CONTEXT;

extern DRAW_CONTEXT *current_draw_context;	/* current context */

#ifdef __cplusplus
}
#endif

#endif /*_DRAW_CONTEXT_H_*/
