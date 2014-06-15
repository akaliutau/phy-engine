
#include <math.h>

#include "error.h"
#include "spherical.h"
#include "edf.h"
#include "raytools.H"
#include "lightdirsampler.H"

bool CLightDirSampler::Sample(CPathNode *, CPathNode *thisNode,
			      CPathNode *newNode, double x_1, double x_2,
			      bool , BSDFFLAGS )
{
  double pdfDir; //, cosPatch, cosLight, dist2;

  if(!thisNode->m_hit.material->edf)
  {
    Error("CLightDirSampler::Sample", "No EDF");
    return false;
  }

  // Sample a light direction
  VECTOR dir = EdfSample(thisNode->m_hit.material->edf,
			 &thisNode->m_hit, DIFFUSE_COMPONENT,
			 x_1, x_2, &thisNode->m_bsdfEval,
			 &pdfDir);

  if(pdfDir < EPSILON) return false; // Zero pdf event, no valid sample

  // Determine ray type
  thisNode->m_rayType = Starts;
  newNode->m_inBsdf = thisNode->m_outBsdf; // Light can be placed in a medium  

  // Transfer
  if(!SampleTransfer(thisNode, newNode, &dir, pdfDir))
  {
    thisNode->m_rayType = Stops;
    return false;
  }

  // thisNode->m_bsdfEval = EdfEval(thisNode->m_hit.material->edf,
  // &thisNode->m_hit,
  // &(newNode->m_inDirT),
  // DIFFUSE_COMPONENT, (double *)0);

  
  thisNode->m_bsdfComp.Clear();
  thisNode->m_bsdfComp.Fill(thisNode->m_bsdfEval, BRDF_DIFFUSE_COMPONENT);

  // Component propagation
  thisNode->m_usedComponents = NO_COMPONENTS; // the light...
  newNode->m_accUsedComponents = (thisNode->m_accUsedComponents | 
				  thisNode->m_usedComponents);

  newNode->m_rracc = thisNode->m_rracc;

  return true;
}

double CLightDirSampler::EvalPDF(CPathNode *thisNode, CPathNode *newNode,
				 BSDFFLAGS , double* , 
				 double* )
{
  double pdfDir, cosa, dist, dist2;
  VECTOR outDir;

  if(!thisNode->m_hit.material->edf)
  {
    Error("CLightDirSampler::EvalPDF", "No EDF");
    return false;
  }
  

  VECTORSUBTRACT(newNode->m_hit.point, thisNode->m_hit.point, outDir);
  dist2 = VECTORNORM2(outDir);
  dist = sqrt(dist2);
  VECTORSCALEINVERSE(dist, outDir, outDir);

  // EDF sampling
  EdfEval(thisNode->m_hit.material->edf,
	  &thisNode->m_hit, &outDir, DIFFUSE_COMPONENT, &pdfDir);

  if(pdfDir < 0.)
    return 0.0;  // Back face of a light does not radiate !

  cosa = -VECTORDOTPRODUCT(outDir, newNode->m_normal);

  return pdfDir * cosa / dist2;  
}


