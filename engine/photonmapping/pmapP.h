/* pmapP.h*/

#ifndef _PMAPP_H_
#define _PMAPP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "color.h"
#include <time.h>


/* data to be associated with each patch */
typedef struct PMAP_DATA {
  /* int nrhits; */
} PMAP_DATA;



/* macros that save a lot of typing work. */
#define NRHITS(patch)	(((PMAP_DATA *)((patch)->radiance_data))->nrhits)


extern void CreatePmapControlPanel(void *parent_widget);
extern void ShowPmapControlPanel(void);
extern void HidePmapControlPanel(void);

void PmapChooseSurfaceSampler(void);

void PmapBalance(PMAP_TYPE type);

void PmapRaycast(void);
void PmapRaycastInterrupt(void);
void PmapRedisplayRaycast(void);
void PmapDoPrintValue(PATCH *P, VECTOR *hitp);

#ifdef __cplusplus
}
#endif

#endif /*_PMAPP_H_*/
