



#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "poly.h"

#define SWAP(a, b, temp)	{temp = a; a = b; b = temp;}
#define COORD(vert, i) ((double *)(vert))[i]

#define CLIP_AND_SWAP(elem, sign, k, p, q, r) { \
    poly_clip_to_halfspace(p, q, &v->elem-(double *)v, sign, sign*k); \
    if (q->n==0) {p1->n = 0; return POLY_CLIP_OUT;} \
    SWAP(p, q, r); \
}



int poly_clip_to_box(Poly *p1, Poly_box *box)
{
    int x0out = 0, x1out = 0, y0out = 0, y1out = 0, z0out = 0, z1out = 0;
    int i;
    Poly_vert *v;
    Poly p2, *p, *q, *r;

    if (p1->n+6>POLY_NMAX) {
	fprintf(stderr, "poly_clip_to_box: too many vertices: %d (max=%d-6)\n",
	    p1->n, POLY_NMAX);
	exit(1);
    }
    if (sizeof(Poly_vert)/sizeof(double) > 32) {
	fprintf(stderr, "Poly_vert structure too big; must be <=32 doubles\n");
	exit(1);
    }

    
    for (v=p1->vert, i=p1->n; i>0; i--, v++) {
	if (v->sx < box->x0*v->sw) x0out++;	
	if (v->sx > box->x1*v->sw) x1out++;	
	if (v->sy < box->y0*v->sw) y0out++;	
	if (v->sy > box->y1*v->sw) y1out++;	
	if (v->sz < box->z0*v->sw) z0out++;	
	if (v->sz > box->z1*v->sw) z1out++;	
    }

    
    if (x0out+x1out+y0out+y1out+z0out+z1out == 0) return POLY_CLIP_IN;

    
    if (x0out==p1->n || x1out==p1->n || y0out==p1->n ||
	y1out==p1->n || z0out==p1->n || z1out==p1->n) {
	    p1->n = 0;
	    return POLY_CLIP_OUT;
	}

    
    p = p1;
    q = &p2;
    if (x0out) CLIP_AND_SWAP(sx, -1., box->x0, p, q, r);
    if (x1out) CLIP_AND_SWAP(sx,  1., box->x1, p, q, r);
    if (y0out) CLIP_AND_SWAP(sy, -1., box->y0, p, q, r);
    if (y1out) CLIP_AND_SWAP(sy,  1., box->y1, p, q, r);
    if (z0out) CLIP_AND_SWAP(sz, -1., box->z0, p, q, r);
    if (z1out) CLIP_AND_SWAP(sz,  1., box->z1, p, q, r);

    
    if (p==&p2)
	bcopy(&p2, p1, sizeof(Poly)-(POLY_NMAX-p2.n)*sizeof(Poly_vert));
    return POLY_CLIP_PARTIAL;
}



void poly_clip_to_halfspace(Poly *p, Poly *q, int index, double sign, double k)
{
    unsigned long m;
    double *up, *vp, *wp;
    Poly_vert *v;
    int i;
    Poly_vert *u;
    double t, tu, tv;

    q->n = 0;
    q->mask = p->mask;

    
    u = &p->vert[p->n-1];
    tu = sign*COORD(u, index) - u->sw*k;
    for (v= &p->vert[0], i=p->n; i>0; i--, u=v, tu=tv, v++) {
	
	
	tv = sign*COORD(v, index) - v->sw*k;
	if (((tu<=0.) ^ (tv<=0.))) {
	    
	    t = tu/(tu-tv);
	    up = (double *)u;
	    vp = (double *)v;
	    wp = (double *)&q->vert[q->n];
	    for (m=p->mask; m!=0; m>>=1, up++, vp++, wp++)
		if (m&1) *wp = *up+t*(*vp-*up);
	    q->n++;
	}
	if (tv<=0.)		
	    q->vert[q->n++] = *v;
    }
}
