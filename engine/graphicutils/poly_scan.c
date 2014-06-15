



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "poly.h"



static void incrementalize_y(double *p1, double *p2, double *p, double *dp, int y, long unsigned int mask)
{
    double dy, frac;

    dy = ((Poly_vert *)p2)->sy - ((Poly_vert *)p1)->sy;
    if (dy==0.) dy = 1.;
    frac = y+.5 - ((Poly_vert *)p1)->sy;

    for (; mask!=0; mask>>=1, p1++, p2++, p++, dp++)
	if (mask&1) {
	    *dp = (*p2-*p1)/dy;
	    *p = *p1+*dp*frac;
	}
}



static void incrementalize_x(double *p1, double *p2, double *p, double *dp, int x, long unsigned int mask)
{
    double dx, frac;

    dx = ((Poly_vert *)p2)->sx - ((Poly_vert *)p1)->sx;
    if (dx==0.) dx = 1.;
    frac = x+.5 - ((Poly_vert *)p1)->sx;

    for (; mask!=0; mask>>=1, p1++, p2++, p++, dp++)
	if (mask&1) {
	    *dp = (*p2-*p1)/dx;
	    *p = *p1+*dp*frac;
	}
}

static void increment(double *p, double *dp, long unsigned int mask)
{
    for (; mask!=0; mask>>=1, p++, dp++)
	if (mask&1)
	    *p += *dp;
}



static void scanline(int y, Poly_vert *l, Poly_vert *r, Window *win, void (*pixelproc) (int x, int y, Poly_vert *point), long unsigned int mask)
{
    int x, lx, rx;
    Poly_vert p, dp;

    mask &= ~POLY_MASK(sx);		
    lx = ceil(l->sx-.5);
    if (lx<win->x0) lx = win->x0;
    rx = floor(r->sx-.5);
    if (rx>win->x1) rx = win->x1;
    if (lx>rx) return;
    incrementalize_x((double *)l, (double *)r, (double *)&p, (double *)&dp, lx, mask);
    for (x=lx; x<=rx; x++) {		
	(*pixelproc)(x, y, &p);
	increment((double *)&p, (double *)&dp, mask);
    }
}



void poly_scan(Poly *p, Window *win, void (*pixelproc) (int x, int y, Poly_vert *point))
                 		
            			
                    		
{
    int i, li, ri, y, ly, ry, top, rem;
    unsigned long mask;
    double ymin;
    Poly_vert l, r, dl, dr;

    if (p->n>POLY_NMAX) {
	fprintf(stderr, "poly_scan: too many vertices: %d\n", p->n);
	return;
    }
    if (sizeof(Poly_vert)/sizeof(double) > 32) {
	fprintf(stderr, "Poly_vert structure too big; must be <=32 doubles\n");
	exit(1);
    }

    ymin = HUGE; top = -1;
    for (i=0; i<p->n; i++)		
	if (p->vert[i].sy < ymin) {
	    ymin = p->vert[i].sy;
	    top = i;
	}

    li = ri = top;			
    rem = p->n;				
    y = ceil(ymin-.5);			
    ly = ry = y-1;			
    mask = p->mask & ~POLY_MASK(sy);	

    while (rem>0) {	
			

	while (ly<=y && rem>0) {	
	    rem--;
	    i = li-1;			
	    if (i<0) i = p->n-1;
	    incrementalize_y((double *)&p->vert[li], (double *)&p->vert[i], (double *)&l, (double *)&dl, y, mask);
	    ly = floor(p->vert[i].sy+.5);
	    li = i;
	}
	while (ry<=y && rem>0) {	
	    rem--;
	    i = ri+1;			
	    if (i>=p->n) i = 0;
	    incrementalize_y((double *)&p->vert[ri], (double *)&p->vert[i], (double *)&r, (double *)&dr, y, mask);
	    ry = floor(p->vert[i].sy+.5);
	    ri = i;
	}

	while (y<ly && y<ry) {	    
	  if (y>=win->y0 && y<=win->y1) {
	    if (l.sx<=r.sx) scanline(y, &l, &r, win, pixelproc, mask);
	    else		scanline(y, &r, &l, win, pixelproc, mask);
	  }
	  y++;
	  increment((double *)&l, (double *)&dl, mask);
	  increment((double *)&r, (double *)&dr, mask);
	}
    }
}

