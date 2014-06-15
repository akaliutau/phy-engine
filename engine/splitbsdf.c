

#include "bsdf.h"
#include "pools.h"
#include "vector.h"
#include "error.h"
#include "splitbsdf.h"
#include "spherical.h"

#ifdef NOPOOLS
#define NEWSPLITBSDF()	(SPLIT_BSDF *)Alloc(sizeof(SPLIT_BSDF))
#define DISPOSESPLITBSDF(ptr) Free((char *)ptr, sizeof(SPLIT_BSDF))
#else
static POOL *bsdfPool = (POOL *)NULL;
#define NEWSPLITBSDF()	(SPLIT_BSDF *)NewPoolCell(sizeof(BSDF), 0, "split bsdfs", &bsdfPool)
#define DISPOSESPLITBSDF(ptr) Dispose((char *)ptr, &bsdfPool)
#endif



void SplitBsdfPrint(FILE *out, SPLIT_BSDF *bsdf)
{
  fprintf(out, "Split Bsdf :\n  ");
  BrdfPrint(out, bsdf->brdf);
  fprintf(out, "  ");
  BtdfPrint(out, bsdf->btdf);
  fprintf(out, "  ");
  PrintTexture(out, bsdf->texture);
}

SPLIT_BSDF *SplitBSDFCreate(BRDF *brdf, BTDF *btdf, TEXTURE *texture)
{
  SPLIT_BSDF *bsdf = NEWSPLITBSDF();

  bsdf->brdf = brdf;
  bsdf->btdf = btdf;
  bsdf->texture = texture;

  return bsdf;
}

static SPLIT_BSDF *SplitBsdfDuplicate(SPLIT_BSDF *bsdf)
{
  SPLIT_BSDF *newBsdf = NEWSPLITBSDF();
  *newBsdf = *bsdf;
  return newBsdf;
}

static void SplitBsdfDestroy(SPLIT_BSDF *bsdf)
{
  DISPOSESPLITBSDF(bsdf);
}

static COLOR SplitBsdfEvalTexture(TEXTURE* texture, HITREC *hit)
{
  VECTOR texCoord;
  COLOR col;
  COLORCLEAR(col);

  if (!texture)
    return col;

  if (!hit || !HitTexCoord(hit, &texCoord)) {
    Warning("SplitBsdfEvalTexture", "Couldn't get texture coordinates");
    return col;
  }

  return EvalTextureColor(texture, texCoord.x, texCoord.y);
}


#define TEXTURED_COMPONENT BRDF_DIFFUSE_COMPONENT

static double TexturedScattererEval(VECTOR *in, VECTOR *out, VECTOR *normal)
{
  return (1. / M_PI);
}



static VECTOR TexturedScattererSample(VECTOR *in, VECTOR *normal, double x_1, double x_2, double*pdf)
{
  COORDSYS coord;
  VectorCoordSys(normal, &coord);
  return SampleHemisphereCosTheta(&coord, x_1, x_2, pdf);
}

static void TexturedScattererEvalPdf(VECTOR *in, VECTOR* out, VECTOR* normal, double *pdf)
{
  *pdf = VECTORDOTPRODUCT(*normal, *out) / M_PI;
}

static COLOR SplitBsdfScatteredPower(SPLIT_BSDF *bsdf, HITREC *hit, VECTOR *in, BSDFFLAGS flags)
{
  COLOR albedo;
  COLORCLEAR(albedo);

  if (bsdf->texture && (flags & TEXTURED_COMPONENT)) {
    COLOR textureColor = SplitBsdfEvalTexture(bsdf->texture, hit);
    COLORADD(albedo, textureColor, albedo);
    flags &= ~TEXTURED_COMPONENT;  
  }

  if (bsdf->brdf) {
    COLOR refl = BrdfReflectance(bsdf->brdf, GETBRDFFLAGS(flags));
    COLORADD(albedo, refl, albedo);
  }

  if (bsdf->btdf) {
    COLOR trans = BtdfTransmittance(bsdf->btdf, GETBTDFFLAGS(flags));
    COLORADD(albedo, trans, albedo);
  }

  return albedo;
}

static void SplitBsdfIndexOfRefraction(SPLIT_BSDF *bsdf, REFRACTIONINDEX *index)
{
  BtdfIndexOfRefraction(bsdf->btdf, index);
}

static COLOR SplitBsdfEval(SPLIT_BSDF *bsdf, HITREC *hit,
			   BSDF *inBsdf, BSDF *outBsdf, 
			   VECTOR *in, VECTOR *out,
			   BSDFFLAGS flags)
{
  COLOR result;
  VECTOR normal;

  COLORCLEAR(result);
  if (!HitShadingNormal(hit, &normal)) {
    Warning("SplitBsdfEval", "Couldn't determine shading normal");
    return result;
  }

  if (bsdf->texture && (flags & TEXTURED_COMPONENT)) {
    double textureBsdf = TexturedScattererEval(in, out, &normal);
    COLOR textureCol =  SplitBsdfEvalTexture(bsdf->texture, hit);
    COLORADDSCALED(result, textureBsdf, textureCol, result);
    flags &= ~TEXTURED_COMPONENT;
  }

  
  if (bsdf->brdf) {
    COLOR reflectionCol = BrdfEval(bsdf->brdf, in, out, &normal, 
				   GETBRDFFLAGS(flags));
    COLORADD(result, reflectionCol, result);
  }

  if (bsdf->btdf) {
    REFRACTIONINDEX inIndex, outIndex;
    COLOR refractionCol;
    BsdfIndexOfRefraction(inBsdf, &inIndex);
    BsdfIndexOfRefraction(outBsdf, &outIndex);
    refractionCol = BtdfEval(bsdf->btdf, inIndex, outIndex,
			     in, out, &normal,  GETBTDFFLAGS(flags));
    COLORADD(result, refractionCol, result);
  }

  return result;
}




static void SplitBsdfProbabilities(SPLIT_BSDF *bsdf, HITREC *hit, BSDFFLAGS flags,
				   double *Ptexture,
				   double *Preflection,
				   double *Ptransmission,
				   XXDFFLAGS *brdfFlags, XXDFFLAGS *btdfFlags)
{
  COLOR textureColor, reflectance, transmittance;

  *Ptexture = 0.;
  if (bsdf->texture && (flags & TEXTURED_COMPONENT)) {
    
    textureColor = SplitBsdfEvalTexture(bsdf->texture, hit);
    *Ptexture = COLORAVERAGE(textureColor);
    flags &= ~TEXTURED_COMPONENT;
  }

  *brdfFlags = GETBRDFFLAGS(flags);
  *btdfFlags = GETBTDFFLAGS(flags);

  reflectance = BrdfReflectance(bsdf->brdf, *brdfFlags);
  *Preflection = COLORAVERAGE(reflectance);

  transmittance = BtdfTransmittance(bsdf->btdf, *btdfFlags);
  *Ptransmission = COLORAVERAGE(transmittance);
}


typedef enum SAMPLING_MODE {SAMPLE_TEXTURE, SAMPLE_REFLECTION, SAMPLE_TRANSMISSION, SAMPLE_ABSORPTION} SAMPLING_MODE;

static SAMPLING_MODE SplitBsdfSamplingMode(double Ptexture, double Preflection, double Ptransmission, double *x_1)
{
  SAMPLING_MODE mode = SAMPLE_ABSORPTION;
  if (*x_1 < Ptexture) {
    mode = SAMPLE_TEXTURE;
    *x_1 /= Ptexture;     
  } else {
    *x_1 -= Ptexture;
    if (*x_1 < Preflection) {
      mode = SAMPLE_REFLECTION;
      *x_1 /= Preflection;
    } else {
      *x_1 -= Preflection;
      if (*x_1 < Ptransmission) {
	mode = SAMPLE_TRANSMISSION;
	*x_1 /= Ptransmission;
      }
    }
  }
  return mode;
}

static VECTOR SplitBsdfSample(SPLIT_BSDF *bsdf, HITREC *hit,
			      BSDF *inBsdf, BSDF *outBsdf,
			      VECTOR *in,
			      int doRussianRoulette, 
			      BSDFFLAGS flags,
			      double x_1, double x_2,
			      double *pdf)
{
  double Ptexture, Preflection, Ptransmission, Pscattering, p, pRR;
  REFRACTIONINDEX inIndex, outIndex;
  XXDFFLAGS brdfFlags, btdfFlags;
  VECTOR normal;
  SAMPLING_MODE mode;
  VECTOR out;

  *pdf = 0; 
  if (!HitShadingNormal(hit, &normal)) {
    Warning("SplitBsdfSample", "Couldn't determine shading normal");
    VECTORSET(out, 0., 0., 1.);
    return out;
  }

  
  SplitBsdfProbabilities(bsdf, hit, flags, 
			 &Ptexture, &Preflection, &Ptransmission,
			 &brdfFlags, &btdfFlags);
  Pscattering = Ptexture + Preflection + Ptransmission;
  if (Pscattering < EPSILON)
    return out;

  
  if (!doRussianRoulette) {
    
    Ptexture /= Pscattering;
    Preflection /= Pscattering;
    Ptransmission /= Pscattering;
  }
  mode = SplitBsdfSamplingMode(Ptexture, Preflection, Ptransmission, &x_1);

  BsdfIndexOfRefraction(inBsdf, &inIndex);
  BsdfIndexOfRefraction(outBsdf, &outIndex);

  
  switch (mode) {
  case SAMPLE_TEXTURE:
    out = TexturedScattererSample(in, &normal, x_1, x_2, &p);
    if (p < EPSILON) return out; 
    *pdf = Ptexture * p; 
    break;
  case SAMPLE_REFLECTION:
    out = BrdfSample(bsdf->brdf, in, &normal, FALSE, brdfFlags, x_1, x_2, &p);
    if (p < EPSILON) return out;
    *pdf = Preflection * p;
    break;
  case SAMPLE_TRANSMISSION:
    out = BtdfSample(bsdf->btdf, inIndex, outIndex, in, &normal, 
		     FALSE, btdfFlags, x_1, x_2, &p);
    if (p < EPSILON) return out;
    *pdf = Ptransmission * p;
    break;
  case SAMPLE_ABSORPTION:
    *pdf = 0;
    return out;
    break;
  default:
    Fatal(-1, "SplitBsdfSample", "Impossible sampling mode %d", mode);
  }

  
  if (mode != SAMPLE_TEXTURE) {
    TexturedScattererEvalPdf(in, &out, &normal, &p);
    *pdf += Ptexture * p;
  }
  if (mode != SAMPLE_REFLECTION) {
    BrdfEvalPdf(bsdf->brdf, in, &out, &normal,
		brdfFlags, &p, &pRR);
    *pdf += Preflection * p;
  }
  if (mode != SAMPLE_TRANSMISSION) {
    BtdfEvalPdf(bsdf->btdf, inIndex, outIndex, in, &out, &normal,
		btdfFlags, &p, &pRR);
    *pdf += Ptransmission * p;
  }

  return out;
}






static void SplitBsdfEvalPdf(SPLIT_BSDF *bsdf, HITREC *hit,
			     BSDF *inBsdf, BSDF *outBsdf, 
			     VECTOR *in, VECTOR *out,
			     BSDFFLAGS flags,
			     double *pdf, double *pdfRR)
{
  double Ptexture, Preflection, Ptransmission, Pscattering, p, pRR;
  REFRACTIONINDEX inIndex, outIndex;
  XXDFFLAGS brdfFlags, btdfFlags;
  VECTOR normal;

  *pdf = *pdfRR = 0.; 
  if (!HitShadingNormal(hit, &normal)) {
    Warning("SplitBsdfEvalPdf", "Couldn't determine shading normal");
    return;
  }

  
  SplitBsdfProbabilities(bsdf, hit, flags, 
			 &Ptexture, &Preflection, &Ptransmission,
			 &brdfFlags, &btdfFlags);
  Pscattering = Ptexture + Preflection + Ptransmission;
  if (Pscattering < EPSILON)
    return;

  
  *pdfRR = Pscattering;

  
  BsdfIndexOfRefraction(inBsdf, &inIndex);
  BsdfIndexOfRefraction(outBsdf, &outIndex);

  TexturedScattererEvalPdf(in, out, &normal, &p);
  *pdf = Ptexture * p;

  BrdfEvalPdf(bsdf->brdf, in, out, &normal,
	      brdfFlags, &p, &pRR);
  *pdf += Preflection * p;

  BtdfEvalPdf(bsdf->btdf, inIndex, outIndex, in, out, &normal,
	      btdfFlags, &p, &pRR);
  *pdf += Ptransmission * p;

  *pdf /= Pscattering;
}

static int SplitBsdfIsTextured(SPLIT_BSDF *bsdf)
{
  return bsdf->texture != NULL;
}

BSDF_METHODS SplitBsdfMethods = {
  (COLOR (*)(void *data, HITREC *hit, VECTOR *in, BSDFFLAGS flags))SplitBsdfScatteredPower,
  (int (*)(void *))SplitBsdfIsTextured,
  (void (*)(void *data, REFRACTIONINDEX *index))SplitBsdfIndexOfRefraction,
  (COLOR (*)(void *data, HITREC *hit, void *inBsdf, void *outBsdf, VECTOR *in, VECTOR *out, BSDFFLAGS flags))SplitBsdfEval,
  (VECTOR (*)(void *data, HITREC *hit, void *inBsdf, void *outBsdf, VECTOR *in, 
		   int doRussianRoulette,
		   BSDFFLAGS flags, double x_1, double x_2,
		   double *pdf))SplitBsdfSample,
  (void (*)(void *data, HITREC *hit, void *inBsdf, void *outBsdf, 
	    VECTOR *in, VECTOR *out, 
	    BSDFFLAGS flags,
	    double *pdf, double *pdfRR))SplitBsdfEvalPdf,
  (int (*)(void *data, HITREC *hit, VECTOR *X, VECTOR *Y, VECTOR *Z))NULL,
  (void (*)(FILE *out, void *data))SplitBsdfPrint,
  (void *(*)(void *data))SplitBsdfDuplicate,
  (void *(*)(void *parent, void *data))NULL,
  (void (*)(void *data))SplitBsdfDestroy
};
