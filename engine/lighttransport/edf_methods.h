/* edf_methods.h: EDF methods */

#ifndef _EDF_METHODS_H_
#define _EDF_METHODS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "color.h"
#include "vector.h"
#include "xxdf.h"
#include "hit.h"

/* EDF methods: every kind of EDF needs to have these functions
 * implemented. */
typedef struct EDF_METHODS {
  /* returns the emittance */
  COLOR (*Emittance)(void *data, HITREC *hit, XXDFFLAGS flags);

  int (*IsTextured)(void *data);

  /* evaluates the edf */
  COLOR (*Eval)(void *data, HITREC *hit, VECTOR *out, XXDFFLAGS flags, double *pdf);

  /* samples the edf */
  VECTOR (*Sample)(void *data, HITREC *hit, XXDFFLAGS flags, double xi1, double xi2, COLOR *emitted_radiance, double *pdf);

  
  /* Computes shading frame at hit point. Returns TRUE if succesful and
   * FALSE if not. X and Y may be null pointers. */
  int (*ShadingFrame)(void *data, HITREC *hit, VECTOR *X, VECTOR *Y, VECTOR *Z);

  /* prints the EDF data to the specified file */
  void (*Print)(FILE *out, void *data);

  /* creates a duplicate of the EDF data */
  void *(*Duplicate)(void *data);

  /* creates a EDF editor widget (included in the material editor implemented
   * in ui_material.c whenever appropriate). Returns the Widget, casted to a
   * void * in order not to have to include all X window header files. */
  void *(*CreateEditor)(void *parent, void *data);

  /* disposes of the EDF data */
  void (*Destroy)(void *data);
} EDF_METHODS;

#ifdef __cplusplus
}
#endif

#endif /*_EDF_METHODS_H_*/
