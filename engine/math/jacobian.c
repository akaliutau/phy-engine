

#include "jacobian.h"
#include "pools.h"

#ifndef NULL
#define NULL 0
#endif 

#ifndef NOPOOLS
static POOL *jacobianPool = (POOL *)NULL;
#define NEWJACOBIAN()  	(JACOBIAN *)NewPoolCell(sizeof(JACOBIAN), 0, "jacobians", &jacobianPool)
#define DISPOSEJACOBIAN(ptr) Dispose((unsigned char *)(ptr), &jacobianPool)
#else 
#define NEWJACOBIAN()	(JACOBIAN *)Alloc(sizeof(JACOBIAN))
#define DISPOSEJACOBIAN(ptr) Free((char *)ptr, sizeof(JACOBIAN))
#endif 

JACOBIAN *JacobianCreate(float A, float B, float C)
{
  JACOBIAN *jacobian = (JACOBIAN *)NULL;

  jacobian = NEWJACOBIAN();
  jacobian->A = A;
  jacobian->B = B;
  jacobian->C = C;

  return jacobian;
}

void JacobianDestroy(JACOBIAN *jacobian)
{
  DISPOSEJACOBIAN(jacobian);
}

