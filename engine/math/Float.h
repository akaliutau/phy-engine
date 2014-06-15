/* Float.h */

#ifndef _PHY_FLOAT_H_
#define _PHY_FLOAT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "extmath.h"

#define EPSILON	1e-6

/* tests whether two floating point numbers are eual within the given tolerance */
#define FLOATEQUAL(a, b, tolerance)	(((a)-(b)) > -(tolerance) && ((a)-(b)) < (tolerance))

#ifndef MAX
#define MAX(a, b)		((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b)		((a) < (b) ? (a) : (b))
#endif

#define phy_float float

#ifdef __cplusplus
}
#endif

#endif /*_PHY_FLOAT_H_*/
