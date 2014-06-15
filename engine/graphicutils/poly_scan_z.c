





#include <stdio.h>
#include <math.h>
#include "poly.h"
#include "draw.h"



static void incrementalize_y(double *p1, double *p2, double *p, double *dp, int y)
{
    double dy, frac;

    dy = ((Poly_vert *)p2)->sy - ((Poly_vert *)p1)->sy;
    if (dy==0.) dy = 1.;
    frac = y+.5 - ((Poly_vert *)p1)->sy;

    
    dp[0] = (p2[0]-p1[0])/dy;
    p[0] = p1[0]+dp[0]*frac;
    dp[2] = (p2[2]-p1[2])/dy;
    p[2] = p1[2]+dp[2]*frac;
}

static void increment(double *p, double *dp)
{
  
  p[0] += dp[0];
  p[2] += dp[2];
}



static void scanline(int y, Poly_vert *l, Poly_vert *r, Window *win)
{
    int x, lx, rx, offset;
    int dz;
    DRAW_PIXEL *pix;
    DRAW_ZVAL *zval, z;
    double dx, frac, dzf;

    lx = ceil(l->sx-.5);
    if (lx<win->x0) lx = win->x0;
    rx = floor(r->sx-.5);
    if (rx>win->x1) rx = win->x1;
    if (lx>rx) return;

    dx = r->sx - l->sx;
    if (dx==0.) dx = 1.;
    frac = lx+.5 - l->sx;
    dzf = (r->sz - l->sz)/dx;
    z = (DRAW_ZVAL)(l->sz + dzf * frac);
    dz = (int)dzf;

    offset = y * current_draw_context->width + lx;
    pix = current_draw_context->fbuf + offset;
    zval = current_draw_context->zbuf + offset;
    for (x=lx; x<=rx; x++) {		
      if (z <= *zval) {
	*pix = current_draw_context->curpixel;
	*zval = z;
      }
      pix++;
      zval++;
      z += dz;
    }
}



void poly_scan_z(Poly *p, Window *win)
                 		
            			
{
    int i, li, ri, y, ly, ry, top, rem;
    double ymin;
    Poly_vert l, r, dl, dr;

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

    while (rem>0) {	
			

	while (ly<=y && rem>0) {	
	    rem--;
	    i = li-1;			
	    if (i<0) i = p->n-1;
	    incrementalize_y((double *)&p->vert[li], (double *)&p->vert[i], (double *)&l, (double *)&dl, y);
	    ly = floor(p->vert[i].sy+.5);
	    li = i;
	}
	while (ry<=y && rem>0) {	
	    rem--;
	    i = ri+1;			
	    if (i>=p->n) i = 0;
	    incrementalize_y((double *)&p->vert[ri], (double *)&p->vert[i], (double *)&r, (double *)&dr, y);
	    ry = floor(p->vert[i].sy+.5);
	    ri = i;
	}

	while (y<ly && y<ry) {	    
	  if (y>=win->y0 && y<=win->y1) {
	    if (l.sx<=r.sx) scanline(y, &l, &r, win);
	    else		scanline(y, &r, &l, win);
	  }
	  y++;
	  increment((double *)&l, (double *)&dl);
	  increment((double *)&r, (double *)&dr);
	}
    }
}

