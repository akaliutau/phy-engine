/* photonmap.H: the real photonmap storage */

#ifndef _PHOTONMAP_H_
#define _PHOTONMAP_H_

#include "vector.h"
#include "color.h"
#include "photonkdtree.H"
#include "photon.H"
#include "bsdf.h"
#include "spherical.h"
#include "samplegrid.H"
#include "pmapoptions.H"


bool ZeroAlbedo(BSDF *bsdf, HITREC *hit, BSDFFLAGS flags);
// Convert a value val given a maximum into some nice color
COLOR GetFalseColor(float val);
COLOR GetFalseColorMonochrome(float val);
bool ZeroAlbedo(BSDF *bsdf, VECTOR *pos, BSDFFLAGS flags);


class CPhotonMap
{
protected:
  bool m_balanced;
  bool m_doBalancing;

  bool m_precomputeIrradiance;
  bool m_irradianceComputed;


  // kdtree storage

  int *m_estimate_nrp; // Points to GUI changeable value (comfort)
  int m_sample_nrp;
  int m_nrPhotons;
  int m_totalPhotons;
  long m_totalPaths; // Number of traced paths, not number of photons!!
  // Stored flux value still has to be divided by the total number of paths.

  CPhotonkdtree *m_kdtree;

  // A grid is permanently allocated for importance sampling
  CSampleGrid2D *m_grid;
  VECTOR m_sampleLastPos;

  // Space to hold the photons and distances for queries

  int m_nrpFound; // Number of photons in the array
  int m_nrpCosinePos; /* Number of photons in the array that have
			direction * normal > 0, where normal is
			the a supplied reconstruction normal. */
  
  CPhoton **m_photons;
  float *m_distances;
  float *m_cosines; // photon dir * reconstruction normal
  bool m_cosinesOk; // indicates if cosines are computed

protected:
  // nearest photon queries must use these fucntions!
  int DoQuery(VECTOR *pos, int nrPhotons, float maxradius,
	      short excludeFlags = 0)
  {
    m_cosinesOk = false;
    return m_kdtree->Query((float*)pos, nrPhotons, 
			   m_photons, m_distances, maxradius,
			   excludeFlags);
  }

  int DoQuery(VECTOR *pos)
  {
    m_cosinesOk = false;
    return m_kdtree->Query((float*)pos, *m_estimate_nrp /*pmapstate.reconPhotons*/, 
			   m_photons, m_distances, GetMaxR2());
  }

  CIrrPhoton* DoIrradianceQuery(VECTOR *pos, VECTOR *normal, 
				float maxR2 = HUGE)
  {
    return m_kdtree->NormalPhotonQuery(pos, normal, 0.8, maxR2);
  }

  // Compute cosines of photons with a supplied normal
  void ComputeCosines(VECTOR &normal);

  // Add a photon taking possible irrPhoton into account
  void DoAddPhoton(CPhoton &photon, VECTOR &normal, short flags);

public:
  // Constructor

  CPhotonMap(int *estimate_nrp, bool doPrecomputeIrradiance = false);

  // Destructor

  virtual ~CPhotonMap(void);

  // Initialize

  void Init(void);

  // Set total paths

  void SetTotalPaths(long totalPaths) { m_totalPaths = totalPaths; }
  void AddPath(void) { m_totalPaths++; }

  // Adding photons, returns if photon was added

  virtual bool AddPhoton(CPhoton &photon, VECTOR &normal, short flags=0);

  bool DC_AddPhoton(CPhoton &photon, HITREC &hit,
		    float requiredD, short flags=0);
  void Redistribute(CPhoton &photon, float acceptProb, short flags=0);

  // Get a maximum radius^2 for locating nearest photons
  virtual double GetMaxR2(void);

  // Precompute irradiance
  virtual void PrecomputeIrradiance(void);
  // For 1 specific photon
  virtual void PhotonPrecomputeIrradiance(CIrrPhoton *photon);

  // Reconstruct
  virtual COLOR Reconstruct(HITREC *hit, VECTOR &outDir,
			    BSDF *bsdf, BSDF* inBsdf, BSDF *outBsdf);

  /*
  bool IrradianceReconstruct(HITREC *hit, VECTOR &outDir, 
					 BSDF *bsdf, BSDF *inBsdf,
					 BSDF *outBsdf,
					 COLOR *result);
  */

  bool IrradianceReconstruct(HITREC *hit, VECTOR &outDir, 
			     COLOR& diffuseAlbedo,
			     COLOR *result);
    
  virtual float GetCurrentDensity(HITREC &hit, int nrPhotons=0);
  // Return a color coded density of the photonmap
  virtual COLOR GetDensityColor(HITREC &hit);


  // Sample values: Random values r,s are transformed into new
  // random values so that importance sampling using the photon
  // map is incorportated.

  // IN: r,s in [0,1[ 
  //     coord: coordinate system that determines angles
  //     flags: component that will be sampled (GR or DR !!)
  //     n: phong exponent for GR

  // OUT: r,s are changed for importance sampling, pdf is returned

  double Sample(VECTOR &pos, double *r, double *s, COORDSYS *coord,
	      BSDFFLAGS flag, float n=1);
  double DebugSample(VECTOR &pos, double *r, double *s, COORDSYS *coord,
	      BSDFFLAGS flag, float n=1);
  float GetGridValue(double phi, double theta);

  // Utility functions

  void PrintStats(FILE *fp)
  {
    fprintf(fp, "%i stored photons\n", m_nrPhotons);
  }

  void GetStats(char *p)
    {
      sprintf(p, 
	      "%i stored photons, %i total, %li paths\n", 
	      m_nrPhotons, m_totalPhotons, m_totalPaths);
    }

  void BalanceAnalysis(void)
    {
      m_kdtree->BalanceAnalysis();
    }

  void Balance(void)
  {
    m_kdtree->Balance();
  }

  void CheckNBalance(void)
  {
    if((!m_balanced) && (m_doBalancing || m_precomputeIrradiance))
      Balance();
  }

  void DoBalancing(bool state)
  {
    m_doBalancing = state;
  }
};

#endif
