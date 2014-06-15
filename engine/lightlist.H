/*
 * lightlist.H : definition of the lightlist class
 *  this class can be used for sampling lights
 */

#ifndef _LIGHTLIST_H_
#define _LIGHTLIST_H_

#include "patchlist.h"
#include "vector.h"

#ifdef __cplusplus
extern "C" {
#endif

double LightEvalPDFImportant(PATCH *light, VECTOR *lightPoint,
			     VECTOR *point, VECTOR *normal);

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus

#include "CSList.H"

class CLightInfo
{
public:
  float emittedFlux;
  float importance; // cumlative probability : for importance sampling
  PATCH *light;
};


class CLightList_Iter;

class CLightList : private CTSList<CLightInfo>
{
private:
  // Total flux ( sum(L * A * PI))
  float totalFlux, totalImp;
  bool includeVirtual;
  
  int lightCount;

  CTSList_Iter<CLightInfo> *current;
  
public:

  // Iteration over lights, not multithreaded!!

  PATCH *GetFirstLight(void);
  PATCH *GetNextLight(void);

  // Discrete sampling of lightsources

  // A patchlist must be supplied for building a light list.
  // Non emitting patches (edf == NULL) are NOT put in the list. 
  CLightList(PATCHLIST *list, bool includeVirtualPatches = false);
  ~CLightList();

  // mutator of includeVirtual
  void IncludeVirtualPatches(bool newValue);

  // Normal sampling : uniform over emitted power
  PATCH *Sample(double *x_1, double *pdf);


  // Normal PDF evaluation : uniform over emitted power
  double EvalPDF(PATCH *light, POINT *point);

  // Importance sampling routines

  PATCH *SampleImportant(POINT *point, VECTOR *normal, double *x_1, 
			 double *pdf);
  double EvalPDFImportant(PATCH *light, POINT *lightPoint,
			  POINT *litPoint, VECTOR *normal);
protected:
  VECTOR lastPoint, lastNormal;

  void ComputeLightImportances(VECTOR *point, VECTOR *normal);
  double ComputeOneLightImportance(PATCH *light, const VECTOR *point, 
				   const VECTOR *normal,
				   float avgEmittedRadiance);


  // specialisations by patch type (normal or virtual) of ComputeOneLightImportance
  double ComputeOneLightImportance_virtual(PATCH *light, const VECTOR *point, 
					   const VECTOR *normal,
					   float avgEmittedRadiance);
  double ComputeOneLightImportance_real (PATCH *light, const VECTOR *point, 
					 const VECTOR *normal,
					 float avgEmittedRadiance);

  // specialisations by patch type (normal or virtual) of EvalPDF
  double EvalPDF_virtual(PATCH *light, POINT *point);
  double EvalPDF_real(PATCH *light, POINT *point);

  friend class CLightList_Iter;
};


class CLightList_Iter
{
private:
  CTSList_Iter<CLightInfo> m_iter;
public:
  CLightList_Iter(CLightList& list) : m_iter(list) {}
  PATCH *First(CLightList& list) 
  { 
    m_iter.Init(list); 

    CLightInfo *li = m_iter.Next();
    if(li != NULL)
      return li->light;
    else 
      return NULL;
  }
  PATCH *Next() 
  { 
    CLightInfo *li = m_iter.Next();
    if(li)
      return li->light;
    else
      return NULL;
  }
};


/********************************************************************/

// global var for the scene light list

extern CLightList *gLightList;

#endif

#endif
