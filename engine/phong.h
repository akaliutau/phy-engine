/* phong.h: Phong type EDFs, BRDFs, BTDFs */

#ifndef _PHONG_H_
#define _PHONG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "color.h"
#include "edf_methods.h"
#include "brdf_methods.h"
#include "btdf_methods.h"

typedef struct PHONG_BRDF {
  COLOR Kd, Ks;
  float avgKd, avgKs;
  float Ns;
} PHONG_BRDF;

typedef struct PHONG_EDF {
  COLOR Kd, kd, Ks;
  float Ns;
} PHONG_EDF;

typedef struct PHONG_BTDF {
  COLOR Kd, Ks;
  float avgKd, avgKs;
  float Ns;
  REFRACTIONINDEX refrIndex;
} PHONG_BTDF;

  /* Define the phong exponent making the difference
     between glossy and highly specular reflection/transmission.
     Choice is arbitrary for the moment */

#define PHONG_LOWEST_SPECULAR_EXP 250
#define PHONG_IS_SPECULAR(p) ((p).Ns >= PHONG_LOWEST_SPECULAR_EXP)

/* creates Phong type EDF, BRDF, BTDF data structs:
 * Kd = diffuse emittance [W/m^2], reflectance or transmittance (number between 0 and 1)
 * Ks = specular emittance, reflectance or transmittance (same dimensions as Kd)
 * Ns = Phong exponent.
 * note: Emittance is total power emitted by the light source per unit of area. */
extern PHONG_EDF *PhongEdfCreate(COLOR *Kd, COLOR *Ks, double Ns);
extern PHONG_BRDF *PhongBrdfCreate(COLOR *Kd, COLOR *Ks, double Ns);
extern PHONG_BTDF *PhongBtdfCreate(COLOR *Kd, COLOR *Ks, double Ns, double nr, double ni);

/* methods for manipulating Phong type EDFs, BRDFs, BTDFs */
extern EDF_METHODS PhongEdfMethods;
extern BRDF_METHODS PhongBrdfMethods;
extern BTDF_METHODS PhongBtdfMethods;

#define BsdfIsPhongBrdf(bsdf) (bsdf->methods == &PhongBrdfMethods)
#define BsdfGetPhongBrdf(bsdf) (BsdfIsPhongBrdf ? (PHONG_BRDF*)(bsdf->data) : (PHONG_BRDF*)NULL)


#ifdef __cplusplus
}
#endif

#endif /*_PHONG_H_*/
