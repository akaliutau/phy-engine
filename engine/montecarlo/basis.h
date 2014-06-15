/* basis.h: higher order approximations for Galerkin radiosity */

#ifndef _BASIS_H_
#define _BASIS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mcrad.h"
#include "translatenames.h"
#include "color.h"

/* no basis consists of more than this number of basis functions */
#define MAX_BASIS_SIZE	10

typedef double FILTER[MAX_BASIS_SIZE][MAX_BASIS_SIZE];
typedef FILTER FILTERTABLE[4];

/* all bases are orthonormal on their standard domain. */
typedef struct BASIS {
  char *description;	/* for debugging */

  int size;		/* number of basis functions */
  
  /* function[alpha](u,v) evaluates \alpha-th basis function at (u,v). */
  double (**function)(double u, double v);

  /* same, but evaluated dual basis function (on standard domain) */
  double (**dualfunction)(double u, double v);

  /* push-pull filter coefficients for regular quadtree subdivision.
   * regular_filter[sigma][alpha][beta] is the filter coefficient
   * relating basis function alpha on the parent element with
   * basis function beta on the regular subelement with index
   * sigma. See PushRadiance() and PullRadiance() in basis.c. */
  FILTERTABLE *regular_filter;
} BASIS;

/* bases for quadrilaterals and triangles, implemented in basis[quad|tri].c */
extern BASIS triBasis, quadBasis, clusterBasis;

#define NR_APPROX_TYPES 5
typedef enum APPROX_TYPE {
  AT_CONSTANT	= 0, 	/* 1 */
  AT_LINEAR	= 1, 	/* 1, u, v */
  AT_BILINEAR	= 2, 	/* 1, u, v, uv */
  AT_QUADRATIC	= 3, 	/* 1, u, v, uv, u2, v2 */
  AT_CUBIC	= 4	/* 1, u, v, uv, u2, v2, u3, u2v, uv2, v3 */
} APPROX_TYPE;

/* description of the approximation types */
extern struct APPROXDESC {
  char *name;
  int basis_size;
} approxdesc[NR_APPROX_TYPES];

#define NR_ELEMENT_TYPES 2
typedef enum ELEMENT_TYPE {
  ET_TRIANGLE	= 0,
  ET_QUAD	= 1
} ELEMENT_TYPE;

/* orthonormal canonical basis of given order for given type of elements */
extern BASIS basis[NR_ELEMENT_TYPES][NR_APPROX_TYPES], dummyBasis;

/* initialises table of bases */
extern void InitBasis(void);

/* prints info about basis */
extern void PrintBasis(BASIS *basis);

/* returns color at a given point, with parameters (u,v) */
extern COLOR ColorAtUV(BASIS *basis, COLOR *rad, double u, double v);

/* these routines add the result of filterting down/up the source
 * coefficients to the destination coefficients */
extern void FilterColorDown(COLOR *parent, FILTER *h, COLOR *child, int n);
extern void FilterColorUp(COLOR *child, FILTER *h, COLOR *parent, int n, double areafactor);

#ifdef __cplusplus
}
#endif

#endif /*_BASIS_H_*/
