

#include "error.h"
#include "xxdf.h"
#include "vector.h"



float ComplexToGeometricRefractionIndex(REFRACTIONINDEX nc)
{
  float sqrtF, f1,f2;

  f1 = (nc.nr - 1.0); 
  f1 = f1*f1 + nc.ni * nc.ni;

  f2 = (nc.nr + 1.0); 
  f2 = f2*f2 + nc.ni * nc.ni;

  sqrtF = sqrt(f1 / f2);

  return (1.0 + sqrtF) / (1.0 - sqrtF);
}


VECTOR IdealReflectedDirection(VECTOR *in, VECTOR *normal)
{
  double tmp;
  VECTOR result;

  tmp = 2 * VECTORDOTPRODUCT(*normal, *in);
  VECTORSCALE(tmp, *normal, result);
  VECTORSUBTRACT(*in, result, result);
  VECTORNORMALISE(result);

  return result;
}

DVECTOR DIdealReflectedDirection(DVECTOR *in, DVECTOR *normal)
{
  double tmp;
  DVECTOR result;

  tmp = 2.0 * VECTORDOTPRODUCT(*normal, *in);
  VECTORSCALE(tmp, *normal, result);
  VECTORSUBTRACT(*in, result, result);
  VECTORNORMALISE(result);

  return result;
}





VECTOR IdealRefractedDirection(VECTOR *in, VECTOR *normal, 
				   REFRACTIONINDEX inIndex, 
				   REFRACTIONINDEX outIndex, 
				   int *totalInternalReflection)
{
  double Ci,Ct, Ct2;
  double refractionIndex;
  
  double normalScale;
  VECTOR result;

  

  refractionIndex = inIndex.nr / outIndex.nr;

  Ci = - VECTORDOTPRODUCT(*in, *normal);
  Ct2 = 1 + refractionIndex * refractionIndex * ( Ci * Ci - 1);

  if(Ct2 < 0)
  {
    *totalInternalReflection = TRUE;
    return(IdealReflectedDirection(in, normal));
  }
  *totalInternalReflection = FALSE;

  Ct = sqrt(Ct2);

  normalScale = refractionIndex * Ci - Ct;

  VECTORSCALE(refractionIndex, *in, result);
  VECTORADDSCALED(result, normalScale, *normal, result);

  
  VECTORNORMALISE(result);

  return result;
}


DVECTOR DIdealRefractedDirection(DVECTOR *in, DVECTOR *normal, 
				 REFRACTIONINDEX inIndex, 
				 REFRACTIONINDEX outIndex, 
				 int *totalInternalReflection)
{
  double Ci,Ct, Ct2;
  double refractionIndex;
  
  double normalScale;
  DVECTOR result;

  
  refractionIndex = inIndex.nr / outIndex.nr;

  Ci = - VECTORDOTPRODUCT(*in, *normal);
  Ct2 = 1 + refractionIndex * refractionIndex * ( Ci * Ci - 1);

  if(Ct2 < 0)
  {
    *totalInternalReflection = TRUE;
    return(DIdealReflectedDirection(in, normal));
  }
  *totalInternalReflection = FALSE;

  Ct = sqrt(Ct2);

  normalScale = refractionIndex * Ci - Ct;

  VECTORSCALE(refractionIndex, *in, result);
  VECTORADDSCALED(result, normalScale, *normal, result);

  
  VECTORNORMALISE(result);

  return result;
}

