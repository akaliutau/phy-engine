/* extmath.h */
#ifndef _MYMATH_H_INCLUDED_
#define _MYMATH_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <math.h>

#ifdef DRAND_LRAND_HAVE_NO_PROTOTYPE
extern double drand48(void);	/* not always defined in the header files */
extern long lrand48(void);
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif /*M_PI*/

#ifndef M_1_PI
#define	M_1_PI		0.31830988618379067154
#endif /*M_1_PI*/

#ifndef M_2_PI
#define	M_2_PI		0.63661977236758134308
#endif

#undef HUGE
#ifndef HUGE
#define HUGE 1e30
#endif /*HUGE*/

#ifndef M_LN2
#define M_LN2 0.69314718
#endif /*M_LN2*/

#ifdef __cplusplus
}
#endif

#endif /*_MYMATH_H_INCLUDED_*/
