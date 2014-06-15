/*
 * densitykernel.H : Density kernel functions
 * Many routines borrowed from Density Estimation master thesis by
 * Olivier Ceulemans.
 */

#ifndef _DENSITYKERNEL_H_
#define _DENSITYKERNEL_H_

#ifdef __cplusplus

#include "vector2d.h"
#include "screenbuffer.H"

class CKernel2D
{
protected:
  float m_h;       // Kernel size.
  float m_h2;      // h2=h*h.
  float m_h2inv;   // h2inv=1/h2.
  float m_weight;  // The weight of the kernel

public:
  CKernel2D(void);
  void Init(float h, float w);

  // Get the kernel width.
  // OUT: The kernel width.
  inline float GetH(void) { return m_h;}


  // Change kernel width.
  // IN:   The new kernel width, newh.
  // PRE:  newh>0.0f
  // POST: The kernel size is newh.
  void SetH(const float newh);

  // Check if one point is inside a kernel.
  // IN:  The position of a 2D point.
  //      The position of the center of the kernel.
  // OUT: 0    , if the point is outside the kernel.
  //      non 0, otherwise.
  bool IsInside(const VEC2D &point,const VEC2D &center);

  // Evaluate the kernel.
  // IN:  The position of a 2D point.
  //      The position of the center of the kernel.
  // OUT: The value of the kernel at the point. (a positive number or 0.0f)
  float Evaluate(const VEC2D &point,const VEC2D &center);

  // Check if one line segment intersects a kernel.
  // IN:  The origin of the line segment.
  //      The direction of the line segment.
  //      The length of the line segment.
  //      The position of the center of the kernel.
  // PRE: The direction vector has a length of 1.0f.
  // OUT: 0    , if there are no intersections.
  //      non 0, otherwise.
  // int IntersectSegment(const VEC2D &lorg,const VEC2D &ldir,const float ls,const VEC2D &cc);

  // Cover the affected pixels of a screenbuffer with the kernel
  // IN: The screenbuffer to cover.
  // OUT: /
  void Cover(const VEC2D &point, float factor, COLOR &col, CScreenBuffer *screen);
  void VarCover(const VEC2D &center, COLOR &col, CScreenBuffer *ref,
		CScreenBuffer *dest, int totalSamples, int scaleSamples,
		float baseSize = 4.0);
};

#endif /* __cplusplus */

#endif /* __DENSITYKERNEL_H_ */
