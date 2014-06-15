

#include <math.h>

#include "eyesampler.H"
#include "error.h"
#include "camera.h"
#include "scene.h"

bool CEyeSampler::Sample(CPathNode *prevNode, CPathNode *thisNode,
			 CPathNode *newNode, double , double ,
			 bool , BSDFFLAGS )
{
  if(prevNode != NULL || thisNode != NULL)
    Warning("CEyeSampler::Sample", "Not first node in path ?!");

  // Just fill in newNode with camera data. Appropiate pdf fields are
  // are set to 1
  
  newNode->m_depth = 0;  // We expect this to be the first node in an eye path
  newNode->m_rayType = Stops;

  // Choose eye : N/A
  // Choose point on eye : N/A

  // Fake a hit record

  HITREC *hit = &newNode->m_hit;

  InitHit(hit, NULL, NULL, &Camera.eyep, &Camera.Z, NULL, 0.);
  hit->normal = Camera.Z; 
  hit->X = Camera.X;
  hit->Y = Camera.Y;
  hit->Z = Camera.Z;
  hit->flags |= HIT_NORMAL|HIT_SHADINGFRAME;

  VECTORCOPY(newNode->m_hit.normal, newNode->m_normal);
  newNode->m_G = 1.0;
  
  // outDir's not filled in

  newNode->m_pdfFromPrev = 1.0; 

  newNode->m_pdfFromNext = 0.0; 

  newNode->m_useBsdf = NULL;
  newNode->m_inBsdf = NULL;
  newNode->m_outBsdf = NULL;

  // Component propagation
  newNode->m_accUsedComponents = NO_COMPONENTS; // Eye had no accumulated comps.

  newNode->m_rracc = 1.0; // No russian roulette

  return true;
}

double CEyeSampler::EvalPDF(CPathNode *, 
			    CPathNode *, BSDFFLAGS ,
			    double* , double* )
{
  return 1.0;
}

