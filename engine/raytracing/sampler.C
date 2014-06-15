

#include "error.h"
#include "sampler.H"
#include "raytools.H"
#include "hit.h"
#include "scene.h"

bool CSampler::SampleTransfer(CPathNode *thisNode,
			      CPathNode *newNode, VECTOR *dir, 
			      double pdfDir)
{
  RAY ray;
  HITREC *hit;

  ray.pos = thisNode->m_hit.point;
  ray.dir = *dir;

  // Fill in depth

  newNode->m_depth = thisNode->m_depth + 1;
  newNode->m_rayType = Stops;

  hit = FindRayIntersection(&ray, thisNode->m_hit.patch,
			    newNode->m_inBsdf, &newNode->m_hit);

  if(!hit)
  {
    if(Background)
      {
	// Fill in pathnode for background
	InitHit(&(newNode->m_hit), Background->bkgPatch, NULL, NULL, dir, NULL, HUGE);
	newNode->m_inDirT = *dir;
	newNode->m_inDirF = -(*dir);
	newNode->m_pdfFromPrev = pdfDir;
	newNode->m_G = fabs(VECTORDOTPRODUCT(thisNode->m_hit.normal, newNode->m_inDirT));
	newNode->m_inBsdf = thisNode->m_outBsdf;
	newNode->m_useBsdf = NULL;
	newNode->m_outBsdf = NULL;
	newNode->m_rayType = Environment;
	newNode->m_hit.flags |= HIT_FRONT;

	return true;
      }

    // if no background is present
    else return(false);
  }

  if(hit->flags & HIT_BACK)
  {
    // Back hit, invert normal (only happens when newNode->m_inBsdf != NULL
    VECTORSCALE(-1, newNode->m_hit.normal, newNode->m_hit.normal);
  }

  VECTORCOPY(ray.dir, newNode->m_inDirT);
  VECTORSCALE(-1, newNode->m_inDirT, newNode->m_inDirF);
  
  // Check for shading normal vs. geometric normal errors
      
  if(VECTORDOTPRODUCT(newNode->m_hit.normal, newNode->m_inDirF) < 0)
  {
    // Error("Sample", "Shading normal anomaly");
    return false;
  }
      

  // Compute geometry factor cos(a)*cos(b)/r^2 

  double cosa, cosb, dist2;
  VECTOR tmpVec;
      
  cosa = fabs(VECTORDOTPRODUCT(thisNode->m_hit.normal, 
			       newNode->m_inDirT));
  cosb = fabs(VECTORDOTPRODUCT(newNode->m_hit.normal, 
			       newNode->m_inDirT));
  VECTORSUBTRACT(newNode->m_hit.point, thisNode->m_hit.point, tmpVec);
  dist2 = VECTORNORM2(tmpVec);

  if(dist2 < EPSILON)
  {
    
    return false;
  }
	
  newNode->m_G = cosa * cosb / dist2; // Integrate over area !

  // Fill in probability

  newNode->m_pdfFromPrev = pdfDir * cosb / dist2;

  return true; // Transfer succeeded
}



////////// Surface sampler

void CSurfaceSampler::DetermineRayType(CPathNode *thisNode, 
				       CPathNode *newNode, VECTOR *dir)
{
  double cosThisPatch = VECTORDOTPRODUCT(*dir, thisNode->m_normal);

  if(cosThisPatch < 0)
  {
    // Refraction !
    if(thisNode->m_hit.flags & HIT_BACK)
      thisNode->m_rayType = Leaves;
    else
      thisNode->m_rayType = Enters;
    
    newNode->m_inBsdf = thisNode->m_outBsdf;
  } 
  else 
  {
    // Reflection
    thisNode->m_rayType = Reflects;
    newNode->m_inBsdf = thisNode->m_inBsdf; // staying in same medium
  }
}
