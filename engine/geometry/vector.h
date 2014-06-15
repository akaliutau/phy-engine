/* vector.h: all about vectors */

#ifndef _PHY_VECTOR_H_
#define _PHY_VECTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include "../math/Float.h"
#include "../math/extmath.h"
#include "vectortype.h"

/* fills in x, y, and z component of a vector */
#define VECTORSET(v, a, b, c)	{(v).x = a; (v).y = b; (v).z = c;}

/* copies the vector v to d: d = v. They may be different vector types. */
#define VECTORCOPY(v, d)	{(d).x = (v).x; (d).y = (v).y; (d).z = (v).z;}

/* tolerance value for e.g. a vertex position */
#define VECTORTOLERANCE(v)  	(EPSILON * (fabs((v).x) + fabs((v).y) + fabs((v).z)))

/* two vectors are equal if their components are equal within the given tolerance. */
#define VECTOREQUAL(v, w, eps)	(FLOATEQUAL((v).x, (w).x, (eps)) && \
				 FLOATEQUAL((v).y, (w).y, (eps)) && \
				 FLOATEQUAL((v).z, (w).z, (eps)))

/* vector difference */
#define VECTORSUBTRACT VECTORDIFF
#define VECTORDIFF(a, b, d) {(d).x = (a).x - (b).x; \
			     (d).y = (a).y - (b).y; \
			     (d).z = (a).z - (b).z;}

/* scaled vector difference: d = a-s.b */
#define VECTORSUBTRACTSCALED VECTORDIFFSCALED
#define VECTORDIFFSCALED(a, s, b, d){(d).x = (a).x - (s) * (b).x; \
                                     (d).y = (a).y - (s) * (b).y; \
				     (d).z = (a).z - (s) * (b).z;}

/* vector sum: d = a+b */
#define VECTORADD VECTORSUM
#define VECTORSUM(a, b, d) 	 {(d).x = (a).x + (b).x; \
				  (d).y = (a).y + (b).y; \
				  (d).z = (a).z + (b).z;}

/* scaled vector sum: d = a+s.b */
#define VECTORADDSCALED VECTORSUMSCALED
#define VECTORSUMSCALED(a, s, b, d) 	 {(d).x = (a).x + (s) * (b).x; \
					  (d).y = (a).y + (s) * (b).y; \
				          (d).z = (a).z + (s) * (b).z;}

/* scalar vector product: a.b */
#define VECTORDOTPRODUCT(a, b) 	((a).x * (b).x + (a).y * (b).y + (a).z * (b).z)

/* square of vector norm: scalar product with itself */
#define VECTORNORM2(v)		  ((v).x * (v).x + (v).y * (v).y + (v).z * (v).z)	 	

/* norm of a vector: sqaure root of the sqaure norm */
#define VECTORNORM(v)		  (double)sqrt((double)VECTORNORM2(v))

/* scale a vector: d = s.v (s is a real number) */
#define VECTORSCALE(s, v, d)	  {(d).x = (s) * (v).x; \
				   (d).y = (s) * (v).y; \
				   (d).z = (s) * (v).z;}

/* scales a vector with the inverse of the real number s if not zero: d = (1/s).v */
#define VECTORSCALEINVERSE(s, v, d)	  {double _is_ = ((s) < -EPSILON || (s) > EPSILON ? 1./(s) : 1.); \
						   (d).x = _is_ * (v).x; \
						   (d).y = _is_ * (v).y; \
						   (d).z = _is_ * (v).z;}

/* normalizes a vector: scale it with the inverse of its norm */
#define VECTORNORMALISE VECTORNORMALIZE
#define VECTORNORMALIZE(v)	{double _norm_; _norm_ = VECTORNORM(v); \
				 VECTORSCALEINVERSE(_norm_, v, v);}

/* inproduct of two vectors. */
#define VECTORCROSSPRODUCT(a, b, d)	{DVECTOR _d; \
					 (_d).x = (a).y * (b).z - (a).z * (b).y; \
					 (_d).y = (a).z * (b).x - (a).x * (b).z; \
					 (_d).z = (a).x * (b).y - (a).y * (b).x; \
				         VECTORCOPY(_d, (d));}

/* normal of the triangle defined by three non-colinear vectors:
 * n = (v2-v1) x (v3-v2) normalized */
#define NORMAL(v1, v2, v3, n)		{DVECTOR _d1_, _d2_; \
					 VECTORSUBTRACT(v2, v1, _d1_); \
					 VECTORSUBTRACT(v3, v2, _d2_); \
					 VECTORCROSSPROD(_d1_, _d2_, n); \
					 VECTORNORMALIZE(n); }

/* linear combination of two vectors: d = a.v + b.w */
#define VECTORCOMB2(a, v, b, w, d)	{(d).x = (a) * (v).x + (b) * (w).x; \
				 (d).y = (a) * (v).y + (b) * (w).y; \
				 (d).z = (a) * (v).z + (b) * (w).z;}

/* affine linear combination of two vectors: d = o + a.v + b.w */
#define VECTORCOMB3(o, a, v, b, w, d)	{(d).x = (o).x + (a) * (v).x + (b) * (w).x; \
				 (d).y = (o).y + (a) * (v).y + (b) * (w).y; \
				 (d).z = (o).z + (a) * (v).z + (b) * (w).z;}

/* linear combination of three vectors */
#define VECTORCOORD(a, X, b, Y, c, Z, d) {(d).x = (a) * (X).x + (b) * (Y).x + (c) * (Z).x; \
					  (d).y = (a) * (X).y + (b) * (Y).y + (c) * (Z).y; \
					  (d).z = (a) * (X).z + (b) * (Y).z + (c) * (Z).z;}

/* triple (cross) product: d = (v3-v2) x (v1-v2) */
#define VECTORTRIPLECROSSPRODUCT(v1, v2, v3, d) {DVECTOR _D1, _D2;	\
	VECTORSUBTRACT(v3, v2, _D1);					\
	VECTORSUBTRACT(v1, v2, _D2);					\
	VECTORCROSSPRODUCT(_D1, _D2, d)					\
				      }

/* triple (dot) product: s = (v3-v2) . (v1-v2) */
#define VECTORTRIPLEDOTPRODUCT(v1, v2, v3, s) {DVECTOR _D1, _D2;	\
	VECTORSUBTRACT(v3, v2, _D1);					\
	VECTORSUBTRACT(v1, v2, _D2);					\
	s = VECTORDOTPRODUCT(_D1, _D2)					\
				      }

/* determinant s = (v1 x v2) . v3 = v1 . (v2 x v3) */
#define VECTORDETERMINANT(v1, v2, v3, s) {DVECTOR _D;	\
        VECTORCROSSPRODUCT(v1, v2, _D);			\
        s = VECTORDOTPRODUCT(_D, v3);			\
				       }

/* distance between two points in 3D space: s = |p2-p1| */
#define VECTORDIST(p1, p2, s) {	DVECTOR _D; 	\
        VECTORSUBTRACT(p2, p1, _D);		\
	s = VECTORNORM(_D);			\
}

/* squared distance between two points in 3D space: s = |p2-p1| */
#define VECTORDIST2(p1, p2, s) {	DVECTOR _D; 	\
        VECTORSUBTRACT(p2, p1, _D);		\
	s = VECTORNORM2(_D);			\
			      }

/* orthogonal component of X with respect to Y. Result is stored in d */
#define VECTORORTHOCOMP(X, Y, d)	{double _dotp_;	\
	_dotp_ = VECTORDOTPRODUCT(X, Y);		\
        VECTORSUMSCALED(X, -_dotp_, Y, d);		\
				 }

#define XNORMAL 0
#define YNORMAL 1
#define ZNORMAL 2
#define VECTORINDEXNAME(index)	((index) == XNORMAL ? "XNORMAL" :	\
				 ((index) == YNORMAL ? "YNORMAL" : "ZNORMAL"))

/* given a vector p in 3D space and an index i, which is 
 * XNORMAL, YNORMAL or ZNORMAL, projects the vector on the YZ, XZ or XY plane 
 * respectively. */
#define VECTORPROJECT(r, p, i)	{switch(i) { \
				case XNORMAL: \
					r.u = (p).y; \
					r.v = (p).z; \
					break; \
				case YNORMAL: \
					r.u = (p).x; \
					r.v = (p).z; \
					break; \
				case ZNORMAL: \
					r.u = (p).x; \
					r.v = (p).y; \
					break; \
  				} }

/* centre of two points */
#define MIDPOINT(p1, p2, m)	{ 	\
	(m).x = 0.5 * ((p1).x + (p2).x);	\
	(m).y = 0.5 * ((p1).y + (p2).y);	\
	(m).z = 0.5 * ((p1).z + (p2).z);	\
				}

/* centre of four points */
#define MIDPOINT4(p1, p2, p3, p4, m)	{ 	\
	(m).x = 0.25 * ((p1).x + (p2).x + (p3).x + (p4).x);	\
	(m).y = 0.25 * ((p1).y + (p2).y + (p3).y + (p4).y);	\
	(m).z = 0.25 * ((p1).z + (p2).z + (p3).z + (p4).z);	\
				}

/* sum of four vectors */
#define VECTORSUM4(v1, v2, v3, v4, s)	{ 		\
	(s).x = (v1).x + (v2).x + (v3).x + (v4).x;	\
	(s).y = (v1).y + (v2).y + (v3).y + (v4).y;	\
	(s).z = (v1).z + (v2).z + (v3).z + (v4).z;	\
				}


/* Point IN Triangle: barycentric parametrisation */
#define PINT(v0, v1, v2, u, v, p)     {						\
        double _u = (u), _v = (v); 						\
	(p).x = (v0).x + _u * ((v1).x - (v0).x) + _v * ((v2).x - (v0).x);	\
	(p).y = (v0).y + _u * ((v1).y - (v0).y) + _v * ((v2).y - (v0).y);	\
	(p).z = (v0).z + _u * ((v1).z - (v0).z) + _v * ((v2).z - (v0).z);	\
				      }

/* Point IN Quadrilateral: bilinear parametrisation */
#define PINQ(v0, v1 ,v2 ,v3, u, v, p)	{	\
        double _c=(u)*(v), _b=(u)-_c, _d=(v)-_c;	\
	(p).x = (v0).x + (_b) * ((v1).x - (v0).x) + (_c) * ((v2).x - (v0).x)+ (_d) * ((v3).x - (v0).x);	\
	(p).y = (v0).y + (_b) * ((v1).y - (v0).y) + (_c) * ((v2).y - (v0).y)+ (_d) * ((v3).y - (v0).y);	\
	(p).z = (v0).z + (_b) * ((v1).z - (v0).z) + (_c) * ((v2).z - (v0).z)+ (_d) * ((v3).z - (v0).z);	\
					};

/* computes d = (1-s).p + s.q = p + s.(q-p) */
#define VECTORINTERPOLATE(p,q,s,d)  {double _s = (s);	\
	(d).x = (p).x + _s * ((q).x - (p).x); \
	(d).y = (p).y + _s * ((q).y - (p).y); \
	(d).z = (p).z + _s * ((q).z - (p).z); \
}

/* maximum of two vectors: d.x = max(v1.x,v2.x) etc ...*/
#define VECTORMAX(v1,v2,d) {		\
        (d).x = MAX((v1).x, (v2).x);	\
        (d).y = MAX((v1).y, (v2).y);	\
        (d).z = MAX((v1).z, (v2).z);	\
}

/* minimum of two vectors: d.x = min(v1.x,v2.x) etc ...*/
#define VECTORMIN(v1,v2,d) {		\
        (d).x = MIN((v1).x, (v2).x);	\
        (d).y = MIN((v1).y, (v2).y);	\
        (d).z = MIN((v1).z, (v2).z);	\
}

/* transforms a vector to the first quadrant by replacing its components
 * by their absolute value */
#define VECTORABS(v,d) {	\
        (d).x = fabs((v).x); 	\
        (d).y = fabs((v).y); 	\
        (d).z = fabs((v).z); 	\
}

#include <stdio.h>
#define VectorPrint(fp, v) fprintf(fp, "%g %g %g", (v).x, (v).y, (v).z)

/* normalizes a vector, returns the length of the supplied vector */
extern double VectorNormalize(VECTOR *vector);

/* Fills in a Coordinate frame given the Z vector and a projection
 * axis i0 (XNORMAL, YNORMAL or ZNORMAL). If Z.i0 < EPSILON
 * FALSE is returned. Make another call with another axis. */
extern int VectorFrame(VECTOR *Z, int axis, VECTOR *X, VECTOR *Y);

/* Find the "dominant" part of the vector (eg patch-normal).  This
 * is used to turn the point-in-polygon test into a 2D problem. */
extern int VectorDominantCoord(VECTOR *v);
extern int DVectorDominantCoord(DVECTOR *v);

/* This routine compares two vectors: if the vectors are equal, XYZ_EQUAL is returned. If
 * they are not equal, a code 0-7 is returned which can be used for sorting vectors in
 * an octree. This code is a combination of X_GREATER, Y_GREATER and Z_GREATER. */
#define X_GREATER	0x01
#define Y_GREATER	0x02
#define Z_GREATER	0x04
#define XYZ_EQUAL	0x08
extern int VectorCompare(VECTOR *v1, VECTOR *v2, float epsilon);


/********* PACKED UNIT VECTORS *********/

#define PACKUNITVECTOR(pv, v) {pv.x = (char)(int)(v.x *127); \
  pv.y = (char)(int)(v.y *127);pv.z = (char)(int)(v.z *127);}
#define UNPACKUNITVECTOR(v, pv) {v.x = (float)pv.x * 127.0; \
  v.y = (float)pv.y * 127.0;v.z = (float)pv.z * 127.0;}

#ifdef __cplusplus
}
#endif

#endif /* _PHY_VECTOR_H_ */
