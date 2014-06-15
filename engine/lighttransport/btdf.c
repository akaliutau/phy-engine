

#include "btdf.h"
#include "brdf.h"
#include "pools.h"
#include "Boolean.h"
#include "error.h"

#ifdef NOPOOLS
#define NEWBTDF()	(BTDF *)Alloc(sizeof(BTDF))
#define DISPOSEBTDF(ptr) Free((char *)ptr, sizeof(BTDF))
#else
static POOL *btdfPool = (POOL *)NULL;
#define NEWBTDF()	(BTDF *)NewPoolCell(sizeof(BTDF), 0, "btdfs", &btdfPool)
#define DISPOSEBTDF(ptr) Dispose((char *)ptr, &btdfPool)
#endif


BTDF *BtdfCreate(void *data, BTDF_METHODS *methods)
{
  BTDF *btdf;

  btdf = NEWBTDF();
  btdf->data = data;
  btdf->methods = methods;

  return btdf;
}


BTDF *BtdfDuplicate(BTDF *obtdf)
{
  BTDF *btdf;

  if (!obtdf)
    return obtdf;

  btdf = NEWBTDF();
  btdf->data = obtdf->methods->Duplicate(obtdf->data);
  btdf->methods = obtdf->methods;

  return btdf;
}


void *BtdfCreateEditor(void *parent, BTDF *btdf)
{
  if (!btdf)
    Fatal(-1, "BtdfCreateEditor", "NULL btdf pointer passed.");
  return btdf->methods->CreateEditor(parent, btdf->data);
}


void BtdfDestroy(BTDF *btdf)
{
  if (!btdf) return;
  btdf->methods->Destroy(btdf->data);
  DISPOSEBTDF(btdf);
}


COLOR BtdfTransmittance(BTDF *btdf, XXDFFLAGS flags)
{
  if (btdf && btdf->methods->Transmittance)
    return btdf->methods->Transmittance(btdf->data, flags);
  else {
    static COLOR refl;
    COLORCLEAR(refl);
    return refl;
  }
}


void BtdfIndexOfRefraction(BTDF *btdf, REFRACTIONINDEX *index)
{
  if (btdf && btdf->methods->IndexOfRefraction)
  {
    btdf->methods->IndexOfRefraction(btdf->data, index);
  }
  else
  {
    index->nr = 1.0;
    index->ni = 0.0; 
  }
}




COLOR BtdfEval(BTDF *btdf, REFRACTIONINDEX inIndex, REFRACTIONINDEX outIndex, VECTOR *in, VECTOR *out, VECTOR *normal, XXDFFLAGS flags)
{
  if (btdf && btdf->methods->Eval)
    return btdf->methods->Eval(btdf->data, inIndex, outIndex, in, out, normal, flags);
  else {
    static COLOR refl;
    COLORCLEAR(refl);
    return refl;
  }
}

VECTOR BtdfSample(BTDF *btdf, REFRACTIONINDEX inIndex, 
		  REFRACTIONINDEX outIndex, VECTOR *in, 
		  VECTOR *normal, int doRussianRoulette,
		  XXDFFLAGS flags, double x_1, double x_2, 
		  double *pdf)
{
  if (btdf && btdf->methods->Sample)
    return btdf->methods->Sample(btdf->data, inIndex, outIndex, in, normal, 
				 doRussianRoulette, flags, x_1, x_2, pdf);
  else {
    VECTOR dummy = {0., 0., 0.};
    *pdf = 0;
    return dummy;
  }
}

void BtdfEvalPdf(BTDF *btdf, REFRACTIONINDEX inIndex, 
		 REFRACTIONINDEX outIndex, VECTOR *in, 
		 VECTOR *out, VECTOR *normal,
		 XXDFFLAGS flags, double *pdf, double *pdfRR)
{
  if (btdf && btdf->methods->EvalPdf)
    btdf->methods->EvalPdf(btdf->data, inIndex, outIndex, in, out, 
			   normal, flags, pdf, pdfRR);
  else {
    *pdf = 0;
  }
}


void BtdfPrint(FILE *out, BTDF *btdf)
{
  if (!btdf)
    fprintf(out, "(NULL BTDF)\n");
  else
    btdf->methods->Print(out, btdf->data);
}
