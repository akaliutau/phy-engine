/* rtstochastic.h : stochastical ray tracing method */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _RTSTOCHASTIC_H_
#define _RTSTOCHASTIC_H_

#include "raytracing.h"
#include "color.h"
extern RAYTRACINGMETHOD RT_StochasticMethod;

void RTStochastic_DebugPixel(int nx, int ny);
COLOR RTStochastic_GetPixel(int nx, int ny);

#endif /* _RTSTOCHASTIC_H_ */

#ifdef __cplusplus
}
#endif
