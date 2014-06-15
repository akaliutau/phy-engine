
#include <math.h>
#include "line.h"




#define ABS(x)	((x) < 0 ? -(x) : (x))
#define SGN(x)	((x) < 0 ? -1 : 1)



static void incrementalize_y(float *p1, float *p2, float *p, float *dp, int y, int mask)
{
    float dy, frac;

    dy = ((Poly_vert *)p2)->sy - ((Poly_vert *)p1)->sy;
    if (dy==0.) dy = 1.;
    frac = y+.5 - ((Poly_vert *)p1)->sy;

    for (; mask!=0; mask>>=1, p1++, p2++, p++, dp++)
		if (mask&1) {
			*dp = (*p2 - *p1) / dy;
			*p = *p1 + *dp * frac;
		}
}



static void incrementalize_x(float *p1, float *p2, float *p, float *dp, int x, int mask)
{
    float dx, frac;

    dx = ((Poly_vert *)p2)->sx - ((Poly_vert *)p1)->sx;
    if (dx==0.) dx = 1.;
    frac = x+.5 - ((Poly_vert *)p1)->sx;

    for (; mask!=0; mask>>=1, p1++, p2++, p++, dp++)
		if (mask&1) {
			*dp = (*p2 - *p1) / dx;
			*p = *p1 + *dp * frac;
		}
}

static void increment(float *v, float *dv, int mask)
{
	for (; mask; mask>>=1, v++, dv++)
		if (mask&1) 
			*v += *dv;	
}

void line_scan(Line *line,
	       void (*dotproc)(int x, int y, Poly_vert *v))
{
	int x1, x2, y1, y2;
	int d, x, y, ax, ay, sx, sy, dx, dy;
	Poly_vert v1, v2, v, dv;

	x1 = (int)line->v1.sx;
	x2 = (int)line->v2.sx;
	y1 = (int)line->v1.sy;
	y2 = (int)line->v2.sy;

	dx = x2-x1;  ax = ABS(dx)<<1;  
	dy = y2-y1;  ay = ABS(dy)<<1;  

	if (ax>ay) {		
		if (x1 > x2) {	
			v1=line->v2; v2=line->v1;
			x=x1; x1=x2; x2=x; dx = -dx; 
			y=y1; y1=y2; y2=y; dy = -dy;
		} else {
			v1=line->v1; v2=line->v2;
		}

		sy = SGN(dy);

		x = x1;
		y = y1;

		incrementalize_x((float *)&v1, (float *)&v2, (float *)&v, (float *)&dv, x1, line->mask);

		d = ay-(ax>>1);
		for (;;) {
			(*dotproc)(x, y, &v);
			increment((float *)&v, (float *)&dv, line->mask);

			if (x==x2) break;
			if (d>=0) {
				y += sy;
				d -= ax;
			}
			x ++;
			d += ay;
		}
	} else {		
		if (y1 > y2) {	
			v1=line->v2; v2=line->v1;
			x=x1; x1=x2; x2=x; dx = -dx; 
			y=y1; y1=y2; y2=y; dy = -dy;
		} else {
			v1=line->v1; v2=line->v2;
		}

		sx = SGN(dx);

		x = x1;
		y = y1;

		incrementalize_y((float *)&line->v1, (float *)&line->v2, (float *)&v, (float *)&dv, y1, line->mask);

		d = ax-(ay>>1);
		for (;;) {
			(*dotproc)(x, y, &v);
			increment((float *)&v, (float *)&dv, line->mask);

			if (y==y2) break;
			if (d>=0) {
				x += sx;
				d -= ay;
			}
			y ++;
			d += ax;
		}
	}
}


