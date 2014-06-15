/*
 * Photon kdtree : specialized kdtree with some photon map
 * specific additions
 */

#ifndef _PHOTONKDTREE_H_
#define _PHOTONKDTREE_H_

#include "../accelerators/kdtree.H"
#include "photon.H"

class CPhotonkdtree : public Ckdtree
{
public:
  CPhotonkdtree( int dimension, int dataSize, bool CopyData = true )
    : Ckdtree(dimension, dataSize, CopyData) {} ;

  // NormalPhotonQuery : find the nearest photon with a similar normal constraint
  // returns NULL is no appropriate photon was found (should barely ever happen)
  virtual CIrrPhoton* NormalPhotonQuery(VECTOR *pos, VECTOR *normal, 
					float threshold, float maxR2);
protected:
  void NormalBQuery_rec(const int index);
};

#endif /* _PHOTONKDTREE_H_ */
