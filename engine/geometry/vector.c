

#include <math.h>

#include "error.h"

#include "vector.h"
#include "poolsP.h"
#include "pools.h"

#ifndef NOPOOLS
static POOL *vectorPool = (POOL *)NULL;
#define NEWVECTOR()  	(VECTOR *)NewPoolCell(sizeof(VECTOR), 0, "vectors", &vectorPool)
#define DISPOSEVECTOR(ptr) Dispose((unsigned char *)(ptr), &vectorPool)
#else 
#define NEWVECTOR()	(VECTOR *)Alloc(sizeof(VECTOR))
#define DISPOSEVECTOR(ptr) Free((char *)ptr, sizeof(VECTOR))
#endif 

static int TotalVectors = 0;


VECTOR *VectorCreate(float x, float y, float z)
{	
  VECTOR *vector;

  vector = NEWVECTOR();
  
  vector->x = x;
  vector->y = y;
  vector->z = z;
  
  TotalVectors++;
  
  return vector;
}


void VectorDestroy(VECTOR *vector)
{
  DISPOSEVECTOR(vector);
  
  TotalVectors--;
}


VECTOR *VectorCopy(VECTOR *vector)
{
  VECTOR *newvector;
  
  newvector = NEWVECTOR();
  *newvector = *vector;
  return newvector;
}


double VectorNormalize(VECTOR *vector)
{
  double norm;
  DVECTOR dvec;
  
  dvec.x = vector->x;
  dvec.y = vector->y;
  dvec.z = vector->z;
  
  norm = VECTORNORM(dvec);
  if (norm < EPSILON)
    return norm;
  
  VECTORSCALEINVERSE(norm, dvec, *vector);
  return norm;
}


int VectorFrame(VECTOR *Z, int proj_axis, VECTOR *X, VECTOR *Y)
{
  float dominant;
  VECTOR lX;

  if(X == NULL && Y == NULL)
    return TRUE;

  

  switch (proj_axis) {
  case XNORMAL: 
    VECTORSET(lX, -Z->y, Z->x, 0);
    dominant = Z->x;
    break;
  case YNORMAL: 
    VECTORSET(lX, 0, -Z->z, Z->y);
    dominant = Z->y;
    break;
  case ZNORMAL: 
    VECTORSET(lX, Z->z, 0, -Z->x);
    dominant = Z->z;
    break;
  default:
    Error("VectorFrame", "Unknown Projection Axis %i", proj_axis);
    return FALSE;
  }

  if(fabs(dominant) < EPSILON)
    return FALSE;

  VECTORNORMALIZE(lX);
  
  if(Y)
  {
    VECTORCROSSPRODUCT(*Z, lX, *Y);
  }

  if(X) *X = lX;

  return TRUE;
}


int VectorDominantCoord(VECTOR *v)
{
  DVECTOR anorm;
  double indexval;
  
  anorm.x = fabs(v->x);
  anorm.y = fabs(v->y);
  anorm.z = fabs(v->z);
  indexval = MAX(anorm.y, anorm.z);
  indexval = MAX(anorm.x, indexval);
  
  return (indexval == anorm.x ? XNORMAL :
	  (indexval == anorm.y ? YNORMAL : ZNORMAL));
}

int DVectorDominantCoord(DVECTOR *v)
{
  DVECTOR anorm;
  double indexval;
  
  anorm.x = fabs(v->x);
  anorm.y = fabs(v->y);
  anorm.z = fabs(v->z);
  indexval = MAX(anorm.y, anorm.z);
  indexval = MAX(anorm.x, indexval);
  
  return (indexval == anorm.x ? XNORMAL :
	  (indexval == anorm.y ? YNORMAL : ZNORMAL));
}

int VectorCompare(VECTOR *v1, VECTOR *v2, float tolerance)
{
  int code = 0;
  if (v1->x > v2->x + tolerance) code += X_GREATER;
  if (v1->y > v2->y + tolerance) code += Y_GREATER;
  if (v1->z > v2->z + tolerance) code += Z_GREATER;
  if (code!=0) 	
    return code;

  if (v1->x < v2->x - tolerance ||
      v1->y < v2->y - tolerance ||
      v1->z < v2->z - tolerance)
    return code;	

  return XYZ_EQUAL;	  
}

