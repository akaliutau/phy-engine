

#include <math.h>

#include "pixelsampler.H"
#include "raytools.H"
#include "scene.h"
#include "camera.h"
#include "error.h"

bool CPixelSampler::Sample(CPathNode *, CPathNode *thisNode,
			   CPathNode *newNode, double x_1, double x_2,
			   bool , BSDFFLAGS )
{
  VECTOR dir;

  // Precond: thisNode == eye, prevNode == NULL, SetPixel called

  // Sample direction
  double xsample = (m_px + Camera.pixh * x_1);
  double ysample = (m_py + Camera.pixv * x_2);

  VECTORCOMB3(Camera.Z, xsample, Camera.X, ysample, Camera.Y, dir);
  double distPixel2 = VECTORNORM2(dir);
  double distPixel = sqrt(distPixel2);
  VECTORSCALEINVERSE(distPixel, dir, dir);

  double cosPixel = fabs(VECTORDOTPRODUCT(Camera.Z, dir));

  double pdfDir = ((1. / (Camera.pixh * Camera.pixv)) * // 1 / Area pixel
		   (distPixel2 / cosPixel));  // spher. angle measure 

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

  newNode->m_rracc = thisNode->m_rracc; // No russian roulette

  return true;
}

void CPixelSampler::SetPixel(int nx, int ny, CAMERA *cam)
{
  m_nx = nx;
  m_ny = ny;

  if(cam == NULL) cam = &Camera; // Primary camera

  m_px = -cam->pixh * cam->hres / 2.0 + nx * cam->pixh;
  m_py = -cam->pixv * cam->vres / 2.0 + ny * cam->pixv;
}

double CPixelSampler::EvalPDF(CPathNode *thisNode, CPathNode *newNode,
			      BSDFFLAGS , double* , 
			      double* )
{
  double dist2, dist, cosa, cosb, pdf;
  VECTOR outDir;

  

  VECTORSUBTRACT(newNode->m_hit.point, thisNode->m_hit.point, outDir);
  dist2 = VECTORNORM2(outDir);
  dist = sqrt(dist2);
  VECTORSCALEINVERSE(dist, outDir, outDir);

  // pdf = 1 / A_pixel transformed to area measure
  
  cosa = VECTORDOTPRODUCT(thisNode->m_normal, outDir);

  // pdf = 1/Apix * (r^2 / cos(dir, eyeNormal) * (cos(dir, patchNormal) / d^2)
  //                 |__> to sper.angle           |__> to area on patch
  
  // Three cosines : r^2 / cos = 1 / cos^3 since r is length
  // of viewing ray to the pixel.
  pdf = 1.0 / (Camera.pixv * Camera.pixh * cosa * cosa * cosa);
  
  cosb = -VECTORDOTPRODUCT(newNode->m_normal, outDir);
  pdf = pdf * cosb / dist2;
  
  return pdf;
}

