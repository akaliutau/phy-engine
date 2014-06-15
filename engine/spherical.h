/* spherical.h: routine for sampling directions on a spherical triangle or 
 * quadrilateral using Arvo's technique published in SIGGRAPH '95 p 437 
 * and similar stuff. */

#ifndef _SPHERICAL_H_
#define _SPHERICAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"

#include "patch_type.h"
typedef struct COORDSYS {
  VECTOR X, Y, Z;
} COORDSYS;

typedef struct DCOORDSYS {
  DVECTOR X, Y, Z;
} DCOORDSYS;

/* creates a coordinate system on the patch P with Z direction along the normal */
extern void PatchCoordSys(PATCH *P, COORDSYS *coord);

/* creates a coordinate system with the given UNIT direction vector as Z-axis */
extern void VectorCoordSys(VECTOR *Z, COORDSYS *coord);
extern void DVectorCoordSys(DVECTOR *Z, DCOORDSYS *coord);

extern void PrintCoordSys(FILE *fp, COORDSYS *coord);

/* Given a unit vector and a coordinate system, this routine computes the spherical 
 * coordinates phi and theta of the vector with respect to the coordinate system */
extern void VectorToSphericalCoord(VECTOR *C, COORDSYS *coordsys, double *phi, double *theta);

extern void DSphericalCoordToVector(DCOORDSYS *coordsys, double *phi, double *theta, DVECTOR *C);

extern void SphericalCoordToVector(COORDSYS *coordsys, double *phi, double *theta, VECTOR *C);


/* Given a unit vector, this routine computes the spherical coordinates
   with respect to principal axes X (1 0 0) Y (0 1 0) and Z (0 0 1) */

void VectorToSpherical_Principal(VECTOR *C, double *phi, double *theta);

/* Given spherical coordinates, this routine computes the corresponding
   unit direction vector with respect to principal axes */

void SphericalToVector_Principal(double phi, double theta, VECTOR *C);



/* J. Arvo, Stratified Sampling of Hemispherical Triangles, SIGGRAPH 95 p 437 */
extern DVECTOR SampleSphericalTriangle(DVECTOR *A, DVECTOR *B, DVECTOR *C, 
				       double Area, double alpha, 
				       double xi_1, double xi_2, double *pdf_value);

/* Girard's formula, see Arvo, "Irradiance Tensors", SIGGRAPH '95. The area is
 * returned in Area, the angle at A in alpha */
extern void SphTriangleArea(DVECTOR *A, DVECTOR *B, DVECTOR *C, 
			    double *Area, double *alpha);

/* solid angle subtended by the (convex) spherical polygon, Girard's formula but
 * for arbitrary convex spherical polygons and this routine does not return
 * any angles. The spherical polygon has n vertices given in the array C. */
extern double SphericalArea(int n, DVECTOR *C);

/* given to numbers xi_1 and xi_2 in [0,1], this routines finds the corresponding
 * direction from point x on patch P to patch Q using Arvo's SIGGRAPH '95 technique.
 * The direction is returned in <dir>, the solid angle Omega subtended by Q 
 * on the hemisphere at point x is returned. */
extern double PatchDirection(PATCH *P, POINT *x, PATCH *Q, double xi_1, double xi_2, 
			     VECTOR *dir);

/*samples the hemisphere according to a uniform distribution i.e. proportional to
 *hemisphere area.*/

extern VECTOR SampleHemisphereUniform(COORDSYS *coord, double xi_1, double xi_2, double *pdf_value);


/* samples the hemisphere according to a cos_theta distribution */
extern VECTOR SampleHemisphereCosTheta(COORDSYS *coord, double xi_1, double xi_2, double *pdf_value);


/* samples the hemisphere according to a cos_theta ^ n  distribution */
extern VECTOR SampleHemisphereCosNTheta(COORDSYS *coord, double n, double xi_1, double xi_2, double *pdf_value);


/*samples the hemisphere according to a uniform distribution i.e. proportional to
 *hemisphere area.*/

extern DVECTOR DSampleHemisphereUniform(DCOORDSYS *coord, double xi_1, double xi_2, double *pdf_value);


/* samples the hemisphere according to a cos_theta distribution */
extern DVECTOR DSampleHemisphereCosTheta(DCOORDSYS *coord, double xi_1, double xi_2, double *pdf_value);


/* samples the hemisphere according to a cos_theta ^ n  distribution */
extern DVECTOR DSampleHemisphereCosNTheta(DCOORDSYS *coord, double n, double xi_1, double xi_2, double *pdf_value);


/* defines a nice grid for stratified sampling */
extern void GetNrDivisions(int samples, int *divs1, int *divs2);

#ifdef __cplusplus
}
#endif

#endif /*_SPHERICAL_H_*/

