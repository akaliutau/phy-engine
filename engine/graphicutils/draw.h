/* draw.h: Small Graphics Library */

#ifndef _DRAW_H_
#define _DRAW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "draw_context.h"

/* creates, destroys an DRAW rendering context. drawOpen() also makes the new context
 * the current context. */
extern DRAW_CONTEXT *drawOpen(int width, int height);
extern void drawClose(DRAW_CONTEXT *context);

/* makes the specified context current, returns the previous current context */
extern DRAW_CONTEXT *drawMakeCurrent(DRAW_CONTEXT *context);

/* returns current draw renderer */
#define drawGetCurrent()	(current_draw_context)

/* all the following operate on the current DRAW context and behave very similar as
 * the corresponding functions in OpenGL. */
extern void drawClearFrameBuffer(DRAW_PIXEL backgroundcol);
extern void drawClearZBuffer(DRAW_ZVAL defzval);
extern void drawClear(DRAW_PIXEL backgroundcol, DRAW_ZVAL defzval);
extern void drawDepthTesting(DRAW_BOOLEAN on);
extern void drawClipping(DRAW_BOOLEAN on);
extern void drawPushMatrix(void);
extern void drawPopMatrix(void);
extern void drawLoadMatrix(TRANSFORM xf);
extern void drawMultMatrix(TRANSFORM xf);
extern void drawSetColor(DRAW_PIXEL col);
extern void drawViewport(int x, int y, int width, int height);
extern void drawDepthRange(double near, double far);
extern void drawPolygon(int nrverts, VECTOR *verts);

/* returns pixel at position (x,y) */
#define drawGetPixel(x, y) (current_draw_context->fbuf[(current_draw_context->height-1-y) * current_draw_context->width + x])

#ifdef __cplusplus
}
#endif

#endif /*_DRAW_H_*/
