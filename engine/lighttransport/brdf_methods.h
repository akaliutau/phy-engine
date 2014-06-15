/* brdf_methods.h: BRDF methods */

#ifndef _BRDF_METHODS_H_
#define _BRDF_METHODS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "color.h"
#include "vector.h"
#include "xxdf.h"
#include "hit.h"

/* BRDF methods: every kind of BRDF needs to have these functions
 * implemented. */
typedef struct BRDF_METHODS {
   /* returns the reflectance */
   COLOR (*Reflectance)(void *data, XXDFFLAGS flags);

  /* eval brdf */
  COLOR (*Eval)(void *data, VECTOR *in, VECTOR *out, VECTOR *normal, XXDFFLAGS flags);

  VECTOR (*Sample)(void *data, VECTOR *in, 
		    VECTOR *normal, int doRussianRoulette,
		    XXDFFLAGS flags, double x_1, double x_2, 
		    double *pdf);

  void (*EvalPdf)(void *data, VECTOR *in, 
		  VECTOR *out, VECTOR *normal, 		  
		  XXDFFLAGS flags, double *pdf, double *pdfRR);
  
  /* prints the BRDF data to the specified file */
  void (*Print)(FILE *out, void *data);

  /* creates a duplicate of the BRDF data */
  void *(*Duplicate)(void *data);

  /* creates a BRDF editor widget (included in the material editor implemented
   * in ui_material.c whenever appropriate). Returns the Widget, casted to a
   * void * in order not to have to include all X window header files. */
  void *(*CreateEditor)(void *parent, void *data);

  /* disposes of the BRDF data */
  void (*Destroy)(void *data);
} BRDF_METHODS;

#ifdef __cplusplus
}
#endif

#endif /*_BRDF_METHODS_H_*/
