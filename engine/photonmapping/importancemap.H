/* importancemap.H: the real importance map storage */

#ifndef _IMPORTANCEMAP_H_
#define _IMPORTANCEMAP_H_

#include "photonmap.H"

// Importance map, derived from photonmap

class CImportanceMap : public CPhotonMap
{
protected:
  float m_maxImp, m_avgImp;
  float m_totalMaxDistance;
  int m_preReconPhotons;
  float *m_impScalePtr;
public:
  // Constructor: always use irradiance percomputation
  CImportanceMap(int *estimate_nrp, float *impScalePtr) : CPhotonMap(estimate_nrp, true) 
  { 
    m_impScalePtr = impScalePtr;
  }
  
  // Override some photonmap functions
  
  virtual bool AddPhoton(CPhoton &photon, VECTOR &normal, 
			 short flags=0);
  virtual void PhotonPrecomputeIrradiance(CIrrPhoton *photon);
  virtual void PrecomputeIrradiance(void);
  
  // New functions

  virtual float ReconstructImp(VECTOR &pos, VECTOR &normal);
  virtual float GetImpReqDensity(VECTOR &pos, VECTOR &normal);
  virtual float GetRequiredDensity(VECTOR &pos, VECTOR &normal);
protected:
  void ComputeAllRequiredDensities(VECTOR &pos, VECTOR &normal, 
				   float *imp, float *pot, float *diff);

};

#endif /* _IMPORTANCEMAP_H_ */
