

#include "brdf.h"
#include "pools.h"
#include "vector.h"
#include "error.h"

#ifdef NOPOOLS
#define NEWBRDF()	(BRDF *)Alloc(sizeof(BRDF))
#define DISPOSEBRDF(ptr) Free((char *)ptr, sizeof(BRDF))
#else
static POOL *brdfPool = (POOL *)NULL;
#define NEWBRDF()	(BRDF *)NewPoolCell(sizeof(BRDF), 0, "brdfs", &brdfPool)
#define DISPOSEBRDF(ptr) Dispose((char *)ptr, &brdfPool)
#endif


BRDF *BrdfCreate(void *data, BRDF_METHODS *methods)
{
  BRDF *brdf;

  brdf = NEWBRDF();
  brdf->data = data;
  brdf->methods = methods;

  return brdf;
}


BRDF *BrdfDuplicate(BRDF *obrdf)
{
  BRDF *brdf;

  if (!obrdf)
    return obrdf;

  brdf = NEWBRDF();
  brdf->data = obrdf->methods->Duplicate(obrdf->data);
  brdf->methods = obrdf->methods;

  return brdf;
}


void *BrdfCreateEditor(void *parent, BRDF *brdf)
{
  if (!brdf)
    Fatal(-1, "BrdfCreateEditor", "NULL brdf pointer passed.");
  return brdf->methods->CreateEditor(parent, brdf->data);
}


void BrdfDestroy(BRDF *brdf)
{
  if (!brdf) return;
  brdf->methods->Destroy(brdf->data);
  DISPOSEBRDF(brdf);
}


COLOR BrdfReflectance(BRDF *brdf, XXDFFLAGS flags)
{
  if (brdf && brdf->methods->Reflectance) {
    COLOR test = brdf->methods->Reflectance(brdf->data, flags);
    if (! finite(COLORAVERAGE(test))) {
      Fatal(-1, "BrdfReflectance", "Oops - test Rd is not finite!");
    }
    return test;
  }
  else {
    static COLOR refl;
    COLORCLEAR(refl);
    return refl;
  }
}



COLOR BrdfEval(BRDF *brdf, VECTOR *in, VECTOR *out, VECTOR *normal, XXDFFLAGS flags)
{
  if (brdf && brdf->methods->Eval)
     return brdf->methods->Eval(brdf->data, in, out, normal, flags);
  else {
    static COLOR refl;
    COLORCLEAR(refl);
    return refl;
  }
}



VECTOR BrdfSample(BRDF *brdf, VECTOR *in, 
		  VECTOR *normal, int doRussianRoulette,
		  XXDFFLAGS flags, double x_1, double x_2,
		  double *pdf)
{
  if (brdf && brdf->methods->Sample)
    return brdf->methods->Sample(brdf->data, in, normal, 
				 doRussianRoulette, flags, x_1, x_2, pdf);
  else {
    VECTOR dummy = {0., 0., 0.};
    *pdf = 0;
    return dummy;
  }
}


void BrdfEvalPdf(BRDF *brdf, 
		 VECTOR *in, VECTOR *out, VECTOR *normal, 
		 XXDFFLAGS flags,
		 double *pdf, double *pdfRR)
{
  if (brdf && brdf->methods->EvalPdf)
    brdf->methods->EvalPdf(brdf->data, in, out, 
			   normal, flags, pdf, pdfRR);
  else {
    *pdf = 0;
  }
}



void BrdfPrint(FILE *out, BRDF *brdf)
{
  if (!brdf)
    fprintf(out, "(NULL BRDF)\n");
  else
    brdf->methods->Print(out, brdf->data);
}


