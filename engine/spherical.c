

#include <math.h>

#include "spherical.h"
#include "patch.h"

#include "error.h"



void PrintCoordSys(FILE *fp, COORDSYS *coord)
{
  fprintf(fp, "COORDSYS\nX =");
  VectorPrint(fp, coord->X);
  fprintf(fp, "\nY =");
  VectorPrint(fp, coord->Y);
  fprintf(fp, "\nZ =");
  VectorPrint(fp, coord->Z);

  fprintf(fp, "\n\n");
 
}


void PatchCoordSys(PATCH *P, COORDSYS *coord)
{
  coord->Z = P->normal;
  VECTORSUBTRACT(*P->vertex[1]->point, *P->vertex[0]->point, coord->X);
  VECTORNORMALIZE(coord->X);
  VECTORCROSSPRODUCT(coord->Z, coord->X, coord->Y);
}


extern void VectorCoordSys(VECTOR *Z, COORDSYS *coord)
{

#ifdef NEVER
  VECTOR Axis;

  coord->Z = *Z;

  switch(VectorDominantCoord(Z))
  {
  case XNORMAL:
    VECTORSET(Axis, 0, 0, 1);
    break;
  case YNORMAL:
    VECTORSET(Axis, 0, 0, 1);
    break;
  case ZNORMAL:
    VECTORSET(Axis, 0, 1, 0);
    break;
  }

  VECTORCROSSPRODUCT(Axis, *Z, coord->X);
  VECTORNORMALISE(coord->X);

  VECTORCROSSPRODUCT(*Z, coord->X, coord->Y);
  VECTORNORMALISE(coord->Y);  
#endif

  double zz;

  coord->Z = *Z;
  zz = sqrt(1 - Z->z*Z->z);
  
  if(zz < EPSILON)
  {
    coord->X.x = 1.;
    coord->X.y = 0.;
    coord->X.z = 0.;
  }
  else
  {
    coord->X.x =  Z->y/zz;
    coord->X.y = -Z->x/zz;
    coord->X.z = 0.;
  }
  
  VECTORCROSSPRODUCT(coord->Z, coord->X, coord->Y);
}


extern void DVectorCoordSys(DVECTOR *Z, DCOORDSYS *coord)
{
  DVECTOR Axis;

  coord->Z = *Z;

  switch(DVectorDominantCoord(Z))
  {
  case XNORMAL:
    VECTORSET(Axis, 0, 0, 1);
    break;
  case YNORMAL:
    VECTORSET(Axis, 0, 0, 1);
    break;
  case ZNORMAL:
    VECTORSET(Axis, 0, 1, 0);
    break;
  }

  VECTORCROSSPRODUCT(Axis, *Z, coord->X);
  VECTORNORMALISE(coord->X);

  VECTORCROSSPRODUCT(*Z, coord->X, coord->Y);
  VECTORNORMALISE(coord->Y);	
}


DVECTOR SampleSphericalTriangle(DVECTOR *A, DVECTOR *B, DVECTOR *C, 
				double Area, double alpha, 
				double xi_1, double xi_2,
				double *pdf_value)
{
  double Area1, s, t, u, v, cosalpha, sinalpha, cosc, q, q1, z, z1;
  DVECTOR C1, P;
  
  cosc = VECTORDOTPRODUCT(*B, *A);
  
  cosalpha = cos(alpha);
  sinalpha = sin(alpha);
  
  Area1 = xi_1 * Area;
  
  s = sin(Area1 - alpha);
  t = cos(Area1 - alpha);
  
  u = t - cosalpha;
  v = s + sinalpha * cosc;
  
  q1 = (v * s + u * t) * sinalpha;
  if (q1 > 1e-40 || q1 < -1e-40)	
    q = ((v * t - u * s) * cosalpha - v) / q1;
  else	
    q = 1. - xi_1 * (1. - VECTORDOTPRODUCT(*A, *C));	
  if (q < -1.) q = -1.;
  if (q >  1.) q =  1.;
  
  VECTORORTHOCOMP(*C, *A, C1);
  VECTORNORMALIZE(C1);
  q1 = sqrt(1. - q*q);
  VECTORSCALE(q1, C1, C1);
  VECTORSUMSCALED(C1, q, *A, C1);
  
  z = 1. - xi_2 * (1. - VECTORDOTPRODUCT(C1, *B));
  if (z < -1.) z = -1.;
  if (z >  1.) z =  1.;
  
  VECTORORTHOCOMP(C1, *B, P);
  VECTORNORMALIZE(P);
  z1 = sqrt(1. - z*z);
  VECTORSCALE(z1, P, P);
  VECTORSUMSCALED(P, z, *B, P);

  *pdf_value = 1.0 / Area ;
  
  return P;
}


void SphTriangleArea(DVECTOR *A, DVECTOR *B, DVECTOR *C, 
		     double *Area, double *alpha)
{
  double beta, gamma, cosalpha, cosbeta, cosgamma, n1, n2, n3;
  DVECTOR N1, N2, N3; 

  VECTORCROSSPRODUCT(*C, *B, N1); n1 = VECTORNORM(N1);
  VECTORCROSSPRODUCT(*A, *C, N2); n2 = VECTORNORM(N2);
  VECTORCROSSPRODUCT(*B, *A, N3); n3 = VECTORNORM(N3);

  cosalpha = -VECTORDOTPRODUCT(N2, N3) / (n2*n3);
  if (cosalpha < -1.) cosalpha = -1.;
  if (cosalpha >  1.) cosalpha =  1.;
  *alpha = acos(cosalpha);

  cosbeta  = -VECTORDOTPRODUCT(N3, N1) / (n3*n1);
  if (cosbeta  < -1.) cosbeta  = -1.;
  if (cosbeta  >  1.) cosbeta  =  1.;
  beta   = acos(cosbeta);

  cosgamma = -VECTORDOTPRODUCT(N1, N2) / (n1*n2);
  if (cosgamma < -1.) cosgamma = -1.;
  if (cosgamma >  1.) cosgamma =  1.;
  gamma  = acos(cosgamma);

  *Area = *alpha + beta + gamma - M_PI;		      
}


double SphericalArea(int n, DVECTOR *C)
{
  DVECTOR N0, N1, N2;
  int i;
  double Omega, n0, n1, n2;

  Omega = 0.;
  VECTORCROSSPRODUCT(C[0], C[1], N0); n0 = VECTORNORM(N0);
  N2 = N0; n2 = n0;
  for (i=1; i<n; i++) {
    N1 = N2; n1 = n2;
    VECTORCROSSPRODUCT(C[i], C[(i+1)%n], N2); n2 = VECTORNORM(N2);
    Omega += acos(-VECTORDOTPRODUCT(N1,N2)/(n1*n2));
  }
  Omega += acos(-VECTORDOTPRODUCT(N2,N0)/(n2*n0));
  
  Omega -= (double)(n-2) * M_PI;
  
  return Omega;	
}


void VectorToSphericalCoord(VECTOR *C, COORDSYS *coordsys, double *phi, double *theta)
{
  double x, y, z;
  VECTOR c;

  z = VECTORDOTPRODUCT(*C, coordsys->Z);
  if(z > 1.0) z = 1.0;  
  if(z < -1.0) z = -1.0;

  *theta = acos(z);
	
  VECTORSUMSCALED(*C, -z, coordsys->Z, c);
  VECTORNORMALIZE(c);
  x = VECTORDOTPRODUCT(c, coordsys->X);
  y = VECTORDOTPRODUCT(c, coordsys->Y);

  if(x > 1.0) x = 1.0;  
  if(x < -1.0) x = -1.0;
  *phi = acos(x);
  if (y < 0.) *phi = 2. * M_PI - *phi;
}



extern void DSphericalCoordToVector(DCOORDSYS *coordsys, double *phi, 
				    double *theta, DVECTOR *C)
{
  double cos_phi, sin_phi, cos_theta, sin_theta;
  DVECTOR CP;
  
  cos_phi = cos(*phi);
  sin_phi = sin(*phi);
  cos_theta = cos(*theta);
  sin_theta = sin(*theta);

  VECTORCOMB2(cos_phi, coordsys->X, sin_phi, coordsys->Y, CP);
  VECTORCOMB2(cos_theta, coordsys->Z, sin_theta, CP, *C);
}

extern void SphericalCoordToVector(COORDSYS *coordsys, double *phi, 
				   double *theta, VECTOR *C)
{
  double cos_phi, sin_phi, cos_theta, sin_theta;
  VECTOR CP;
  
  cos_phi = cos(*phi);
  sin_phi = sin(*phi);
  cos_theta = cos(*theta);
  sin_theta = sin(*theta);

  VECTORCOMB2(cos_phi, coordsys->X, sin_phi, coordsys->Y, CP);
  VECTORCOMB2(cos_theta, coordsys->Z, sin_theta, CP, *C);
}



void VectorToSpherical_Principal(VECTOR *C, double *phi, double *theta)
{
  double x, norm;

  *theta = acos(C->z);
  norm = sqrt(C->x * C->x + C->y * C->y);
  if(norm > EPSILON)
  {
    x = C->x / norm;
    *phi = acos(x);
    if(C->y < 0.) *phi = 2. * M_PI - *phi;
  }
  else
  {
    *phi = 0.;
  }
}



void SphericalToVector_Principal(double phi, double theta, VECTOR *C)
{
  double scale;
  C->z = cos(theta);
  scale = sqrt(1.0 / (1 - C->z * C->z));
  C->x = cos(phi) / scale;
  C->y = sin(phi) / scale;
}



double PatchDirection(PATCH *P, POINT *x, PATCH *Q, double xi_1, double xi_2, 
		      VECTOR *dir)
{
  static DVECTOR C[PATCHMAXVERTICES], d;
  static double Omega, Omega1, Omega2;
  static double alpha, alpha2;
  static PATCH *lastQ = (PATCH *)NULL, *lastP = (PATCH *)NULL;
  static POINT lastx;
  int i;
  double xi_3, pdf;

  if (P != lastP || Q != lastQ || !VECTOREQUAL(*x, lastx, EPSILON)) { 
    
    for (i=0; i<Q->nrvertices; i++) {
      VECTORSUBTRACT(*Q->vertex[i]->point, *x, C[i]);
      VECTORNORMALIZE(C[i]);
    }

    
    SphTriangleArea(&C[2], &C[0], &C[1], &Omega, &alpha);
    Omega1 = Omega;

    if (Q->nrvertices == 4) {
      SphTriangleArea(&C[3], &C[0], &C[2], &Omega2, &alpha2);
      Omega = Omega1 + Omega2;
    }

    lastP = P;
    lastQ = Q;
    lastx = *x;
  }

  if (Q->nrvertices == 4) {
    if (xi_1 <= Omega1 / Omega) {
      xi_3 = xi_1 * Omega / Omega1;
      d = SampleSphericalTriangle(&C[2], &C[0], &C[1], Omega1, alpha, xi_3, xi_2, &pdf);
    } else {
      xi_3 = (xi_1 * Omega - Omega1) / Omega2;
      d = SampleSphericalTriangle(&C[3], &C[0], &C[2], Omega2, alpha2, xi_3, xi_2, &pdf);
    }
  } else	
    d = SampleSphericalTriangle(&C[2], &C[0], &C[1], Omega, alpha, xi_1, xi_2, &pdf);
  
  dir->x = d.x;
  dir->y = d.y;
  dir->z = d.z;

  return Omega;
}




DVECTOR DSampleHemisphereUniform(DCOORDSYS *coord, double xi_1, double xi_2, double *pdf_value)
{
  double phi, cos_phi, sin_phi, cos_theta, sin_theta;
  DVECTOR dir;
  
  phi = 2. * M_PI * xi_1;
  cos_phi = cos(phi);
  sin_phi = sin(phi);
  cos_theta = 1.0-xi_2;
  sin_theta = sqrt(1-cos_theta*cos_theta);
  
  VECTORCOMB2(cos_phi, coord->X, 
	      sin_phi, coord->Y, 
	      dir);
  VECTORCOMB2(sin_theta, dir, 
	      cos_theta, coord->Z, 
	      dir);

  *pdf_value = .5 / M_PI;

  return dir;
}

VECTOR SampleHemisphereUniform(COORDSYS *coord, double xi_1, double xi_2, double *pdf_value)
{
  double phi, cos_phi, sin_phi, cos_theta, sin_theta;
  VECTOR dir;
  
  phi = 2. * M_PI * xi_1;
  cos_phi = cos(phi);
  sin_phi = sin(phi);
  cos_theta = 1.0-xi_2;
  sin_theta = sqrt(1-cos_theta*cos_theta);
  
  VECTORCOMB2(cos_phi, coord->X, 
	      sin_phi, coord->Y, 
	      dir);
  VECTORCOMB2(sin_theta, dir, 
	      cos_theta, coord->Z, 
	      dir);

  *pdf_value = .5 / M_PI;

  return dir;
}





DVECTOR DSampleHemisphereCosTheta(DCOORDSYS *coord, double xi_1, double xi_2, double *pdf_value)
{
  double phi, cos_phi, sin_phi, cos_theta, sin_theta;
  DVECTOR dir;
  
  phi = 2. * M_PI * xi_1;
  cos_phi = cos(phi);
  sin_phi = sin(phi);
  cos_theta = sqrt(1.0-xi_2);
  sin_theta = sqrt(xi_2);
  
  VECTORCOMB2(cos_phi, coord->X, 
	      sin_phi, coord->Y, 
	      dir);
  VECTORCOMB2(sin_theta, dir, 
	      cos_theta, coord->Z, 
	      dir);

  *pdf_value = cos_theta / M_PI ;

  return dir;
}

VECTOR SampleHemisphereCosTheta(COORDSYS *coord, double xi_1, double xi_2, double *pdf_value)
{
  double phi, cos_phi, sin_phi, cos_theta, sin_theta;
  VECTOR dir;
  
  phi = 2. * M_PI * xi_1;
  cos_phi = cos(phi);
  sin_phi = sin(phi);
  cos_theta = sqrt(1.0-xi_2);
  sin_theta = sqrt(xi_2);
  
  VECTORCOMB2(cos_phi, coord->X, 
	      sin_phi, coord->Y, 
	      dir);
  VECTORCOMB2(sin_theta, dir, 
	      cos_theta, coord->Z, 
	      dir);

  *pdf_value = cos_theta / M_PI ;

  return dir;
}



DVECTOR DSampleHemisphereCosNTheta(DCOORDSYS *coord, double n, double xi_1, double xi_2, double *pdf_value)
{
  double phi, cos_phi, sin_phi, cos_theta, sin_theta;
  DVECTOR dir;
  
  phi = 2. * M_PI * xi_1;
  cos_phi = cos(phi);
  sin_phi = sin(phi);
  cos_theta = pow(xi_2, 1.0/(n+1));
  sin_theta = sqrt(1.0 - cos_theta*cos_theta);
  
  VECTORCOMB2(cos_phi, coord->X, 
	      sin_phi, coord->Y, 
	      dir);
  VECTORCOMB2(sin_theta, dir, 
	      cos_theta, coord->Z, 
	      dir);

  *pdf_value = (n+1.0) * pow (cos_theta, n) / (2.0 * M_PI);

  return dir;
}

VECTOR SampleHemisphereCosNTheta(COORDSYS *coord, double n, double xi_1, double xi_2, double *pdf_value)
{
  double phi, cos_phi, sin_phi, cos_theta, sin_theta;
  VECTOR dir;
  
  phi = 2. * M_PI * xi_1;
  cos_phi = cos(phi);
  sin_phi = sin(phi);
  cos_theta = pow(xi_2, 1.0/(n+1));
  sin_theta = sqrt(1.0 - cos_theta*cos_theta);
  
  VECTORCOMB2(cos_phi, coord->X, 
	      sin_phi, coord->Y, 
	      dir);
  VECTORCOMB2(sin_theta, dir, 
	      cos_theta, coord->Z, 
	      dir);

  *pdf_value = (n+1.0) * pow (cos_theta, n) / (2.0 * M_PI);

  return dir;
}




void GetNrDivisions(int samples, int *divs1, int *divs2)
{
  if (samples <= 0) {
    *divs1 = 0; *divs2 = 0;
    return;
  }

  *divs1 = (int)ceil(sqrt((double)samples));
  *divs2 = samples/(*divs1);
  while ((*divs1) * (*divs2) != samples && (*divs1) > 1) {
    (*divs1) --;
    *divs2 = samples/(*divs1);			
  }
}

