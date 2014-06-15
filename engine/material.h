/* material.h */

#ifndef _MATERIAL_H_
#define _MATERIAL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "edf.h"
#include "bsdf.h"
#include "hit.h"

typedef struct MATERIAL {
  char 	*name;		/* material name */
  EDF 	*edf;		/* emittance distribution function */
  BSDF  *bsdf;          /* reflection and transmission together */
  int 	sided;		/* 1 for 1-sided surface, 0 for 2-sided, see MGF docs */
  void  *radiance_data;
} MATERIAL;

extern MATERIAL defaultMaterial;

extern MATERIAL *MaterialCreate(char *name, 
				EDF *edf, BSDF *bsdf,
				int sided);

extern MATERIAL *MaterialDuplicate(MATERIAL *mat);

extern void MaterialDestroy(MATERIAL *material);

extern void MaterialPrint(FILE *out, MATERIAL *material);

/* Computes shading frame at hit point. Z is the shading normal. Returns FALSE
 * if the shading frame could not be determined. 
 * If X and Y are null pointers, only the shading normal is returned in Z
 * possibly avoiding computations of the X and Y axis. */
extern int MaterialShadingFrame(HITREC *hit, VECTOR *X, VECTOR *Y, VECTOR *Z);

#ifdef __cplusplus
}
#endif

#endif /*_MATERIAL_H_*/
