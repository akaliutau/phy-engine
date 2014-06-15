/* pmap.h: Photonmap */

#ifndef _PMAP_H_
#define _PMAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "radiance.h"
extern RADIANCEMETHOD Pmap;

  /* C external function defs */

void PmapTestReconstruction(POINT point, PATCH *patch, VECTOR dir);


#ifdef __cplusplus
}
#endif



#ifdef __cplusplus

// C++ external function defs
#include "pathnode.H"
#include "photonmap.H"

COLOR GetPmapNodeGRadiance(CPathNode *node);
COLOR GetPmapNodeCRadiance(CPathNode *node);

CPhotonMap *GetPmapGlobalMap(void);
//CPhotonMap *GetPmapCausticMap(void);

#endif

#endif /*_PMAP_H_*/
