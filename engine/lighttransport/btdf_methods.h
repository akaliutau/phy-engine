/* btdf_methods.h: BTDF methods */

#ifndef _BTDF_METHODS_H_
#define _BTDF_METHODS_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "color.h"
#include <vector.h>
#include "xxdf.h"

/* BTDF methods: every kind of BTDF needs to have these functions
 * implemented. */
typedef struct BTDF_METHODS {
  /* returns the transmittance */
  COLOR (*Transmittance)(void *data, XXDFFLAGS flags);

  /* returns the index of refraction */
  void (*IndexOfRefraction)(void *data, REFRACTIONINDEX *index);

  COLOR (*Eval)(void *data, REFRACTIONINDEX inIndex, REFRACTIONINDEX outIndex, VECTOR *in, VECTOR *out, VECTOR *normal, XXDFFLAGS flags);

  VECTOR (*Sample)(void *data, REFRACTIONINDEX inIndex, 
		   REFRACTIONINDEX outIndex, VECTOR *in, 
		   VECTOR *normal, int doRussianRoulette,
		   XXDFFLAGS flags, double x_1, double x_2, 
		   double *pdf);

  void (*EvalPdf)(void *data, REFRACTIONINDEX inIndex, 
		  REFRACTIONINDEX outIndex, VECTOR *in, 
		  VECTOR *out, VECTOR *normal,
		  XXDFFLAGS flags, double *pdf, double *pdfRR);

  /* prints the BTDF data to the specified file */
  void (*Print)(FILE *out, void *data);

  /* creates a duplicate of the BTDF data */
  void *(*Duplicate)(void *data);

  /* creates a BTDF editor widget (included in the material editor implemented
   * in ui_material.c whenever appropriate). Returns the Widget, casted to a
   * void * in order not to have to include all X window header files. */
  void *(*CreateEditor)(void *parent, void *data);

  /* disposes of the BTDF data */
  void (*Destroy)(void *data);
} BTDF_METHODS;

#ifdef __cplusplus
}
#endif
#endif /*_BTDF_METHODS_H_*/
