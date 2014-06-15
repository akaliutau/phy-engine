/* ray.h */

#ifndef _RAY_H_
#define _RAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vector.h"
#include "hit.h"

/* ray struct */
typedef struct RAY {
  VECTOR pos, dir;	/* direction is supposed to be normalized. */
} RAY;

#ifdef __cplusplus
}
#endif

#endif /*_RAY_H_*/

