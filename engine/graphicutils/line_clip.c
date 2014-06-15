

#include <stdio.h>
#include "line.h"



int line_clip_to_box(Line *lin, Poly_box *box)
{
	int x0out = 0, x1out = 0, y0out = 0, y1out = 0, z0out = 0, 
	    z1out = 0;
	Poly_vert *v1 = &lin->v1, *v2 = &lin->v2;
	float t1, t2, u1, u2, t, *p1, *p2, dp;
	int mask;


	if (v1->sx < box->x0*v1->sw) x0out++;	
	if (v1->sx > box->x1*v1->sw) x1out++;	
	if (v1->sy < box->y0*v1->sw) y0out++;	
	if (v1->sy > box->y1*v1->sw) y1out++;	
	if (v1->sz < box->z0*v1->sw) z0out++;	
	if (v1->sz > box->z1*v1->sw) z1out++;	


	if (v2->sx < box->x0*v2->sw) x0out++;	
	if (v2->sx > box->x1*v2->sw) x1out++;	
	if (v2->sy < box->y0*v2->sw) y0out++;	
	if (v2->sy > box->y1*v2->sw) y1out++;	
	if (v2->sz < box->z0*v2->sw) z0out++;	
	if (v2->sz > box->z1*v2->sw) z1out++;	


	if (x0out+x1out+y0out+y1out+z0out+z1out == 0) return LINE_CLIP_IN;


	if (x0out==2 || x1out==2 || y0out==2 ||
	    y1out==2 || z0out==2 || z1out==2) 
	    return LINE_CLIP_OUT;


	t1 = 0.; t2 = 1.;	

	if (x0out) {	

		u1 = v1->sx - box->x0 * v1->sw;
		u2 = v2->sx - box->x0 * v2->sw;
		t = u1 / (u1-u2);	

		if (u1 > 0) {	
			t2 = t;
		} else {	
			t1 = t;
		}
	}

	if (x1out) {
		u1 = v1->sx - box->x1 * v1->sw;
		u2 = v2->sx - box->x1 * v2->sw;
		t = u1 / (u1-u2);	
		if (u1 <= 0) {	
			if (t < t2) t2 = t;
		} else {	
			if (t > t1) t1 = t;
		}
	}

	if (y0out) {
		u1 = v1->sy - box->y0 * v1->sw;
		u2 = v2->sy - box->y0 * v2->sw;
		t = u1 / (u1-u2);	
		if (u1 > 0) {	
			if (t < t2) t2 = t;
		} else {	
			if (t > t1) t1 = t;
		}
	}

	if (y1out) {
		u1 = v1->sy - box->y1 * v1->sw;
		u2 = v2->sy - box->y1 * v2->sw;
		t = u1 / (u1-u2);	
		if (u1 <= 0) {	
			if (t < t2) t2 = t;
		} else {	
			if (t > t1) t1 = t;
		}
	}

	if (z0out) {
		u1 = v1->sz - box->z0 * v1->sw;
		u2 = v2->sz - box->z0 * v2->sw;
		t = u1 / (u1-u2);	
		if (u1 > 0) {	
			if (t < t2) t2 = t;
		} else {	
			if (t > t1) t1 = t;
		}
	}

	if (z1out) {
		u1 = v1->sz - box->z1 * v1->sw;
		u2 = v2->sz - box->z1 * v2->sw;
		t = u1 / (u1-u2);	
		if (u1 <= 0) {	
			if (t < t2) t2 = t;
		} else {	
			if (t > t1) t1 = t;
		}
	}


	if (t2 <= t1) 
		return LINE_CLIP_OUT;


	t2 = 1. - t2;	
	p1 = (float *)v1; 
	p2 = (float *)v2;

	if (t1 > 0. && t2 > 0.) {
		for (mask = lin->mask; mask; mask >>= 1, p1++, p2++) 
			if (mask&1) {
				dp = *p2 - *p1;
				*p1 += t1 * dp;
				*p2 -= t2 * dp;
			}
	} else if (t1 > 0.) {
		for (mask = lin->mask; mask; mask >>= 1, p1++, p2++) 
			if (mask&1) 
				*p1 += t1 * (*p2 - *p1);
	} else if (t2 > 0.) {
		for (mask = lin->mask; mask; mask >>= 1, p1++, p2++) 
			if (mask&1) 
				*p2 += t2 * (*p1 - *p2);
	}

	return LINE_CLIP_PARTIAL;
}


