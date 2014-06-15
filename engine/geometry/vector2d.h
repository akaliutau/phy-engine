/* vector2d.h: all about 2d vectors */

#ifndef _VECTOR2D_H_
#define _VECTOR2D_H_

#include <math.h>
#include "extmath.h"
#include "vectortype.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Fills in u and v component of a vector */
#define VEC2DSET(o,a,b)	        {(o).u = a; (o).v = b;}

/* Copies the vector i to o. */
#define VEC2DCOPY(i,o)	        {(o).u = (i).u; (o).v = (i).v;}

/* Tolerance value for e.g. a vertex position */
#define VEC2DTOLERANCE(i)  	(EPSILON * (fabs((i).u) + fabs((i).v)))

/* Two vectors are equal if their components are equal within the given tolerance. */
#define VEC2DEQUAL(i, o, eps)	(FLOATEQUAL((i).u, (o).u, (eps)) && \
				 FLOATEQUAL((i).v, (o).v, (eps)))

/* Vector difference */
#define VEC2DSUBTRACT VEC2DDIFF
#define VEC2DDIFF(a, b, o)      {(o).u = (a).u - (b).u; \
			         (o).v = (a).v - (b).v;}

/* Scaled vector difference: d = a-s.b */
#define VEC2DSUBTRACTSCALED VEC2DDIFFSCALED
#define VEC2DDIFFSCALED(a, s, b, d) {(d).u = (a).u - (s) * (b).u; \
                                     (d).v = (a).v - (s) * (b).v;}

/* Vector sum: d = a+b */
#define VEC2DADD VEC2DSUM
#define VEC2DSUM(a, b, d)       {(d).u = (a).u + (b).u; \
				 (d).v = (a).v + (b).v;}

/* Scaled vector sum: d = a+s.b */
#define VEC2DADDSCALED VEC2DSUMSCALED
#define VEC2DSUMSCALED(a, s, b, d) 	 {(d).u = (a).u + (s) * (b).u; \
					  (d).v = (a).v + (s) * (b).v;}

/* Scalar vector product: a.b */
#define VEC2DDOTPRODUCT(a, b) 	((a).u * (b).u + (a).v * (b).v)

/* Square of vector norm: scalar product with itself */
#define VEC2DNORM2(d)		  ((d).u * (d).u + (d).v * (d).v)	 	

/* Norm of a vector: sqaure root of the sqaure norm */
#define VEC2DNORM(d)		  (double)sqrt((double)VEC2DNORM2(d))

/* Scale a vector: d = s.v (s is a real number) */
#define VEC2DSCALE(s, r, d)	  {(d).u = (s) * (r).u; \
				   (d).v = (s) * (r).v;}

/* Scales a vector with the inverse of the real number s if not zero: d = (1/s).v */
#define VEC2DSCALEINVERSE(s, r, d)	  {double _is_ = ((s) < -EPSILON || (s) > EPSILON ? 1./(s) : 1.); \
						   (d).u = _is_ * (r).u; \
						   (d).v = _is_ * (r).v;}

/* Normalizes a vector: scale it with the inverse of its norm */
#define VEC2DNORMALISE VEC2DNORMALIZE
#define VEC2DNORMALIZE(r)	{double _norm_; _norm_ = VEC2DNORM(r); \
				 VEC2DSCALEINVERSE(_norm_, r, r);}

/* inproduct of two vectors. */
#define VEC2DCROSSPRODUCT(a, b)	((a).u * (b).v - (a).v * (b).u)


/* Linear combination of two vectors: d = a.v + b.w */
#define VEC2DCOMB2(a, s, b, w, d)	{(d).u = (a) * (s).u + (b) * (w).u; \
				         (d).v = (a) * (s).v + (b) * (w).v;}

/* Affine linear combination of two vectors: d = o + a.v + b.w */
#define VEC2DCOMB3(o, a, s, b, w, d)	{(d).u = (o).u + (a) * (s).u + (b) * (w).u; \
				         (d).v = (o).v + (a) * (s).v + (b) * (w).v;}

/* Linear combination of three vectors */
#define VEC2DCOORD(a, X, b, Y, c, Z, d)  {(d).u = (a) * (X).u + (b) * (Y).u + (c) * (Z).u; \
					  (d).v = (a) * (X).v + (b) * (Y).v + (c) * (Z).v;}

/* Centre of two points */
#define VEC2DMIDPOINT(p1, p2, m)	{ 	                                \
	                         (m).u = 0.5 * ((p1).u + (p2).u);	\
	                         (m).v = 0.5 * ((p1).v + (p2).v);       \
                                }

/* Centre of four points */
#define VEC2DMIDPOINT4(p1, p2, p3, p4, m)	{ 	        \
	(m).u = 0.25 * ((p1).u + (p2).u + (p3).u + (p4).u); 	\
	(m).v = 0.25 * ((p1).v + (p2).v + (p3).v + (p4).v);     \
				}

/* Sum of four vectors */
#define VEC2DSUM4(v1, v2, v3, v4, s)	{ 		\
	(s).u = (v1).u + (v2).u + (v3).u + (v4).u;	\
	(s).v = (v1).v + (v2).v + (v3).v + (v4).v;}

/* Perpendicular vector */
#define VEC2DPERP(i,o) { (o).u = -(i).v; (o).v = (i).u; }

/* Opposite of perpendicular vector */
#define VEC2DOPPPERP(i,o) { (o).u = (i).v; (o).v= -(i).u; }


/* Opposite of a vector*/
#define VEC2DOPPOSITE(i,o) { (o).u = -(i).u; (o).v = -(i).v; }

#define VEC2DPRINT(i) {fprintf(stderr,"Vector 2D: (%f,%f)\n",(i).u,(i).v);}

/* linedir has length 1 */
void VEC2DPointOnLineNearestPoint(VEC2D *pt, VEC2D *lineorg,VEC2D *linedir,VEC2D *res);

/* reflect a point around a line */
void VEC2DReflectPointLine(VEC2D *pt,VEC2D *lineorg,VEC2D *linedir,VEC2D *res);

/* a --> b anti-horlogique 
   a & b are normalised
*/ 
double VEC2DAngle(VEC2D* a,VEC2D* b);

int    intersectLineWithCircle(VEC2D *origin,VEC2D *dir,VEC2D *center,double r2);

extern VEC2D origin2d;

#ifdef __cplusplus
}
#endif

#endif /* _VECTOR_2D_H_ */
