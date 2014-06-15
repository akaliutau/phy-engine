/* line.h */

#ifndef _LINE_H_
#define _LINE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "poly.h"

typedef struct Line {
	unsigned mask;
	Poly_vert v1, v2;
} Line;

#define LINE_CLIP_OUT 0		/* line entirely outside box */
#define LINE_CLIP_PARTIAL 1	/* line partially inside */
#define LINE_CLIP_IN 2		/* line entirely inside box */

extern int line_clip_to_box(Line *lin, Poly_box *box);
extern void line_scan(Line *line,
		      void (*dotproc)(int x, int y, Poly_vert *v));

#ifdef __cplusplus
}
#endif

#endif /*_LINE_H_*/
