
#include <math.h>

#include "lightsampler.H"
#include "error.h"
#include "patch.h"

CUniformLightSampler::CUniformLightSampler(void)
{
  // if(gLightList)
  // iterator = new CLightList_Iter(*gLightList);
  iterator = NULL;
  currentPatch = NULL;
  unitsActive = false;
}

bool CUniformLightSampler::ActivateFirstUnit()
{
  if(!iterator)
  {
    if(gLightList)
      iterator = new CLightList_Iter(*gLightList);
    else
      return false;
  }
    
  currentPatch = iterator->First(*gLightList);

  if(currentPatch != NULL)
    {
      unitsActive = true;
      return true;
    }
  else return false;
}

bool CUniformLightSampler::ActivateNextUnit()
{
  currentPatch = iterator->Next();
  return(currentPatch != NULL);
}

void CUniformLightSampler::DeactivateUnits()
{
  unitsActive = false;
  currentPatch = NULL;
}

bool CUniformLightSampler::Sample(CPathNode *, 
				  CPathNode *thisNode,
				  CPathNode *newNode, double x_1, double x_2,
				  bool , BSDFFLAGS flags)
{
  double pdfLight, pdfPoint;
  PATCH *light;
  POINT point;

  

  newNode->m_depth = 0;
  newNode->m_rayType = Stops;

  newNode->m_useBsdf = NULL;
  newNode->m_inBsdf = NULL;
  newNode->m_outBsdf = NULL;
  newNode->m_G = 1.0;

  // Choose light

  if(unitsActive)
  {
    if(currentPatch)
    {
      light = currentPatch;
      pdfLight = 1.0;
    }
    else
    {
      Warning("Sample Unit Light Node", "No valid light selected");
      return false;
    }
  }
  else
  {
    light = gLightList->Sample(&x_1, &pdfLight);

    if(light == NULL)
    {
      Warning("FillLightNode", "No light found");
      return false;
    }
  }

  // Choose point (uniform for real, sampled for background)
  if(IsPatchVirtual(light))
    {
      double pdf;
      VECTOR dir = EdfSample(light->surface->material->edf, &(thisNode->m_hit), flags, x_1, x_2, NULL, &pdf);
      VECTORDIFF(thisNode->m_hit.point, dir, point);   // fake hit at distance 1!

      InitHit(&newNode->m_hit, light, NULL, &point, NULL, 
	      light->surface->material, 0.);

      // fill in directions
      VECTORSCALE(-1, dir, newNode->m_inDirT);
      VECTORCOPY(dir, newNode->m_inDirF);
      VECTORCOPY(dir, newNode->m_normal);

      pdfPoint = pdf;   // every direction corresponds to 1 point
    }
  else
    {
      PatchUniformPoint(light, x_1, x_2, &point);
      pdfPoint = 1.0 / light->area;

      // Fake a hit record

      InitHit(&newNode->m_hit, light, NULL, &point, &light->normal, 
	      light->surface->material, 0.);
      HitShadingNormal(&newNode->m_hit, &newNode->m_hit.normal);
      VECTORCOPY(newNode->m_hit.normal, newNode->m_normal);
    }
  
  // inDir's not filled in
  newNode->m_pdfFromPrev = pdfLight * pdfPoint;

  // Component propagation
  newNode->m_accUsedComponents = NO_COMPONENTS; // Light has no accumulated comps.

  newNode->m_rracc = 1.0;

  return true;
}

double CUniformLightSampler::EvalPDF(CPathNode *, 
				     CPathNode *newNode, 
				     BSDFFLAGS , double* , 
				     double* )
{
  double pdf, pdfdir;

  // The light point is in NEW NODE !! 

  if(unitsActive)
  {
    pdf = 1.0;
  }
  else
  {
    pdf = gLightList->EvalPDF(newNode->m_hit.patch, &newNode->m_hit.point);
  }

  // Prob for choosing this point(/direction)
  if(IsPatchVirtual(newNode->m_hit.patch))          // virtual patch
    {
      // virtual patch has no area!
      // choosing a point == choosing a dir --> use pdf from evalEdf
      EdfEval(newNode->m_hit.patch->surface->material->edf,
	      (HITREC *)NULL,
	      &newNode->m_inDirF,
	      DIFFUSE_COMPONENT || GLOSSY_COMPONENT || SPECULAR_COMPONENT,
	      &pdfdir);

      pdf *= pdfdir;
    }
  else {                                            // normal patch
    // choosing point uniformly
    if(pdf >= EPSILON && newNode->m_hit.patch->area > EPSILON)
      pdf = pdf / newNode->m_hit.patch->area;
    else pdf = 0.0;

  }
  
  return pdf;  
}




bool CImportantLightSampler::Sample(CPathNode *, 
				    CPathNode *thisNode,
				    CPathNode *newNode, double x_1, double x_2,
				    bool , BSDFFLAGS flags )
{
  double pdfLight, pdfPoint;
  PATCH *light;
  POINT point;

  

  newNode->m_depth = 0;
  newNode->m_rayType = Stops;

  newNode->m_useBsdf = NULL;
  newNode->m_inBsdf = NULL;
  newNode->m_outBsdf = NULL;
  newNode->m_G = 1.0;

  // Choose light

  if(thisNode->m_hit.flags & HIT_BACK)
  {
    if(thisNode->m_outBsdf == NULL)
    {
      VECTOR invNormal;

      VECTORSCALE(-1, thisNode->m_normal, invNormal);

      light = gLightList->SampleImportant(&thisNode->m_hit.point,
					  &invNormal, &x_1, &pdfLight);
    }
    else
    {
      // No (important) light sampling inside a material
      light = NULL;
    }
  }
  else
  {
    if(thisNode->m_inBsdf == NULL)
    {
      light = gLightList->SampleImportant(&thisNode->m_hit.point,
					  &thisNode->m_normal, 
					  &x_1, &pdfLight);
    }
    else
    {
      light = NULL;
    }
  }

  if(light == NULL)
  {
    // Warning("FillLightNode", "No light found");
    return false;
  }

  // Choose point (uniform for real, sampled for background)
  if(IsPatchVirtual(light))
    {
      double pdf;
      VECTOR dir = EdfSample(light->surface->material->edf, NULL, flags, x_1, x_2, NULL, &pdf);
      VECTORADD(thisNode->m_hit.point, dir, point);   // fake hit at distance 1!

      InitHit(&newNode->m_hit, light, NULL, &point, NULL, 
	      light->surface->material, 0.);

      // fill in directions
      VECTORSCALE(-1, dir, newNode->m_inDirT);
      VECTORCOPY(dir, newNode->m_inDirF);
      VECTORCOPY(dir, newNode->m_normal);

      pdfPoint = pdf;   // every direction corresponds to 1 point
    }
  else
    {
      PatchUniformPoint(light, x_1, x_2, &point);

      pdfPoint = 1.0 / light->area;

      // Light position and value are known now

      // Fake a hit record

      InitHit(&newNode->m_hit, light, NULL, &point, &light->normal, 
	      light->surface->material, 0.);
      HitShadingNormal(&newNode->m_hit, &newNode->m_hit.normal);
      VECTORCOPY(newNode->m_hit.normal, newNode->m_normal);
    }
  
  // outDir's, m_G not filled in yet (lightdirection sampler does this)

  newNode->m_pdfFromPrev = pdfLight * pdfPoint;

  return true;
}

double CImportantLightSampler::EvalPDF(CPathNode *thisNode,
				       CPathNode *newNode,
				       BSDFFLAGS , double* , 
				       double* )
{
  double pdf, pdfdir;

  // The light point is in NEW NODE !! 
  pdf = gLightList->EvalPDFImportant(newNode->m_hit.patch, 
				     &newNode->m_hit.point,
				     &thisNode->m_hit.point,
				     &thisNode->m_normal);

  // Prob for choosing this point(/direction)
  if(IsPatchVirtual(newNode->m_hit.patch))           // virtual patch
    {
      // virtual patch has no area!
      // choosing a point == choosing a dir --> use pdf from evalEdf
      EdfEval(newNode->m_hit.patch->surface->material->edf,
	      (HITREC *)NULL,
	      &newNode->m_inDirF,
	      DIFFUSE_COMPONENT || GLOSSY_COMPONENT || SPECULAR_COMPONENT,
	      &pdfdir);

      pdf *= pdfdir;
    }
  else {                                            // normal patch
    // choosing point uniformly
    if(pdf >= EPSILON && newNode->m_hit.patch->area > EPSILON)
      pdf = pdf / newNode->m_hit.patch->area;
    else pdf = 0.0;

  }

  return pdf;  
}
