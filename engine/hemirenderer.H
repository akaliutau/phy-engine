/* 
 * hemirenderer.H : tool class to render data defined on the
 *   hemisphere. 
 */

#ifndef _HEMIRENDERER_H_
#define _HEMIRENDERER_H_

#include "color.h"
#include "vector.h"
#include "spherical.h"

typedef void (*CHRAcquireCallback)(VECTOR *vec, COORDSYS *coordsys,
				   double phi, double theta, void *data,
				   RGB *color, double *distance);

class CHRVertexData
{
public:
  VECTOR point;
  VECTOR normal;
  RGB rgb;
};

class CHemisphereRenderer
{
public:
  // Constructor : nr. of steps in phi and theta
  CHemisphereRenderer(void);
  
  ~CHemisphereRenderer(void);

  // AcquireData fills in the data on the hemisphere.
  // The callback is called for every phi and theta value.

  void Initialize(int Nphi, int Ntheta, VECTOR center, 
		  COORDSYS *coordsys,
		  CHRAcquireCallback cb, void *cbData);

  void EnableRendering(bool flag);

  // Render : renders the hemispherical data

  void Render(void);

protected:
  bool m_renderEnabled;
  int m_phiSteps, m_thetaSteps;
  double m_deltaPhi, m_deltaTheta;
  VECTOR m_center;
  COORDSYS m_coordsys;
  CHRVertexData *m_renderData; // Data in WC, together with normals
  CHRVertexData m_top;
};


#endif /* _HEMIRENDERER_H_ */
