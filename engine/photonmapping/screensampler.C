

#include <math.h>

#include "screensampler.H"
#include "raytools.H"
#include "scene.h"
#include "camera.h"

bool CScreenSampler::Sample(CPathNode *, 
			    CPathNode *thisNode,
			    CPathNode *newNode, double x_1, double x_2,
			    bool , BSDFFLAGS )
{
  VECTOR dir;

  // Precond: thisNode == eye, prevNode == NULL, SetPixel called

  // Sample direction
  double xsample = (Camera.pixh * Camera.hres * (-0.5 + x_1));
  double ysample = (Camera.pixv * Camera.vres * (-0.5 + x_2));

  VECTORCOMB3(Camera.Z, xsample, Camera.X, ysample, Camera.Y, dir);
  double distScreen2 = VECTORNORM2(dir);
  double distScreen = sqrt(distScreen2);
  VECTORSCALEINVERSE(distScreen, dir, dir);

  double cosScreen = fabs(VECTORDOTPRODUCT(Camera.Z, dir));

  double pdfDir = ((1. / (Camera.pixh * Camera.hres * 
			  Camera.pixv * Camera.vres)) * // 1 / Area pixel
		   (distScreen2 / cosScreen));  // spher. angle measure 

  // Determine ray type
  thisNode->m_rayType = Starts;
  newNode->m_inBsdf = thisNode->m_outBsdf; // Camera can be placed in a medium

  // Transfer
  if(!SampleTransfer(thisNode, newNode, &dir, pdfDir))
  {
    thisNode->m_rayType = Stops;
    return false;
  }

  // "Bsdf" in thisNode

  // Potential is one for all directions through a pixel
  COLORSETMONOCHROME(thisNode->m_bsdfEval, 1.0);  

  // Make sure evaluation of eye components always includes the diff ref.
  thisNode->m_bsdfComp.Clear();
  thisNode->m_bsdfComp.Fill(thisNode->m_bsdfEval, BRDF_DIFFUSE_COMPONENT);

  // Component propagation
  thisNode->m_usedComponents = NO_COMPONENTS; // the eye...
  newNode->m_accUsedComponents = (thisNode->m_accUsedComponents | 
				  thisNode->m_usedComponents);
  return true;
}

double CScreenSampler::EvalPDF(CPathNode *thisNode, CPathNode *newNode,
			       BSDFFLAGS , double* , 
			       double* )
{
  double dist2, dist, cosa, cosb, pdf;
  VECTOR outDir;

  

  VECTORSUBTRACT(newNode->m_hit.point, thisNode->m_hit.point, outDir);
  dist2 = VECTORNORM2(outDir);
  dist = sqrt(dist2);
  VECTORSCALEINVERSE(dist, outDir, outDir);

  // pdf = 1 / A_screen transformed to area measure
  
  cosa = VECTORDOTPRODUCT(thisNode->m_normal, outDir);

  // pdf = 1/Apix * (r^2 / cos(dir, eyeNormal) * (cos(dir, patchNormal) / d^2)
  //                 |__> to sper.angle           |__> to area on patch
  
  // Three cosines : r^2 / cos = 1 / cos^3 since r is length
  // of viewing ray to the screen.
  pdf = 1.0 / (Camera.pixv * Camera.vres * Camera.pixh *
	       Camera.hres * cosa * cosa * cosa);
  
  cosb = -VECTORDOTPRODUCT(newNode->m_normal, outDir);
  pdf = pdf * cosb / dist2;
  
  return pdf;
}


