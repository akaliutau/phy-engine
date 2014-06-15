/* statistics.h: global variables to print statistics */

#ifndef _STATISTICS_H_
#define _STATISTICS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "color.h"

/* general statstics about the current scene */
extern int
  nrgeoms,		/* current nr of geometries in memory */
  nrcompounds,		/* current number of compounds */
  nrsurfaces,		/* current number of surfaces in memory */
  nrvertices,		/* current number of vertices */
  nrpatches,		/* number of patches just after loading the scene */
  nrelements,		/* current number of patches */
  nrlightsources;       /* number of light emitting patches */

extern double total_area, average_direct_potential, max_direct_potential, 
              max_direct_importance /* potential times area  */, 
              total_direct_potential,
              reference_luminance;

extern COLOR total_emitted_power, estimated_average_radiance,
             average_reflectivity,
             max_selfemitted_radiance, max_selfemitted_power;

extern void UpdateFileStats(void);

/* nr of. shadow cached rays and cache hits. */
extern int ShadowRays, ShadowCacheHits; 	

#ifdef __cplusplus
}
#endif

#endif /*_STATISTICS_H_*/
