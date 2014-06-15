

#include <math.h>
#include "phong.h"
#include "Float.h"
#include "ui_phong.h"
#include "pools.h"
#include "spherical.h"
#include "error.h"



#ifdef NOPOOLS
#define NEWPHONGEDF()	(PHONG_EDF *)Alloc(sizeof(PHONG_EDF))
#define NEWPHONGBRDF()	(PHONG_BRDF *)Alloc(sizeof(PHONG_BRDF))
#define NEWPHONGBTDF()	(PHONG_BTDF *)Alloc(sizeof(PHONG_BTDF))

#define DISPOSEPHONGEDF(ptr) Free((char *)ptr, sizeof(PHONG_EDF))
#define DISPOSEPHONGBRDF(ptr) Free((char *)ptr, sizeof(PHONG_BRDF))
#define DISPOSEPHONGBTDF(ptr) Free((char *)ptr, sizeof(PHONG_BTDF))
#else
static POOL *phongEdfPool = (POOL *)NULL;
static POOL *phongBrdfPool = (POOL *)NULL;
static POOL *phongBtdfPool = (POOL *)NULL;

#define NEWPHONGEDF()	(PHONG_EDF *)NewPoolCell(sizeof(PHONG_EDF), 0, "phong edfs", &phongEdfPool)
#define NEWPHONGBRDF()	(PHONG_BRDF *)NewPoolCell(sizeof(PHONG_BRDF), 0, "phong brdfs", &phongBrdfPool)
#define NEWPHONGBTDF()	(PHONG_BTDF *)NewPoolCell(sizeof(PHONG_BTDF), 0, "phong btdfs", &phongBtdfPool)

#define DISPOSEPHONGEDF(ptr) Dispose((char *)ptr, &phongEdfPool)
#define DISPOSEPHONGBRDF(ptr) Dispose((char *)ptr, &phongBrdfPool)
#define DISPOSEPHONGBTDF(ptr) Dispose((char *)ptr, &phongBtdfPool)
#endif

static void PhongBrdfPrint(FILE *out, PHONG_BRDF *brdf);


PHONG_EDF *PhongEdfCreate(COLOR *Kd, COLOR *Ks, double Ns)
{
  PHONG_EDF *edf = NEWPHONGEDF();
  edf->Kd = *Kd;
  COLORSCALE((1./M_PI), edf->Kd, edf->kd);	
  edf->Ks = *Ks;
  if (!COLORNULL(edf->Ks)) {
    Warning("PhongEdfCreate", "Non-diffuse light sources not yet inplemented");
  }
  edf->Ns = Ns;
  return edf;
}

PHONG_EDF *PhongEdfDuplicate(PHONG_EDF *edf)
{
  PHONG_EDF *new = NEWPHONGEDF();
  *new = *edf;
  return new;
}

PHONG_BRDF *PhongBrdfCreate(COLOR *Kd, COLOR *Ks, double Ns)
{
  PHONG_BRDF *brdf = NEWPHONGBRDF();
  brdf->Kd = *Kd;
  brdf->avgKd = COLORAVERAGE(brdf->Kd);
  brdf->Ks = *Ks;
  brdf->avgKs = COLORAVERAGE(brdf->Ks);
  brdf->Ns = Ns;
  return brdf;
}

PHONG_BRDF *PhongBrdfDuplicate(PHONG_BRDF *brdf)
{
  PHONG_BRDF *new = NEWPHONGBRDF();
  *new = *brdf;
  return new;
}

PHONG_BTDF *PhongBtdfCreate(COLOR *Kd, COLOR *Ks, double Ns, double nr, double ni)
{
  PHONG_BTDF *btdf = NEWPHONGBTDF();
  btdf->Kd = *Kd;
  btdf->avgKd = COLORAVERAGE(btdf->Kd);
  btdf->Ks = *Ks;
  btdf->avgKs = COLORAVERAGE(btdf->Ks);
  btdf->Ns = Ns;
  btdf->refrIndex.nr = nr;
  btdf->refrIndex.ni = ni;
  return btdf;
}

PHONG_BTDF *PhongBtdfDuplicate(PHONG_BTDF *btdf)
{
  PHONG_BTDF *new = NEWPHONGBTDF();
  *new = *btdf;
  return new;
}


static void PhongEdfDestroy(PHONG_EDF *edf)
{
  DISPOSEPHONGEDF(edf);
}

static void PhongBrdfDestroy(PHONG_BRDF *brdf)
{
  DISPOSEPHONGBRDF(brdf);
}

static void PhongBtdfDestroy(PHONG_BTDF *btdf)
{
  DISPOSEPHONGBTDF(btdf);
}


static void PhongEdfPrint(FILE *out, PHONG_EDF *edf)
{
  fprintf(out, "Phong Edf: Kd = "); 
  ColorPrint(out, edf->Kd);
  fprintf(out, ", Ks = ");
  ColorPrint(out, edf->Ks);
  fprintf(out, ", Ns = %g\n", edf->Ns);
}

static void PhongBrdfPrint(FILE *out, PHONG_BRDF *brdf)
{
  fprintf(out, "Phong Brdf: Kd = "); 
  ColorPrint(out, brdf->Kd);
  fprintf(out, ", Ks = ");
  ColorPrint(out, brdf->Ks);
  fprintf(out, ", Ns = %g\n", brdf->Ns);
}

static void PhongBtdfPrint(FILE *out, PHONG_BTDF *btdf)
{
  fprintf(out, "Phong Btdf: Kd = "); 
  ColorPrint(out, btdf->Kd);
  fprintf(out, ", Ks = ");
  ColorPrint(out, btdf->Ks);
  fprintf(out, ", Ns = %g, nr=%g, ni=%g\n", btdf->Ns, btdf->refrIndex.nr, btdf->refrIndex.ni);
}


static COLOR PhongEmittance(PHONG_EDF *edf, HITREC *hit, XXDFFLAGS flags)
{
  COLOR result;

  COLORCLEAR(result);

  if(flags & DIFFUSE_COMPONENT)
  {
    COLORADD(result, edf->Kd, result);
  }

  if(PHONG_IS_SPECULAR(*edf))
  {
    if(flags & SPECULAR_COMPONENT)
    {
      COLORADD(result, edf->Ks, result);
    }
  }
  else
  {
    if(flags & GLOSSY_COMPONENT)
    {
      COLORADD(result, edf->Ks, result);
    }
  }

  return result;
}

static COLOR PhongAverageEmittance(PHONG_EDF *edf, XXDFFLAGS flags)
{
  return PhongEmittance(edf, NULL, flags);
}

static COLOR PhongReflectance(PHONG_BRDF *brdf, XXDFFLAGS flags)
{
  COLOR result;

  COLORCLEAR(result);

  if(flags & DIFFUSE_COMPONENT)
  {
    COLORADD(result, brdf->Kd, result);
  }

  if(PHONG_IS_SPECULAR(*brdf))
  {
    if(flags & SPECULAR_COMPONENT)
    {
      COLORADD(result, brdf->Ks, result);
    }
  }
  else
  {
    if(flags & GLOSSY_COMPONENT)
    {
      COLORADD(result, brdf->Ks, result);
    }
  }

  return result;
}

static COLOR PhongTransmittance(PHONG_BTDF *btdf, XXDFFLAGS flags)
{
  COLOR result;

  COLORCLEAR(result);

  if(flags & DIFFUSE_COMPONENT)
  {
    COLORADD(result, btdf->Kd, result);
  }

  if(PHONG_IS_SPECULAR(*btdf))
  {
    if(flags & SPECULAR_COMPONENT)
    {
      COLORADD(result, btdf->Ks, result);
    }
  }
  else
  {
    if(flags & GLOSSY_COMPONENT)
    {
      COLORADD(result, btdf->Ks, result);
    }
  }

  if (! finite(COLORAVERAGE(result))) {
    Fatal(-1, "PhongTransmittance", "Oops - result is not finite!");
  }

  return result;
}



void PhongIndexOfRefraction(PHONG_BTDF *btdf, REFRACTIONINDEX *index)
{
  index->nr = btdf->refrIndex.nr;
  index->ni = btdf->refrIndex.ni;
}




static COLOR PhongEdfEval(PHONG_EDF *edf, HITREC *hit, VECTOR *out, XXDFFLAGS flags, double *pdf)
{
  VECTOR normal;
  COLOR result;
  double cosl;

  COLORCLEAR(result);
  if (pdf) *pdf = 0.;

  if (!HitShadingNormal(hit, &normal)) {
    Warning("PhongEdfEval", "Couldn't determine shading normal");
    return result;
  }

  cosl = VECTORDOTPRODUCT(*out, normal);

  if(cosl < 0.0)
    return result; 

  

  if(flags & DIFFUSE_COMPONENT)
  {
    
    COLORADD(result, edf->kd, result);
    if (pdf) *pdf = cosl / M_PI;
  }

  if(flags & SPECULAR_COMPONENT)
  {
    
  }

  return result;
}


static VECTOR PhongEdfSample(PHONG_EDF *edf, HITREC *hit, XXDFFLAGS flags,
			     double xi1, double xi2,
			     COLOR *selfemitted_radiance, double *pdf)
{
  VECTOR dir = {0.,0.,1.};
  if (selfemitted_radiance) COLORCLEAR(*selfemitted_radiance);
  if (pdf) *pdf = 0.;

  if (flags & DIFFUSE_COMPONENT) {
    double spdf;
    COORDSYS coord;

    VECTOR normal;
    if (!HitShadingNormal(hit, &normal)) {
      Warning("PhongEdfEval", "Couldn't determine shading normal");
      return dir;
    }

    VectorCoordSys(&normal, &coord);
    dir = SampleHemisphereCosTheta(&coord, xi1, xi2, &spdf);
    if (pdf) *pdf = spdf;
    if (selfemitted_radiance) {
      COLORSCALE((1./M_PI), edf->Kd, *selfemitted_radiance);
    }
  }

  return dir;
}



static COLOR PhongBrdfEval(PHONG_BRDF *brdf, VECTOR *in, VECTOR *out, VECTOR *normal, XXDFFLAGS flags)
{
  COLOR result;
  float tmpFloat, dotProduct;
  VECTOR idealReflected;
  XXDFFLAGS nondiffuseFlag;
  VECTOR inrev; VECTORSCALE(-1., *in, inrev);

  COLORCLEAR(result);

  

  if(VECTORDOTPRODUCT(*out, *normal) < 0)
  {
    
    return result;
  }

  if((flags & DIFFUSE_COMPONENT) && (brdf->avgKd > 0.0))
  {
    COLORADDSCALED(result, M_1_PI, brdf->Kd, result);
  }

  if(PHONG_IS_SPECULAR(*brdf))
    nondiffuseFlag = SPECULAR_COMPONENT;
  else
    nondiffuseFlag = GLOSSY_COMPONENT;


  if((flags & nondiffuseFlag) && (brdf->avgKs > 0.0))
  {
    idealReflected = IdealReflectedDirection(&inrev, normal);
    dotProduct = VECTORDOTPRODUCT(idealReflected,*out);

    if(dotProduct > 0)
    {
      tmpFloat = (float)pow(dotProduct, brdf->Ns); 
      tmpFloat *= ((int)brdf->Ns + 2.0) / (2 * M_PI); 
      COLORADDSCALED(result, tmpFloat, brdf->Ks, result);
    }
  }

  return result;
}




static VECTOR PhongBrdfSample(PHONG_BRDF *brdf, VECTOR *in, 
			      VECTOR *normal, int doRussianRoulette,
			      XXDFFLAGS flags, double x_1, double x_2, 
			      double *pdf)
{
  VECTOR newDir = {0., 0., 0.}, idealDir;
  double cos_theta, diffPdf, nonDiffPdf;
  double scatteredPower, avgKd, avgKs;
  float tmpFloat;
  COORDSYS coord;
  XXDFFLAGS nondiffuseFlag;
  VECTOR inrev; VECTORSCALE(-1., *in, inrev);

  *pdf = 0;

  if(flags & DIFFUSE_COMPONENT)
  {
    avgKd = brdf->avgKd;
  }
  else
  {
    avgKd = 0.0;
  }

  if(PHONG_IS_SPECULAR(*brdf))
    nondiffuseFlag = SPECULAR_COMPONENT;
  else
    nondiffuseFlag = GLOSSY_COMPONENT;


  if(flags & nondiffuseFlag)
  {
    avgKs = brdf->avgKs;
  }
  else
  {
    avgKs = 0.0;
  }

  scatteredPower = avgKd + avgKs;

  if(scatteredPower < EPSILON)
  {
    return newDir;
  }

  

  if(doRussianRoulette)
  {
    if(x_1 > scatteredPower)
    {
      
      return newDir;
    }

    
    x_1 /= scatteredPower;
  }
    
  idealDir = IdealReflectedDirection(&inrev, normal);

  if(x_1 < (avgKd / scatteredPower))
  {
    
    x_1 = x_1 / (avgKd / scatteredPower);

    VectorCoordSys(normal, &coord);
    newDir = SampleHemisphereCosTheta(&coord, x_1, x_2, &diffPdf);
    
    
    tmpFloat = VECTORDOTPRODUCT(idealDir, newDir);

    if(tmpFloat > 0)
    {
      nonDiffPdf = (brdf->Ns + 1.0) * pow(tmpFloat, 
					  brdf->Ns) / (2.0 * M_PI);
    }
    else
    {
      nonDiffPdf = 0;
    }
  }
  else
  {
    
    x_1 = (x_1 - (avgKd/scatteredPower)) / (avgKs/scatteredPower);

    VectorCoordSys(&idealDir, &coord);
    newDir = SampleHemisphereCosNTheta(&coord, brdf->Ns, x_1, x_2, 
				       &nonDiffPdf);

    cos_theta = VECTORDOTPRODUCT(*normal, newDir);
    if(cos_theta <= 0)
    {
      return newDir;
    }

    
    diffPdf = cos_theta / M_PI;
  }

  

  *pdf = avgKd * diffPdf + avgKs * nonDiffPdf;

  if(!doRussianRoulette)
  {
    *pdf /= scatteredPower;
  }

  return newDir;
}



static void PhongBrdfEvalPdf(PHONG_BRDF *brdf, VECTOR *in, 
			     VECTOR *out, VECTOR *normal, 
			     XXDFFLAGS flags, double *pdf, double *pdfRR)
{
  double cos_theta, cos_alpha, cos_in;
  double diffPdf, nonDiffPdf, scatteredPower;
  double avgKs, avgKd;
  XXDFFLAGS nondiffuseFlag;
  VECTOR idealDir;
  VECTOR inrev; 
  VECTOR goodNormal;

  VECTORSCALE(-1., *in, inrev);

  *pdf = 0;
  *pdfRR = 0;

  

  cos_in = VECTORDOTPRODUCT(*in, *normal);
  if(cos_in >= 0)
  {
    VECTORCOPY(*normal, goodNormal);
  }
  else
  {
    VECTORSCALE(-1, *normal, goodNormal);
  }

  cos_theta = VECTORDOTPRODUCT(goodNormal, *out);
  
  if(cos_theta < 0)
  {
    return;
  }

  

  if(flags & DIFFUSE_COMPONENT)
  {
    avgKd = brdf->avgKd;  
  }
  else
  {
    avgKd = 0.0;
  }
  
  if(PHONG_IS_SPECULAR(*brdf))
    nondiffuseFlag = SPECULAR_COMPONENT;
  else
    nondiffuseFlag = GLOSSY_COMPONENT;
  
  
  if(flags & nondiffuseFlag)
  {
    avgKs = brdf->avgKs;
  }
  else
  {
    avgKs = 0.0;
  }
  
  scatteredPower = avgKd + avgKs;
  
  if(scatteredPower < EPSILON)
  {
    return;
  }

  

  diffPdf = 0.0;

  if(avgKd > 0)
  {
    

    diffPdf = cos_theta / M_PI;
  }

  

  nonDiffPdf = 0.0;

  if(avgKs > 0)
  {
    idealDir = IdealReflectedDirection(&inrev, &goodNormal);

    cos_alpha = VECTORDOTPRODUCT(idealDir, *out);

    if(cos_alpha > 0)
    {
      nonDiffPdf = (brdf->Ns + 1.0) * pow(cos_alpha,
					  brdf->Ns) / (2.0 * M_PI);
    }
  }

  *pdf = (avgKd * diffPdf + avgKs * nonDiffPdf) / scatteredPower;
  *pdfRR = scatteredPower;

  return;
}




static COLOR PhongBtdfEval(PHONG_BTDF *btdf, REFRACTIONINDEX inIndex, REFRACTIONINDEX outIndex, VECTOR *in, VECTOR *out, VECTOR *normal, XXDFFLAGS flags)
{
  COLOR result;
  float tmpFloat, dotProduct;
  VECTOR idealrefracted;
  int totalIR;
  int IsReflection;
  XXDFFLAGS nondiffuseFlag;
  VECTOR inrev; VECTORSCALE(-1., *in, inrev);

  

  

  COLORCLEAR(result);

  if((flags & DIFFUSE_COMPONENT) && (btdf->avgKd > 0))
  {
    

    

    IsReflection = (VECTORDOTPRODUCT(*normal, *out) >= 0);

    if(!IsReflection)
    {
      result = btdf->Kd;
      COLORSCALE(M_1_PI, result, result);
    }
  }

  if(PHONG_IS_SPECULAR(*btdf))
    nondiffuseFlag = SPECULAR_COMPONENT;
  else
    nondiffuseFlag = GLOSSY_COMPONENT;


  if((flags & nondiffuseFlag) && (btdf->avgKs > 0))
  {
    

    idealrefracted = IdealRefractedDirection(&inrev, normal, inIndex, 
					     outIndex, &totalIR);

    dotProduct = VECTORDOTPRODUCT(idealrefracted,*out);
  
    if(dotProduct > 0)
    {
      tmpFloat = (float)pow(dotProduct, btdf->Ns); 
      tmpFloat *= (btdf->Ns + 2.0) / (2 * M_PI); 
      COLORADDSCALED(result, tmpFloat, btdf->Ks, result);
    }
  }

  return result;
}


static VECTOR PhongBtdfSample(PHONG_BTDF *btdf, REFRACTIONINDEX inIndex, 
			      REFRACTIONINDEX outIndex, VECTOR *in, 
			      VECTOR *normal, int doRussianRoulette,
			      XXDFFLAGS flags, double x_1, double x_2, 
			      double *pdf)
{
  VECTOR newDir = {0., 0., 0.};
  int totalIR;
  VECTOR idealDir, invNormal;
  COORDSYS coord;
  double cos_theta;
  double avgKd, avgKs, scatteredPower;
  double diffPdf, nonDiffPdf;
  float tmpFloat;
  XXDFFLAGS nondiffuseFlag;
  VECTOR inrev; VECTORSCALE(-1., *in, inrev);

  *pdf = 0;

  

  if(flags & DIFFUSE_COMPONENT)
  {
    avgKd = btdf->avgKd;  
  }
  else
  {
    avgKd = 0.0;
  }

  if(PHONG_IS_SPECULAR(*btdf))
    nondiffuseFlag = SPECULAR_COMPONENT;
  else
    nondiffuseFlag = GLOSSY_COMPONENT;


  if(flags & nondiffuseFlag)
  {
    avgKs = btdf->avgKs;
  }
  else
  {
    avgKs = 0.0;
  }

  scatteredPower = avgKd + avgKs;

  if(scatteredPower < EPSILON)
  {
    return newDir;
  }

  

  if(doRussianRoulette)
  {
    if(x_1 > scatteredPower)
    {
      
      return newDir;
    }

    
    x_1 /= scatteredPower;
  }
    
  idealDir = IdealRefractedDirection(&inrev, normal, inIndex, outIndex,
				     &totalIR);
  VECTORSCALE(-1, *normal, invNormal);

  if(x_1 < (avgKd / scatteredPower))
  {
    
    x_1 = x_1 / (avgKd / scatteredPower);

    VectorCoordSys(&invNormal, &coord);

    newDir = SampleHemisphereCosTheta(&coord, x_1, x_2, &diffPdf);
    
    
    tmpFloat = VECTORDOTPRODUCT(idealDir, newDir);

    if(tmpFloat > 0)
    {
      nonDiffPdf = (btdf->Ns + 1.0) * pow(tmpFloat, 
					  btdf->Ns) / (2.0 * M_PI);
    }
    else
    {
      nonDiffPdf = 0;
    }
  }
  else
  {
    
    x_1 = (x_1 - (avgKd/scatteredPower)) / (avgKs/scatteredPower);

    VectorCoordSys(&idealDir, &coord);
    newDir = SampleHemisphereCosNTheta(&coord, btdf->Ns, x_1, x_2, 
				       &nonDiffPdf);

    cos_theta = VECTORDOTPRODUCT(*normal, newDir);
    if(cos_theta > 0)
    {
      
      diffPdf = cos_theta / M_PI;
    }
    else
    {
      

      diffPdf = 0.0;
    }
  }
  
  

  *pdf = avgKd * diffPdf + avgKs * nonDiffPdf;

  if(!doRussianRoulette)
  {
    *pdf /= scatteredPower;
  }


  return newDir;
}

static void PhongBtdfEvalPdf(PHONG_BTDF *btdf, REFRACTIONINDEX inIndex, 
			     REFRACTIONINDEX outIndex, VECTOR *in, 
			     VECTOR *out, VECTOR *normal, 
			     XXDFFLAGS flags, 
			     double *pdf, double *pdfRR)
{
  double cos_theta, cos_alpha, cos_in;
  double diffPdf, nonDiffPdf=0., scatteredPower;
  double avgKs, avgKd;
  XXDFFLAGS nondiffuseFlag;
  VECTOR idealDir;
  int totalIR;
  VECTOR goodNormal;
  VECTOR inrev; VECTORSCALE(-1., *in, inrev);

  *pdf = 0;
  *pdfRR = 0;

  

  cos_in = VECTORDOTPRODUCT(*in, *normal);
  if(cos_in >= 0)
  {
    VECTORCOPY(*normal, goodNormal);
  }
  else
  {
    VECTORSCALE(-1, *normal, goodNormal);
  }

  cos_theta = VECTORDOTPRODUCT(goodNormal, *out);
  
  if(flags & DIFFUSE_COMPONENT && (cos_theta < 0))  
  {
    avgKd = btdf->avgKd;
  }
  else
  {
    avgKd = 0.0;
  }
  
  if(PHONG_IS_SPECULAR(*btdf))
    nondiffuseFlag = SPECULAR_COMPONENT;
  else
    nondiffuseFlag = GLOSSY_COMPONENT;
  
  
  if(flags & nondiffuseFlag)
  {
    avgKs = btdf->avgKs;
  }
  else
  {
    avgKs = 0.0;
  }
  
  scatteredPower = avgKd + avgKs;
  
  if(scatteredPower < EPSILON)
  {
    return;
  }

  

  if(avgKd > 0)
  {
    
    diffPdf = cos_theta / M_PI;
  }
  else
  {
    diffPdf = 0.0;
  }

  

  if(avgKs > 0)
  {
    if(cos_in >= 0)
    {
      idealDir = IdealRefractedDirection(&inrev, &goodNormal, inIndex, 
					 outIndex, &totalIR);
    }
    else
    {
      
      idealDir = IdealRefractedDirection(&inrev, &goodNormal, outIndex, 
					 inIndex, &totalIR);
    }      

    cos_alpha = VECTORDOTPRODUCT(idealDir, *out);

    nonDiffPdf = 0.0;
    if(cos_alpha > 0)
    {
      nonDiffPdf = (btdf->Ns + 1.0) * pow(cos_alpha,
					  btdf->Ns) / (2.0 * M_PI);
    }
  }

  *pdf = (avgKd * diffPdf + avgKs * nonDiffPdf) / scatteredPower;
  *pdfRR = scatteredPower; 

  return;
}




EDF_METHODS PhongEdfMethods = {
  (COLOR (*)(void *, HITREC *, XXDFFLAGS))PhongEmittance,
  (int (*)(void *))NULL,	
  (COLOR (*)(void *, HITREC *, VECTOR *, XXDFFLAGS, double *))PhongEdfEval,
  (VECTOR (*)(void *, HITREC *, XXDFFLAGS, double, double, COLOR *, double *))PhongEdfSample,
  (int (*)(void *, HITREC *, VECTOR *, VECTOR *, VECTOR *))NULL,
  (void (*)(FILE *, void *))PhongEdfPrint,
  (void *(*)(void *))PhongEdfDuplicate,
  (void *(*)(void *, void *))CreatePhongEdfEditor,
  (void (*)(void *))PhongEdfDestroy
};

BRDF_METHODS PhongBrdfMethods = {
  (COLOR (*)(void *, XXDFFLAGS))PhongReflectance,
  (COLOR (*)(void *, VECTOR *, VECTOR *, VECTOR *, XXDFFLAGS))PhongBrdfEval,
  (VECTOR (*)(void *, VECTOR *, VECTOR *, int, XXDFFLAGS, double, double, 
		    double *))PhongBrdfSample, 
  (void (*)(void *, VECTOR *, VECTOR *, VECTOR *,
	    XXDFFLAGS, double *, double *))PhongBrdfEvalPdf,
  (void (*)(FILE *, void *))PhongBrdfPrint,
  (void *(*)(void *))PhongBrdfDuplicate,
  (void *(*)(void *, void *))CreatePhongBrdfEditor,
  (void (*)(void *))PhongBrdfDestroy
};

BTDF_METHODS PhongBtdfMethods = {
  (COLOR (*)(void *, XXDFFLAGS))PhongTransmittance,
  (void (*)())PhongIndexOfRefraction,
  (COLOR (*)(void *, REFRACTIONINDEX, REFRACTIONINDEX, VECTOR *, VECTOR *, VECTOR *, XXDFFLAGS))PhongBtdfEval,
  (VECTOR (*)(void *, REFRACTIONINDEX , REFRACTIONINDEX, VECTOR *, 
		   VECTOR *, int, XXDFFLAGS, double, double, 
		   double *))PhongBtdfSample,
  (void (*)(void *, REFRACTIONINDEX, REFRACTIONINDEX, VECTOR *, 
	    VECTOR *, VECTOR *, XXDFFLAGS, double *, double *))PhongBtdfEvalPdf,
  (void (*)(FILE *, void *))PhongBtdfPrint,
  (void *(*)(void *))PhongBtdfDuplicate,
  (void *(*)(void *, void *))CreatePhongBtdfEditor,
  (void (*)(void *))PhongBtdfDestroy
};

