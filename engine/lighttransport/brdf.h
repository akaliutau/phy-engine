/* brdf.h: Bidirectional Reflectance Distribution Functions. */

#ifndef _BRDF_H_
#define _BRDF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "vector.h"
#include "xxdf.h"
#include "brdf_methods.h"
#include "hit.h"

/* generic BRDF struct */
typedef struct BRDF {
  void *data;
  struct BRDF_METHODS *methods;
} BRDF;

/* Creates a BRDF instance with given data and methods. A pointer
 * to the created BRDF struct is returned. */
extern BRDF *BrdfCreate(void *data, BRDF_METHODS *methods);

/* Creates and returns a duplicate of the given BRDF */
extern BRDF *BrdfDuplicate(BRDF *brdf);

/* Creates an editor widget for the BRDF, returns the Widget casted
 * to a void * in order not to have to include all X window system files. */
extern void *BrdfCreateEditor(void *parent, BRDF *brdf);

/* disposes of the memory occupied by the BRDF instance */
extern void BrdfDestroy(BRDF *brdf);

/* Print the brdf data the to specified file pointer */
extern void BrdfPrint(FILE *out, BRDF *brdf);

/* Returns the reflectance of hte BRDF according to the flags */
extern COLOR BrdfReflectance(BRDF *brdf, XXDFFLAGS flags);

# define BrdfDiffuseReflectance(brdf) BrdfReflectance((brdf), DIFFUSE_COMPONENT)
# define BrdfGlossyReflectance(brdf) BrdfReflectance((brdf), GLOSSY_COMPONENT)
# define BrdfSpecularReflectance(brdf) BrdfReflectance((brdf), SPECULAR_COMPONENT)

/************* BRDF Evaluation functions ****************/

/* BRDF evaluation functions : 
 *
 *   VECTOR in : incoming ray direction (to patch)
 *   VECTOR out : reflected ray direction (from patch)
 *   VECTOR normal : normal vector
 *   XXDFFLAGS flags : flags indicating which components must be
 *     evaluated
 */

extern COLOR BrdfEval(BRDF *brdf, VECTOR *in, VECTOR *out, VECTOR *normal,
 		      XXDFFLAGS flags);

extern VECTOR BrdfSample(BRDF *brdf, VECTOR *in, 
			VECTOR *normal, int doRussianRoulette,
			XXDFFLAGS flags, double x_1, double x_2, 
			double *pdf);


extern void BrdfEvalPdf(BRDF *brdf, VECTOR *in, 
			VECTOR *out, VECTOR *normal, 			
			XXDFFLAGS flags, double *pdf, double *pdfRR);

#ifdef __cplusplus
}
#endif

#endif /*_BRDF_H_*/


