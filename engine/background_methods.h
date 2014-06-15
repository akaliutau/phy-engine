/* background_methods.h */

#ifndef _BACKGROUND_METHODS_H_
#define _BACKGROUND_METHODS_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "vector.h"
#include "color.h"

typedef struct BACKGROUND_METHODS {
  /* evaluate background radiance coming in from direction (direction
   * points towards the background). If 'pdf' is non-null, also fills 
   * in the probability of sampling this direction with Sample() */
  COLOR (*Radiance)(void *data, VECTOR *position, VECTOR *direction, float *pdf);

  /* Samples a direction to the background, taking into account the
   * the radiance coming in from the background. The returned direction
   * is unique for given xi1, xi2 (in the range [0,1), including 0 but 
   * excluding 1). Directions on a full sphere may be returned. If a
   * direction is inappropriate, a new direction (with new numbers xi1, xi2)
   * needs to be sampled. If value or pdf is non-null, the radiance coming 
   * in from the sampled direction or the probability of sampling the
   * direction are computed on the fly. */
  VECTOR (*Sample)(void *data, VECTOR *position, float xi1, float xi2, COLOR *radiance, float *pdf);

  /* Computes total power emitted by the background (= integral over
   * the full sphere of the background radiance */
  COLOR (*Power)(void *data, VECTOR *position);

  void (*Print)(FILE *out, void *data);

  void (*Destroy)(void *data);
} BACKGROUND_METHODS;

#ifdef __cplusplus
}
#endif
#endif /* _BACKGROUND_METHODS_H_ */
