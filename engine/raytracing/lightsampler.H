/*
 * lightsampler.H : Samples a point on a light source.
 *   Two implementations are given : uniform sampling and
 *   importance sampling
 */

#ifndef _LIGHTSAMPLER_H_
#define _LIGHTSAMPLER_H_

#include "sampler.H"
#include "lightlist.H"

class CUniformLightSampler : public CNextEventSampler
{
private:
  CLightList_Iter *iterator;
  PATCH *currentPatch;
  bool unitsActive;
public:
  CUniformLightSampler(void);

  // Units, see sampler.H
  virtual bool ActivateFirstUnit();
  virtual bool ActivateNextUnit();
  virtual void DeactivateUnits();

  // Sample : newNode gets filled, others may change
  virtual bool Sample(CPathNode *prevNode, CPathNode *thisNode,
		      CPathNode *newNode, double x_1, double x_2,
		      bool doRR = false, 
		      BSDFFLAGS flags = BSDF_ALL_COMPONENTS);
  virtual double EvalPDF(CPathNode *thisNode, CPathNode *newNode,
			 BSDFFLAGS flags = BSDF_ALL_COMPONENTS, 
			 double *pdf = NULL, double *pdfRR = NULL);
};

class CImportantLightSampler : public CNextEventSampler
{
public:
  // Sample : newNode gets filled, others may change
  virtual bool Sample(CPathNode *prevNode, CPathNode *thisNode,
		      CPathNode *newNode, double x_1, double x_2,
		      bool doRR = false, 
		      BSDFFLAGS flags = BSDF_ALL_COMPONENTS);
  virtual double EvalPDF(CPathNode *thisNode, CPathNode *newNode,
			 BSDFFLAGS flags = BSDF_ALL_COMPONENTS, 
			 double *pdf = NULL, double *pdfRR = NULL);
protected:
  VECTOR lastPoint, lastNormal;
};

#endif
