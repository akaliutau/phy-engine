








#include "background_edf.H"


static COLOR BackgroundEdfEmittance(void *bkg, HITREC *hit , XXDFFLAGS )
{
  return(BackgroundPower((BACKGROUND *)bkg, hit ? &(hit->point) : NULL));
}

static int BackgroundEdfIsTextured(void *)
{
  return TRUE;
}


static COLOR BackgroundEdfEval(void *bkg, HITREC *hit, VECTOR *out, XXDFFLAGS , double *pdf)
{
  float fpdf;
  COLOR result = BackgroundRadiance((BACKGROUND *)bkg, hit ? &(hit->point) : NULL, out, &fpdf);
  if(pdf) *pdf = fpdf;

  return(result);
}


static VECTOR BackgroundEdfSample(void *bkg, HITREC *hit, XXDFFLAGS , double xi1, double xi2, COLOR *emitted_radiance, double *pdf)
{
  float fpdf;

  VECTOR result = BackgroundSample((BACKGROUND *)bkg, hit ? &(hit->point) : NULL, xi1, xi2, emitted_radiance, &fpdf);
  if(pdf) *pdf = fpdf;

  return(result);
}


static int BackgroundEdfShadingFrame(void *, HITREC *, VECTOR *X, VECTOR *Y, VECTOR *Z)
{
  VECTORSET(*X, 1, 0, 0);
  VECTORSET(*Y, 0, 1, 0);
  VECTORSET(*Z, 0, 0, 1);
  return(TRUE);
}


static void BackgroundEdfPrint(FILE *, void *)
{
  // do nothing
}


static void *BackgroundEdfDuplicate(void *data)
{
  return(data);
}


static void *BackgroundEdfCreateEditor(void *parent, void *data)
{
  // do nothing
  return NULL;
}


static void BackgroundEdfDestroy(void *)
{
  // do nothing
}


static struct EDF_METHODS BackgroundEdfMethods = {
  (COLOR (*)(void *bkg, HITREC *hit, XXDFFLAGS flags))BackgroundEdfEmittance,
  (int (*)(void *bkg))BackgroundEdfIsTextured,
  (COLOR (*)(void *bkg, HITREC *hit, VECTOR *out, XXDFFLAGS flags, double *pdf))BackgroundEdfEval,
  (VECTOR (*)(void *bkg, HITREC *hit, XXDFFLAGS flags, double xi1, double xi2, COLOR *emitted_radiance, double *pdf))BackgroundEdfSample,
  (int (*)(void *bkg, HITREC *hit, VECTOR *X, VECTOR *Y, VECTOR *Z))BackgroundEdfShadingFrame,
  (void (*)(FILE *out, void *bkg))BackgroundEdfPrint,
  (void *(*)(void *bkg))BackgroundEdfDuplicate,
  (void *(*)(void *parent, void *bkg))BackgroundEdfCreateEditor,
  (void (*)(void *bkg))BackgroundEdfDestroy,
};

EDF *BackgroundEdfCreate(BACKGROUND *bkg)
{     
  return(EdfCreate(bkg, &BackgroundEdfMethods));
}
