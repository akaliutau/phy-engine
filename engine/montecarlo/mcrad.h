/* mcrad.h: Monte Carlo radiosity */

#ifndef _MCRAD_H_
#define _MCRAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "radiance.h"
extern RADIANCEMETHOD StochasticRelaxationRadiosity;
extern RADIANCEMETHOD RandomWalkRadiosity;

#ifdef __cplusplus
}
#endif

#endif /*_MCRAD_H_*/
