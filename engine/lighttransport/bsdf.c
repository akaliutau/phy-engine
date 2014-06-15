

#include "bsdf.h"
#include "pools.h"
#include "vector.h"
#include "error.h"

#ifdef NOPOOLS
#define NEWBSDF()	(BSDF *)Alloc(sizeof(BSDF))
#define DISPOSEBSDF(ptr) Free((char *)ptr, sizeof(BSDF))
#else
static POOL *bsdfPool = (POOL *)NULL;
#define NEWBSDF()	(BSDF *)NewPoolCell(sizeof(BSDF), 0, "bsdfs", &bsdfPool)
#define DISPOSEBSDF(ptr) Dispose((char *)ptr, &bsdfPool)
#endif


BSDF *BsdfCreate(void *data, BSDF_METHODS *methods)
{
  BSDF *bsdf;

  //  bsdf = NEWBSDF();
  bsdf = (BSDF*)malloc(sizeof(BSDF));
  bsdf->data = data;
  bsdf->methods = methods;

  return bsdf;
}


BSDF *BsdfDuplicate(BSDF *obsdf)
{
  BSDF *bsdf;

  if (!obsdf)
    return obsdf;

  bsdf = NEWBSDF();
  bsdf->data = obsdf->methods->Duplicate(obsdf->data);
  bsdf->methods = obsdf->methods;

  return bsdf;
}


void *BsdfCreateEditor(void *parent, BSDF *bsdf)
{
  if (!bsdf)
    Fatal(-1, "BsdfCreateEditor", "NULL bsdf pointer passed.");
  return bsdf->methods->CreateEditor(parent, bsdf->data);
}


void BsdfDestroy(BSDF *bsdf)
{
  if (!bsdf) return;
  bsdf->methods->Destroy(bsdf->data);
  DISPOSEBSDF(bsdf);
}


COLOR BsdfScatteredPower(BSDF *bsdf, HITREC *hit, VECTOR *in, BSDFFLAGS flags)
{
  COLOR refl;
  COLORCLEAR(refl);
  if (bsdf && bsdf->methods->ScatteredPower)
    refl = bsdf->methods->ScatteredPower(bsdf->data, hit, in, flags);
  return refl;
}

int BsdfIsTextured(BSDF *bsdf)
{
  if (bsdf && bsdf->methods->IsTextured)
    return bsdf->methods->IsTextured(bsdf->data);
  return FALSE;
}

int BsdfShadingFrame(BSDF *bsdf, HITREC *hit, VECTOR *X, VECTOR *Y, VECTOR *Z)
{
  if (bsdf && bsdf->methods->ShadingFrame) {
    return bsdf->methods->ShadingFrame(bsdf->data, hit, X, Y, Z);
  }
  return FALSE;
}

void BsdfIndexOfRefraction(BSDF *bsdf, REFRACTIONINDEX *index)
{
  if (bsdf && bsdf->methods->IndexOfRefraction)
  {
    bsdf->methods->IndexOfRefraction(bsdf->data, index);
  }
  else
  {
    index->nr = 1.0;
    index->ni = 0.0; 
  }
}




COLOR BsdfEval(BSDF *bsdf, HITREC *hit, BSDF *inBsdf, BSDF *outBsdf, VECTOR *in, VECTOR *out, BSDFFLAGS flags)
{
  if (bsdf && bsdf->methods->Eval)
    return bsdf->methods->Eval(bsdf->data, hit, inBsdf, outBsdf, in, out, flags);
  else {
    static COLOR refl;
    COLORCLEAR(refl);
    return refl;
  }
}





extern COLOR BsdfEvalComponents(BSDF *bsdf, HITREC *hit, BSDF *inBsdf, 
				BSDF *outBsdf, VECTOR *in, VECTOR *out, 
				BSDFFLAGS flags,
				COLOR *colArray)
{
  

  
 
  COLOR result, empty;
  int i;
  BSDFFLAGS thisFlag;
  
  COLORCLEAR(empty);
  COLORCLEAR(result);
  
  for(i = 0; i < BSDFCOMPONENTS; i++)
  {
    thisFlag = BSDF_INDEXTOCOMP(i);

    if(flags & thisFlag)
    {
      colArray[i] = BsdfEval(bsdf, hit, inBsdf, outBsdf, in, out, thisFlag);
      COLORADD(result, colArray[i], result);
    }
    else
    {
      colArray[i] = empty;  
    }
  }
  
  return result;
}



VECTOR BsdfSample(BSDF *bsdf, HITREC *hit,
		  BSDF *inBsdf, BSDF *outBsdf,
		  VECTOR *in,
		  int doRussianRoulette, BSDFFLAGS flags,
		  double x_1, double x_2,
		  double *pdf)
{
  if (bsdf && bsdf->methods->Sample)
    return bsdf->methods->Sample(bsdf->data, hit, inBsdf, outBsdf, in,
				 doRussianRoulette, flags, x_1, x_2, pdf);
  else {
    VECTOR dummy = {0., 0., 0.};
    *pdf = 0;
    return dummy;
  }
}



void BsdfEvalPdf(BSDF *bsdf, HITREC *hit, 
		 BSDF *inBsdf, BSDF *outBsdf, 
		 VECTOR *in, VECTOR *out,
		 BSDFFLAGS flags,
		 double *pdf, double *pdfRR)
{
  if (bsdf && bsdf->methods->EvalPdf)
    bsdf->methods->EvalPdf(bsdf->data, hit, inBsdf, outBsdf, in, out, 
			   flags, pdf, pdfRR);
  else {
    *pdf = 0;
  }
}




void BsdfPrint(FILE *out, BSDF *bsdf)
{
  if (!bsdf)
    fprintf(out, "(NULL BSDF)\n");
  else
    bsdf->methods->Print(out, bsdf->data);
}

