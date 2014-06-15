/* adaptation.h: estimate static adaptation luminance in the current scene */

#ifndef _PHY_ADAPTATION_H_
#define _PHY_ADAPTATION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "patch.h"

/* Static adaptation estimation methods */
typedef enum TMA_METHOD { TMA_NONE , TMA_AVERAGE , TMA_MEDIAN , TMA_IDRENDER } TMA_METHOD;

/* estimates adaptation luminance in the current scene using the current
 * adaption estimation method in tmopts.statadapt (tonemapping.h).
 * 'patch_radiance' is a pointer to a routine that computes the radiance
 * emitted by a patch. The result is filled in tmopts.lwa. */
extern void EstimateSceneAdaptation(COLOR (*patch_radiance)(PATCH *));

/* same, but uses some a-priori estimate for the radiance emitted by a patch.
 * Used when laoding a new scene. */
extern void InitSceneAdaptation(void);

/* Estimates static adaptation luminance in the current image. Calls 
 * ray casting if the current image is not a ray-traced image. 
 * Uses the current adaptation luminance estimation strategy tmopts.statadapt
 * and fills in the result in tmopts.lwa. */
extern void EstimateViewAdaptation(void);

/* Re-estimates the adaptation luminance: calls either EstimateSceneAdaptation() or
 * EstimateViewAdaptation(), whichever was called last. */
extern void ReEstimateAdaptation(void);

#ifdef __cplusplus
}
#endif
#endif /*_PHY_ADAPTATION_H_*/
