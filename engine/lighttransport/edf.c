

#include "edf.h"
#include "pools.h"
#include "error.h"

#ifdef NOPOOLS
#define NEWEDF()	(EDF *)Alloc(sizeof(EDF))
#define DISPOSEEDF(ptr) Free((char *)ptr, sizeof(EDF))
#else
static POOL *edfPool = (POOL *)NULL;
#define NEWEDF()	(EDF *)NewPoolCell(sizeof(EDF), 0, "edfs", &edfPool)
#define DISPOSEEDF(ptr) Dispose((char *)ptr, &edfPool)
#endif


EDF *EdfCreate(void *data, EDF_METHODS *methods)
{
  EDF *edf;

  edf = NEWEDF();
  edf->data = data;
  edf->methods = methods;

  return edf;
}


EDF *EdfDuplicate(EDF *oedf)
{
  EDF *edf;

  if (!oedf)
    return oedf;

  edf = NEWEDF();
  edf->data = oedf->methods->Duplicate(oedf->data);
  edf->methods = oedf->methods;

  return edf;
}


void *EdfCreateEditor(void *parent, EDF *edf)
{
  if (!edf)
    Fatal(-1, "EdfCreateEditor", "NULL edf pointer passed.");
  return edf->methods->CreateEditor(parent, edf->data);
}


void EdfDestroy(EDF *edf)
{
  if (!edf) return;
  edf->methods->Destroy(edf->data);
  DISPOSEEDF(edf);
}


COLOR EdfEmittance(EDF *edf, HITREC *hit, XXDFFLAGS flags)
{
  if (edf && edf->methods->Emittance)
    return edf->methods->Emittance(edf->data, hit, flags);
  else {
    static COLOR emit;
    COLORCLEAR(emit);
    return emit;
  }
}

int EdfIsTextured(EDF *edf)
{
  if (edf && edf->methods->IsTextured)
    return edf->methods->IsTextured(edf->data);
  return FALSE;
}


COLOR EdfDiffuseRadiance(EDF *edf, HITREC *hit)
{
  COLOR rad = EdfDiffuseEmittance(edf, hit);
  COLORSCALE((1./M_PI), rad, rad);
  return rad;
}


COLOR EdfEval(EDF *edf, HITREC *hit, VECTOR *out, XXDFFLAGS flags, double *pdf)
{
  if (edf && edf->methods->Eval)
    return edf->methods->Eval(edf->data, hit, out, flags, pdf);
  else {
    static COLOR val;
    COLORCLEAR(val);
    if (pdf) *pdf = 0.;
    return val;
  }
}

VECTOR EdfSample(EDF *edf, HITREC *hit, XXDFFLAGS flags,
		 double xi1, double xi2,
		 COLOR *emitted_radiance, double *pdf)
{
  if (edf && edf->methods->Sample) 
    return edf->methods->Sample(edf->data, hit, flags, xi1, xi2, emitted_radiance, pdf);
  else {
    VECTOR v = {0.,0.,0.};
    Fatal(-1, "EdfSample", "Can't sample EDF");
    return v;
  }
}


int EdfShadingFrame(EDF *edf, HITREC *hit, VECTOR *X, VECTOR *Y, VECTOR *Z)
{
  if (edf && edf->methods->ShadingFrame) {
    return edf->methods->ShadingFrame(edf->data, hit, X, Y, Z);
  }
  return FALSE;  
}


void EdfPrint(FILE *out, EDF *edf)
{
  if (!edf)
    fprintf(out, "(NULL EDF)\n");
  else
    edf->methods->Print(out, edf->data);
}
