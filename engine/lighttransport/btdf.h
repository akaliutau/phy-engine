/* btdf.h: Bidirectional Transmittance Distribution Functions. */

#ifndef _BTDF_H_
#define _BTDF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "btdf_methods.h"
#include "xxdf.h"

/* index of refraction data type. Normally when using BSDF's
   this should not be needed. In C++ this would of course
   be a plain complex number */

/* generic BTDF struct */
typedef struct BTDF {
  void *data;
  struct BTDF_METHODS *methods;
} BTDF;

/* Creates a BTDF instance with given data and methods. A pointer
 * to the created BTDF struct is returned. */
extern BTDF *BtdfCreate(void *data, BTDF_METHODS *methods);

/* Creates and returns a duplicate of the given BTDF */
extern BTDF *BtdfDuplicate(BTDF *btdf);

/* Creates an editor widget for the BTDF, returns the Widget casted
 * to a void * in order not to have to include all X window system files. */
extern void *BtdfCreateEditor(void *parent, BTDF *btdf);

/* disposes of the memory occupied by the BTDF instance */
extern void BtdfDestroy(BTDF *btdf);

/* Print the btdf data the to specified file pointer */
extern void BtdfPrint(FILE *out, BTDF *btdf);

/* Returns the transmittance of the BTDF */
extern COLOR BtdfTransmittance(BTDF *btdf, XXDFFLAGS flags);

#define BtdfDiffuseTransmittance(btdf) BtdfTransmittance((btdf), DIFFUSE_COMPONENT)
#define BtdfSpecularTransmittance(btdf) BtdfTransmittance((btdf), SPECULAR_COMPONENT)

/* Returns the index of refraction of the BTDF */
extern void BtdfIndexOfRefraction(BTDF *btdf, REFRACTIONINDEX *index);


/************* BTDF Evaluation functions ****************/

/* All components of the Btdf
 *
 * Vector directions :
 *
 * in : towards patch
 * out : from patch
 * normal : leaving from patch, on the incoming side.
 *          So in.normal < 0 !!!
 */

extern COLOR BtdfEval(BTDF *btdf, REFRACTIONINDEX inIndex, 
		      REFRACTIONINDEX outIndex, VECTOR *in, 
		      VECTOR *out, VECTOR *normal, XXDFFLAGS flags);

extern VECTOR BtdfSample(BTDF *btdf, REFRACTIONINDEX inIndex, 
			REFRACTIONINDEX outIndex, VECTOR *in, 
			VECTOR *normal, int doRussianRoulette,
			XXDFFLAGS flags, double x_1, double x_2, 
			double *pdf);

extern void BtdfEvalPdf(BTDF *btdf, REFRACTIONINDEX inIndex, 
			REFRACTIONINDEX outIndex, VECTOR *in, 
			VECTOR *out, VECTOR *normal,
			XXDFFLAGS flags, double *pdf, double *pdfRR);

#ifdef __cplusplus
}
#endif

#endif /*_BTDF_H_*/
