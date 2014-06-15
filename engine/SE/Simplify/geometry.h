/****************************************************************************\

  Copyright 1995 The University of North Carolina at Chapel Hill.
  All Rights Reserved.

  Permission to use, copy, modify and distribute this software and its
  documentation for educational, research and non-profit purposes,
  without fee, and without a written agreement is hereby granted,
  provided that the above copyright notice and the following three
  paragraphs appear in all copies.

  IN NO EVENT SHALL THE UNIVERSITY OF NORTH CAROLINA AT CHAPEL HILL BE
  LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
  CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE
  USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY
  OF NORTH CAROLINA HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
  DAMAGES.


  Permission to use, copy, modify and distribute this software and its
  documentation for educational, research and non-profit purposes,
  without fee, and without a written agreement is hereby granted,
  provided that the above copyright notice and the following three
  paragraphs appear in all copies.

  THE UNIVERSITY OF NORTH CAROLINA SPECIFICALLY DISCLAIM ANY
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
  PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
  NORTH CAROLINA HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
  UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

  The authors may be contacted via:

  US Mail:  Jonathan Cohen                      Amitabh Varshney
            Department of Computer Science      Department of Computer Science 
            Sitterson Hall, CB #3175            State University of New York
            University of N. Carolina           Stony Brook, NY 11794-4400, USA 
            Chapel Hill, NC 27599-3175
	    
  Phone:    (919)962-1749                       Phone: (516)632-8446 
	    
  EMail:    cohenj@cs.unc.edu                   varshney@cs.sunysb.edu

\****************************************************************************/

/* Protection from multiple includes. */
#ifndef INCLUDED_OFFSET_GEOMETRY_H
#define INCLUDED_OFFSET_GEOMETRY_H


/*------------------ Includes Needed for Definitions Below ------------------*/

#include <math.h>

/*-------------------------------- Constants --------------------------------*/

#if 0
#define USE_TRIOCTREE
#endif

/* sorry for this hack, but this #define must appear or not appear
   consistantly in both simplify.h and geometry.h, so modify them both
   accordingly */
#if 0
#define VERTEX_EPSILONS
#endif


#define LO 0
#define HI 1

#define X 0
#define Y 1
#define Z 2

#define FALSE           0
#define TRUE            1

/*--------------------------------- Macros ----------------------------------*/

#define FMAX(x,y) ((x)>(y) ? (x) : (y))
#define FMIN(x,y) ((x)<(y) ? (x) : (y))

#define BBOX_OVERLAP(box1, box2)						\
    (((((box1)[HI][X] < (box2)[LO][X]) || ((box2)[HI][X] < (box1)[LO][X])) ||	\
     (((box1)[HI][Y] < (box2)[LO][Y]) || ((box2)[HI][Y] < (box1)[LO][Y])) ||	\
     (((box1)[HI][Z] < (box2)[LO][Z]) || ((box2)[HI][Z] < (box1)[LO][Z]))) ?	\
       FALSE : TRUE)
        

#define COMMON_VERTEX(tri1, tri2)				\
    ((((tri1)->verts[0] == (tri2)->verts[0]) ||			\
      ((tri1)->verts[0] == (tri2)->verts[1]) ||			\
      ((tri1)->verts[0] == (tri2)->verts[2]) ||			\
      ((tri1)->verts[1] == (tri2)->verts[0]) ||			\
      ((tri1)->verts[1] == (tri2)->verts[1]) ||			\
      ((tri1)->verts[1] == (tri2)->verts[2]) ||			\
      ((tri1)->verts[2] == (tri2)->verts[0]) ||			\
      ((tri1)->verts[2] == (tri2)->verts[1]) ||			\
      ((tri1)->verts[2] == (tri2)->verts[2])) ? TRUE : FALSE)
     

/*==================== 3D vector macros ===================*/
#define VEC3_ZERO(vec)	       { (vec)[0]=(vec)[1]=(vec)[2]=0; }
#define VEC3_NEG(dest,src)     { (dest)[0]= -(src)[0]; (dest)[1]= -(src)[1];(dest)[2]= -(src)[2];}
#define VEC3_EQ(a,b)           (((a)[0]==(b)[0]) && ((a)[1]==(b)[1]) && ((a)[2]==(b)[2]))
#define ZERO3_TOL(a, tol)      { (a)[0] = (((a)[0]<tol)&&((a)[0]>-tol))?0.0:(a)[0];\
				 (a)[1] = (((a)[1]<tol)&&((a)[1]>-tol))?0.0:(a)[1];\
			         (a)[2] = (((a)[2]<tol)&&((a)[2]>-tol))?0.0:(a)[2];\
			       }

#define VEC3_V_OP_S(a,b,op,c)  {  (a)[0] = (b)[0] op (c);  \
				  (a)[1] = (b)[1] op (c);  \
				  (a)[2] = (b)[2] op (c);  }

#define VEC3_V_OP_V(a,b,op,c)  { (a)[0] = (b)[0] op (c)[0]; \
				 (a)[1] = (b)[1] op (c)[1]; \
				 (a)[2] = (b)[2] op (c)[2]; \
				}
#define VEC3_V_OP_V_OP_S(a,b,op1,c,op2,d)  \
				{ (a)[0] = (b)[0] op1 (c)[0] op2 (d); \
				  (a)[1] = (b)[1] op1 (c)[1] op2 (d); \
				  (a)[2] = (b)[2] op1 (c)[2] op2 (d); }

#define VEC3_VOPV_OP_S(a,b,op1,c,op2,d)  \
				{ (a)[0] = ((b)[0] op1 (c)[0]) op2 (d); \
				  (a)[1] = ((b)[1] op1 (c)[1]) op2 (d); \
				  (a)[2] = ((b)[2] op1 (c)[2]) op2 (d); }

#define VEC3_V_OP_V_OP_V(a,b,op1,c,op2,d)  \
				{ (a)[0] = (b)[0] op1 (c)[0] op2 (d)[0]; \
				  (a)[1] = (b)[1] op1 (c)[1] op2 (d)[1]; \
				  (a)[2] = (b)[2] op1 (c)[2] op2 (d)[2]; }

#define VEC3_ASN_OP(a,op,b)      {a[0] op b[0]; a[1] op b[1]; a[2] op b[2];}

#define DOTPROD3(a, b)		 ((a)[0]*(b)[0] + (a)[1]*(b)[1] + (a)[2]*(b)[2])

#define CROSSPROD3(a,b,c)       {(a)[0]=(b)[1]*(c)[2]-(b)[2]*(c)[1]; \
                                 (a)[1]=(b)[2]*(c)[0]-(b)[0]*(c)[2]; \
                                 (a)[2]=(b)[0]*(c)[1]-(b)[1]*(c)[0];}

#define NORMALIZE3(a)                                               \
        {                                                           \
	 double LAmag;                                              \
	 LAmag=1./sqrt((a)[0]*(a)[0]+(a)[1]*(a)[1]+(a)[2]*(a)[2]);  \
	 (a)[0] *= LAmag; (a)[1] *= LAmag; (a)[2] *= LAmag;         \
        }

#define NORMALIZE_PLANE3(a)						\
        {								\
	    double LAmag;						\
	    LAmag=1./sqrt((a)[0]*(a)[0]+(a)[1]*(a)[1]+(a)[2]*(a)[2]);	\
	    (a)[0] *= LAmag; (a)[1] *= LAmag;  				\
	    (a)[2] *= LAmag; (a)[3] *= LAmag;				\
	}

#define SQ_DIST3(a, b)          (((a)[0]-(b)[0])*((a)[0]-(b)[0]) +      \
                                 ((a)[1]-(b)[1])*((a)[1]-(b)[1]) +      \
                                 ((a)[2]-(b)[2])*((a)[2]-(b)[2]))

/* assumes normalized plane normal */
#define POINT_PLANE_DIST(point, plane)  (DOTPROD3(point,plane)-plane[3])

#define PRINT_VEC3(a,string)    {fprintf(stderr,"%s: (%g %g %g)\n",string, (a)[0], \
				  			   (a)[1], (a)[2]);}

/*================= General vector macros ===================*/
#define VEC_ZERO(a,m)	         {int LAi; for(LAi=0; LAi<m; (a)[LAi++] = 0); }
#define VEC_NEG(a,b,m)        	 {int LAi; for(LAi=0; LAi<m; LAi++) (a)[LAi] = -(b)[LAi]; }

#define VEC_V_OP_S(a,b,op,c,m)  {int LAi; for(LAi=0; LAi<(m); LAi++)   \
				    (a)[LAi] = (b)[LAi] op (c); }
#define VEC_V_OP_V(a,b,op,c,m)  {int LAi; for(LAi=0; LAi<(m); LAi++)   \
				    (a)[LAi] = (b)[LAi] op (c)[LAi]; }
#define VEC_V_OP_V_OP_S(a,b,op1,c,op2,d) \
				{int LAi; for(LAi=0; LAi<(m); LAi++)   \
				    (a)[LAi] = (b)[LAi] op1 (c)[LAi] op2 (d); }
#define VEC_V_OP_V_OP_V(a,b,op1,c,op2,d) \
				{int LAi; for(LAi=0; LAi<(m); LAi++)   \
				    (a)[LAi] = (b)[LAi] op1 (c)[LAi] op2 (d)[LAi]; }
#define VEC_ASN_OP(a,op,b,m){int LAi; for(LAi=0; LAi<(m); LAi++) (a)[LAi] op (b)[LAi];}

#define NORMALIZE(a,m)		{ int LAi; double LAmag;            \
				  for(LAi=0,LAmag=0.;LAi<(m);LAi++) \
				    LAmag += (a)[LAi]*(a)[LAi];	  \
				  LAmag=1./sqrt(LAmag);		  \
				  for(LAi=0; LAi<(m); (a)[LAi++] *= LAmag); }
#define PRINT_VEC(a, m, string)	{ int LAi; printf("%s : (", string);	  \
				  for(LAi=0; LAi<(m); LAi++)	  \
					printf("%g ",(a)[LAi]);	  \
				  printf(")\n"); }


/*==================== Transformation Macros===================*/
#define TRANSFORM_POINT(dest_point, matrix, src_point)		\
        {							\
            (dest_point)[X] = (matrix)[0][0]*(src_point)[X] +	\
		              (matrix)[0][1]*(src_point)[Y] +	\
			      (matrix)[0][2]*(src_point)[Z] +	\
			      (matrix)[0][3];			\
            (dest_point)[Y] = (matrix)[1][0]*(src_point)[X] +	\
		              (matrix)[1][1]*(src_point)[Y] +	\
			      (matrix)[1][2]*(src_point)[Z] +	\
			      (matrix)[1][3];			\
            (dest_point)[Z] = (matrix)[2][0]*(src_point)[X] +	\
		              (matrix)[2][1]*(src_point)[Y] +	\
			      (matrix)[2][2]*(src_point)[Z] +	\
			      (matrix)[2][3];			\
        }

#define TRANSFORM_VECTOR(dest_vector, matrix, src_vector)	\
        {							\
            (dest_vector)[X] = (matrix)[0][0]*(src_vector)[X] +	\
		               (matrix)[0][1]*(src_vector)[Y] +	\
			       (matrix)[0][2]*(src_vector)[Z];	\
            (dest_vector)[Y] = (matrix)[1][0]*(src_vector)[X] +	\
		               (matrix)[1][1]*(src_vector)[Y] +	\
			       (matrix)[1][2]*(src_vector)[Z];	\
            (dest_vector)[Z] = (matrix)[2][0]*(src_vector)[X] +	\
		               (matrix)[2][1]*(src_vector)[Y] +	\
			       (matrix)[2][2]*(src_vector)[Z];	\
        }




#define VERTS_TO_LINESEG(vert1, vert2, lineseg)				\
        {								\
	    VEC3_ASN_OP((lineseg).endpoints[0], =, (vert1)->coord);	\
	    VEC3_ASN_OP((lineseg).endpoints[1], =, (vert2)->coord);	\
	}

/*---------------------------------- Types ----------------------------------*/


typedef double Point[3];
typedef double Vector[3];
typedef double Matrix[3][4];
typedef double Extents[2][3];

typedef struct Vertex
{
    int        	      id;
    Point      	      coord;
    Vector     	      normal;
    float             curvature;
#ifdef VERTEX_EPSILONS
    float                 epsilon;
#endif
    struct Triangle **tris;
    int               num_tris;
    struct Edge     **edges;
    int               num_edges;
    void             *other_props;
    int               handy_mark;
} Vertex;

typedef struct Edge
{
    int              id;
    struct Vertex   *verts[2];
    struct Triangle *tris[2];
    int              handy_mark;
} Edge;

typedef struct Triangle
{
    int            id;
    struct Vertex *verts[3];
    struct Edge   *edges[3];
    double         plane_eq[4];   /* [A,B,C,D], where Ax + By + Cz == D */
    int            handy_mark;
} Triangle;

typedef struct RawTriangle
{
    Point  verts[3];
    double plane_eq[4];
} RawTriangle;

typedef struct RawEdge
{
    Point  verts[2];
} RawEdge;

typedef struct LineSegment
{
    Point  endpoints[2];
} LineSegment;

typedef struct PolyLine
{
    Vertex   **verts;
    int        num_verts;
} PolyLine;

typedef struct Surface
{
    Vertex   *verts;
    int       num_verts;
   	   
    Edge     *edges;
    int       num_edges;

    Triangle *tris;
    int       num_tris;
} Surface;

/*---------------------------- Function Prototypes --------------------------*/

void find_plane_eq(Point pt0, Point pt1, Point pt2, double plane_eq[4]);
void tri_to_rawtri(Triangle *tri, RawTriangle *rawtri);
int get_common_index_verts(Triangle *tri1, Triangle *tri2, int common_verts[3][2]);
int get_common_coord_verts(Triangle *tri1, Triangle *tri2, int common_verts[3][2]);
int get_common_coord_edge_tri_verts(Edge *edge, Triangle *tri,
				    int common_verts[2][2]);
void free_surface(Surface *surface);
double point_line_distance(Vertex *point,
			   Vertex *line_vert1, Vertex *line_vert2);

/*---------------------------Globals (externed)------------------------------*/



/* Protection from multiple includes. */
#endif /*INCLUDED_OFFSET_GEOMETRY_H*/


