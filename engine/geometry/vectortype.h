/* vectortype.h: vector type definitions */

#ifndef _VECTORTYPE_H_
#define _VECTORTYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>

  /* #define ALL_DOUBLE */

/* a vector in 3D space */
typedef struct VECTOR {
#ifdef ALL_DOUBLE
  double x, y, z;
#else
  float x, y, z;
#endif

#ifdef __cplusplus
  // C++ only stuff for vectors
public:
  VECTOR(void) {}

  // Constructors
  VECTOR(float a, float b, float c) : x(a), y(b), z(c) {}

  // Defining this copy constructor, crashes Phy2 !?
  // VECTOR(const VECTOR& v) : x(v.x), y(v.y), z(v.z) {}

  // Comparison

  inline bool operator== (const VECTOR &v) const;
  inline bool operator!= (const VECTOR &v) const;

  // Operations

  // Precond: 0 <= i < 3
  inline float operator[] (int i);

  //inline const VECTOR &operator=  (const VECTOR &v);
  inline const VECTOR &operator+= (const VECTOR &v);
  inline const VECTOR &operator-= (const VECTOR &v);
  inline const VECTOR &operator*= (const VECTOR &v);
  inline const VECTOR &operator/= (const VECTOR &a);

  inline const VECTOR &operator*= (float s);
  inline const VECTOR &operator/= (float s);

  inline VECTOR operator+ (const VECTOR &v) const;
  inline VECTOR operator- (const VECTOR &v) const;
  inline VECTOR operator- () const;
  inline VECTOR operator* (const VECTOR &v) const;
  inline VECTOR operator* (float s) const;
  inline VECTOR operator/ (const VECTOR &v) const;
  inline VECTOR operator/ (float s) const;

  // Compute (T * vector) with 
  // T = transpose[ X Y Z ] so that e.g. T.X = (1 0 0) if
  // X,Y,Z form a coordinate system
  inline VECTOR transform(const VECTOR& X, const VECTOR& Y,
			  const VECTOR& Z) const;
  // Compute (transpose(T) * vector) with 
  // T = transpose[ X Y Z ]
  inline VECTOR itransform(const VECTOR& X, const VECTOR& Y,
			  const VECTOR& Z) const;
#endif

} VECTOR;


#ifdef __cplusplus
  // inline vector operations

  // Comparison

  inline bool VECTOR::operator == (const VECTOR &v) const
  { return x==v.x && y == v.y && z==v.z; }

  inline bool VECTOR::operator != (const VECTOR &v) const
  { return x!=v.x || y!=v.y || z!=v.z; }

  // Operations

  inline float VECTOR::operator[] (int i)
  { return ((float *)this)[i]; }

  //inline const VECTOR& VECTOR::operator=  (const VECTOR &v)
  //{ x=v.x; y=v.y; z=v.z; return *this; }

  inline const VECTOR& VECTOR::operator+= (const VECTOR &v)
  { x+=v.x; y+=v.y; z+=v.z; return *this; }

  inline const VECTOR& VECTOR::operator-= (const VECTOR &v)
  { x-=v.x; y-=v.y; z-=v.z; return *this; }

  inline const VECTOR& VECTOR::operator *= (const VECTOR &v)
  { x*=v.x; y*=v.y; z*=v.z; return *this; }

  inline const VECTOR& VECTOR::operator /= (const VECTOR &v)
  { x/=v.x; y/=v.y; z/=v.z; return *this; }

  inline const VECTOR& VECTOR::operator *= (float s)
  { x*=s; y*=s; z*=s; return *this; }

  inline const VECTOR& VECTOR::operator /= (float s)
  { x/=s; y/=s; z/=s; return *this; }


  inline VECTOR VECTOR::operator - () const
  { return VECTOR(-x,-y,-z); }

  inline VECTOR VECTOR::operator + (const VECTOR &v) const
  { return VECTOR(x+v.x, y+v.y, z+v.z); }

  inline VECTOR VECTOR::operator - (const VECTOR &v) const
  { return VECTOR(x-v.x, y-v.y, z-v.z); }

  inline VECTOR VECTOR::operator * (const VECTOR &v) const
  { return VECTOR(x*v.x, y*v.y, z*v.z); }

  inline VECTOR VECTOR::operator / (const VECTOR &v) const
  { return VECTOR(x/v.x, y/v.y, z/v.z); }

  inline VECTOR VECTOR::operator * (float s) const
  { return VECTOR(x*s, y*s, z*s); }

  inline VECTOR VECTOR::operator / (float s) const
  { return VECTOR(x/s, y/s, z/s); }

  // Separate inline vector functions

  // Scalar times vector
  inline VECTOR operator *(float s, const VECTOR &v)
  { return VECTOR(v.x*s, v.y*s, v.z*s); }

  // Dot product (Warning: '&' has lower precedence than + -)
  inline float operator&(const VECTOR &l, const VECTOR &r)
  { return l.x*r.x + l.y*r.y + l.z*r.z; }

  // Cross product (Warning: '^' has lower precedence than + -)
  inline VECTOR operator^(const VECTOR &l, const VECTOR &r)
  {
    return VECTOR(l.y * r.z - l.z * r.y,
		  l.z * r.x - l.x * r.z,
		  l.x * r.y - l.y * r.x);
  }

  // Compute (T * vector) with 
  // T = transpose[ X Y Z ] so that e.g. T.X = (1 0 0) if
  // X,Y,Z form a coordinate system
  inline VECTOR VECTOR::transform(const VECTOR& X, const VECTOR& Y,
				  const VECTOR& Z) const
  {
    return VECTOR(X & *this, Y & *this, Z & *this);
  }
  // Compute (transpose(T) * vector) with 
  // T = transpose[ X Y Z ]
  inline VECTOR VECTOR::itransform(const VECTOR& X, const VECTOR& Y,
				  const VECTOR& Z) const
  {
    return VECTOR(X.x * x + Y.x * y + Z.x * z,
		  X.y * x + Y.y * y + Z.y * z,
		  X.z * x + Y.z * y + Z.z * z);
  }

#endif /* __cplusplus */

/* a vector in 2D space */
typedef struct VEC2D {
#ifdef ALL_DOUBLE
	double 	u, v;
#else
	float 	u, v;
#endif

#ifdef __cplusplus
public:
  // C++ only stuff for vec2d
  VEC2D(void) {}
  VEC2D(float x, float y) {u = x; v = y;}

  // Comparison

  inline bool operator== (const VEC2D &v) const;
  inline bool operator!= (const VEC2D &v) const;

  // Operations

  //inline const VEC2D &operator=  (const VEC2D &v);
  inline const VEC2D &operator+= (const VEC2D &v);
  inline const VEC2D &operator-= (const VEC2D &v);
  inline const VEC2D &operator*= (const VEC2D &v);
  inline const VEC2D &operator/= (const VEC2D &a);

  inline const VEC2D &operator*= (float s);
  inline const VEC2D &operator/= (float s);

  inline VEC2D operator+ (const VEC2D &v) const;
  inline VEC2D operator- (const VEC2D &v) const;
  inline VEC2D operator- () const;
  inline VEC2D operator* (const VEC2D &v) const;
  inline VEC2D operator* (float s) const;
  inline VEC2D operator/ (const VEC2D &v) const;
  inline VEC2D operator/ (float s) const;

  inline float operator&(const VEC2D &r);
  inline float operator^(const VEC2D &r);
  
#endif
} VEC2D;


#ifdef __cplusplus
  // inline VEC2D operations

  // Comparison

  inline bool VEC2D::operator == (const VEC2D &V) const
  { return u==V.u && v == V.v ;}

  inline bool VEC2D::operator != (const VEC2D &V) const
  { return u!=V.u || v!=V.v ;}

  // Operations

  //inline const VEC2D& VEC2D::operator=  (const VEC2D &V)
  //{ u=V.u; v=V.v; z=V.z; return *this; }

  inline const VEC2D& VEC2D::operator+= (const VEC2D &V)
  { u+=V.u; v+=V.v; return *this; }

  inline const VEC2D& VEC2D::operator-= (const VEC2D &V)
  { u-=V.u; v-=V.v; return *this; }

  inline const VEC2D& VEC2D::operator *= (const VEC2D &V)
  { u*=V.u; v*=V.v; return *this; }

  inline const VEC2D& VEC2D::operator /= (const VEC2D &V)
  { u/=V.u; v/=V.v; return *this; }

  inline const VEC2D& VEC2D::operator *= (float s)
  { u*=s; v*=s; return *this; }

  inline const VEC2D& VEC2D::operator /= (float s)
  { u/=s; v/=s; return *this; }


  inline VEC2D VEC2D::operator - () const
  { return VEC2D(-u,-v); }

  inline VEC2D VEC2D::operator + (const VEC2D &V) const
  { return VEC2D(u+V.u, v+V.v); }

  inline VEC2D VEC2D::operator - (const VEC2D &V) const
  { return VEC2D(u-V.u, v-V.v); }

  inline VEC2D VEC2D::operator * (const VEC2D &V) const
  { return VEC2D(u*V.u, v*V.v); }

  inline VEC2D VEC2D::operator / (const VEC2D &V) const
  { return VEC2D(u/V.u, v/V.v); }

  inline VEC2D VEC2D::operator * (float s) const
  { return VEC2D(u*s, v*s); }

  inline VEC2D VEC2D::operator / (float s) const
  { return VEC2D(u/s, v/s); }

  // Separate inline vector functions
  // Not possible, because of extern "C" { ... } nesting
  // No overloading of operators allowed: clashes with VECTOR functions
  // Functions are made methods where possible
  // -- TODO : Fix this in *all* headers  :-(

  // Scalar times vector
  //inline VEC2D operator *(float s, const VEC2D &V)
  //{ return VEC2D(V.u*s, V.v*s); }

  // Dot product
  //inline float operator&(const VEC2D &l, const VEC2D &r)
  //{ return l.u*r.u + l.v*r.v; }
  inline float VEC2D::operator&(const VEC2D &r)
  { return u*r.u + v*r.v; }

  // Cross product (also determinant of (l.u l.v , r.u r.v)
  //inline float operator^(const VEC2D &l, const VEC2D &r)
  //{ return l.u * r.v - l.v * r.u; }
  inline float VEC2D::operator^(const VEC2D &r)
  { return u * r.v - v * r.u; }

#endif /* __cplusplus */




/* a vector in 4D space */
typedef struct VEC4D {
#ifdef ALL_DOUBLE
	double x, y, z, w;
#else
	float x, y, z, w;
#endif
} VEC4D;

/* double floating point vectors: The extra precision is sometimes needed eg
 * for sampling a spherical triangle or computing an analytical point to
 * patch factor, also for accurately computing normals and areas ... */
typedef struct DVECTOR {
	double x, y, z;
} DVECTOR;

typedef struct DVEC2D {
  double u, v;

#ifdef __cplusplus
public:
  // C++ only stuff for DVEC2d
  DVEC2D(void) {}
  DVEC2D(float x, float y) {u = x; v = y;}

  // Comparison

  inline bool operator== (const DVEC2D &v) const;
  inline bool operator!= (const DVEC2D &v) const;

  // Operations

  //inline const DVEC2D &operator=  (const DVEC2D &v);
  inline const DVEC2D &operator+= (const DVEC2D &v);
  inline const DVEC2D &operator-= (const DVEC2D &v);
  inline const DVEC2D &operator*= (const DVEC2D &v);
  inline const DVEC2D &operator/= (const DVEC2D &a);

  inline const DVEC2D &operator*= (float s);
  inline const DVEC2D &operator/= (float s);

  inline DVEC2D operator+ (const DVEC2D &v) const;
  inline DVEC2D operator- (const DVEC2D &v) const;
  inline DVEC2D operator- () const;
  inline DVEC2D operator* (const DVEC2D &v) const;
  inline DVEC2D operator* (float s) const;
  inline DVEC2D operator/ (const DVEC2D &v) const;
  inline DVEC2D operator/ (float s) const;

  inline float operator&(const DVEC2D &r);
  inline float operator^(const DVEC2D &r);
#endif /* __cplusplus */
} DVEC2D;

#ifdef __cplusplus
  // inline DVEC2D operations

  // Comparison

  inline bool DVEC2D::operator == (const DVEC2D &V) const
  { return u==V.u && v == V.v ;}

  inline bool DVEC2D::operator != (const DVEC2D &V) const
  { return u!=V.u || v!=V.v ;}

  // Operations

  //inline const DVEC2D& DVEC2D::operator=  (const DVEC2D &V)
  //{ u=V.u; v=V.v; z=V.z; return *this; }

  inline const DVEC2D& DVEC2D::operator+= (const DVEC2D &V)
  { u+=V.u; v+=V.v; return *this; }

  inline const DVEC2D& DVEC2D::operator-= (const DVEC2D &V)
  { u-=V.u; v-=V.v; return *this; }

  inline const DVEC2D& DVEC2D::operator *= (const DVEC2D &V)
  { u*=V.u; v*=V.v; return *this; }

  inline const DVEC2D& DVEC2D::operator /= (const DVEC2D &V)
  { u/=V.u; v/=V.v; return *this; }

  inline const DVEC2D& DVEC2D::operator *= (float s)
  { u*=s; v*=s; return *this; }

  inline const DVEC2D& DVEC2D::operator /= (float s)
  { u/=s; v/=s; return *this; }


  inline DVEC2D DVEC2D::operator - () const
  { return DVEC2D(-u,-v); }

  inline DVEC2D DVEC2D::operator + (const DVEC2D &V) const
  { return DVEC2D(u+V.u, v+V.v); }

  inline DVEC2D DVEC2D::operator - (const DVEC2D &V) const
  { return DVEC2D(u-V.u, v-V.v); }

  inline DVEC2D DVEC2D::operator * (const DVEC2D &V) const
  { return DVEC2D(u*V.u, v*V.v); }

  inline DVEC2D DVEC2D::operator / (const DVEC2D &V) const
  { return DVEC2D(u/V.u, v/V.v); }

  inline DVEC2D DVEC2D::operator * (float s) const
  { return DVEC2D(u*s, v*s); }

  inline DVEC2D DVEC2D::operator / (float s) const
  { return DVEC2D(u/s, v/s); }

  // Separate inline vector functions
  // Not possible, because of extern "C" { ... } nesting
  // No overloading of operators allowed: clashes with VECTOR functions
  // Functions are made methods where possible
  // -- TODO : Fix this in *all* headers  :-(

  // Scalar times vector
  //inline DVEC2D operator *(float s, const DVEC2D &V)
  //{ return DVEC2D(V.u*s, V.v*s); }

  // Dot product
  //inline float operator&(const DVEC2D &l, const DVEC2D &r)
  //{ return l.u*r.u + l.v*r.v; }
  inline float DVEC2D::operator&(const DVEC2D &r)
  { return u*r.u + v*r.v; }

  // Cross product (also determinant of (l.u l.v , r.u r.v)
  //inline float operator^(const DVEC2D &l, const DVEC2D &r)
  //{ return l.u * r.v - l.v * r.u; }
  inline float DVEC2D::operator^(const DVEC2D &r)
  { return u * r.v - v * r.u; }

#endif /* __cplusplus */



typedef struct DVEC4D {
	double x, y, z, w;
} DVEC4D;


/* Packed vectors for saving memory.
   Currently only for unit vectors */

typedef struct PACKEDUNITVECTOR {
  char x,y,z;

#ifdef __cplusplus
  // assignment operator
  inline PACKEDUNITVECTOR operator= (VECTOR v)
  {
    x = (char)(int)(v.x * 127);
    y = (char)(int)(v.y * 127);
    z = (char)(int)(v.z * 127);

    return *this;
  }

#endif

} PACKEDUNITVECTOR;



#define POINT VECTOR
#define POINT2D VEC2D
#define POINT4D VEC4D
#define DPOINT DVECTOR
#define DPOINT2D DVEC2D
#define DPOINT4D DVEC4D

/* memory management routines, to store vectors in linear lists or other generic
 * data structures of the generics library */

/* creates a vector with given components */
extern VECTOR *VectorCreate(float x, float y, float z);

/* destroys a vector */
extern void VectorDestroy(VECTOR *vector);

/* duplicates a vector */
extern VECTOR *VectorCopy(VECTOR *vector);

/* compute the distance between two 3D points */
extern double VectorDist(DVECTOR *p1, DVECTOR *p2);

/* The routines below were copied from the rayshade source files. */

/* normalizes the vector and returns the norm. Other difference with 
 * VECTORNORMALIZE is that it is explicitely tested that the components of the 
 * vector are within [-1,1] */
extern double VectorNormalize(VECTOR *vector);

/*
 * Find the "dominant" part of the vector (eg patch-normal).  This
 * is used to turn the point-in-polygon test into a 2D problem.
 */
extern int VectorDominantCoord(VECTOR *v);

#ifdef __cplusplus
}
#endif

#endif /*_VECTORTYPE_H_*/
