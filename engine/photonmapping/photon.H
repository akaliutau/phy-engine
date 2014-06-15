/* photon.H: data structur for individual photons */

#ifndef _PHOTON_H_
#define _PHOTON_H_

#include <vector.h>
#include <color.h>
#include <spherical.h>
#include <memory.h>
#include <cstring>

#include "pmapoptions.H"

// KD tree flags (currently not used)
const short DIRECT_LIGHT_PHOTON = 0x10;
const short CAUSTIC_LIGHT_PHOTON = 0x20; // Lower 4 bits reserved for kd tree
// This type of photon should not be included in the importance sampling
const short NO_IMPSAMP_PHOTON = DIRECT_LIGHT_PHOTON | CAUSTIC_LIGHT_PHOTON;


// Non compact photon representation

class CPhoton
{
protected:
  VECTOR m_pos;  // Position: 3 floats, MUST COME FIRST for kdtree storage
  COLOR m_power;  // Power represented by this photon
  //  float m_dcWeight; // Weight for density control
  VECTOR m_dir;  // Direction

public:
  // Constructor

  CPhoton() {};
  CPhoton(VECTOR &pos, COLOR &power, VECTOR &dir)
    :m_pos(pos), m_power(power), m_dir(dir) { /*m_dcWeight = 1.0;*/ }

  // Accessor functions

  inline VECTOR Pos(void) { return m_pos; }
  inline COLOR Power(void) { return m_power; }
  inline void AddPower(COLOR col) { COLORADD(m_power, col, m_power); }
  inline VECTOR Dir(void) { return m_dir; }  

  //  inline float DCWeight(void) { return m_dcWeight; }  
  //  inline void SetDCWeight(float w) { m_dcWeight = w; }

  // Importance sampling utility functions

  // Find the r,s values in a [0,1[^2 square corresponding to the photon
  void FindRS(double *r, double *s, COORDSYS *coord,
	      BSDFFLAGS flag, float n);
};


// CIrrPhoton: photon with extra irradiance info
class CIrrPhoton : public CPhoton
{
public:
  //protected:
  VECTOR m_normal;
  COLOR m_irradiance;
public:
  inline VECTOR Normal(void) { return m_normal; }
  inline void SetNormal(const VECTOR& normal) {m_normal = normal;}
  inline COLOR Irradiance(void) { return m_irradiance; }
  inline void SetIrradiance(const COLOR& irr) { m_irradiance = irr; }
  inline void Copy(const CPhoton &photon)
  {
     memcpy((char *)this, (char *)&photon, sizeof(CPhoton));
  }
};


// CImporton: identical to CIrrPhoton, but with some extra functions

class CImporton : public CIrrPhoton
{
public:
  inline void SetAll(float imp, float pot, float foot)
  {
    // Abuse m_power for importance estimates. 
    // -- AT LEAST 3 COLOR components needed!  Watch out with compact photon repr.
    ((float *)m_power.spec)[USE_IMPORTANCE] = imp;
  }    

  inline void PSetAll(float imp, float pot, float foot)
  {
    // Abuse m_power for importance estimates. 
    // -- AT LEAST 3 COLOR components needed!  Watch out with compact photon repr.
    ((float *)m_irradiance.spec)[USE_IMPORTANCE] = imp;
  }    

  // Constructor:
  CImporton(VECTOR &pos, float importance, float potential, float footprint, 
	    VECTOR &dir)
  {
    m_pos = pos;
    m_dir = dir;

    SetAll(importance, potential, footprint);
  }

public:
  inline float Importance(void) { return ((float *)m_power.spec)[USE_IMPORTANCE];}
  inline float PImportance(void) { return ((float *)m_irradiance.spec)[USE_IMPORTANCE];}
};


// TODO Compact photon representation, cfr. Jensen Siggraph 99 course notes

/*
#include <packedcolor.h>


class CPhoton
{
  VECTOR m_pos;  // Position: 3 floats, MUST COME FIRST for kdtree storage
 
  PACKEDCOLOR m_power;  // Power represented by this photon

  char m_phi, m_theta;  // Direction
}
*/
#endif
