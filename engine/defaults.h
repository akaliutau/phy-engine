/* defaults.h: rad program defaults */

#ifndef _PHY_DEFAULTS_H_
#define _PHY_DEFAULTS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PHYHOME
#define PHYHOME "https://github.com/akalu/phy-engine"
#endif

#include "Boolean.h"

/* default radiance method: a short name of a radiance method or "none" */
#define DEFAULT_RADIANCE_METHOD		"none"

/* raytracing defaults */
#define DEFAULT_RAYTRACING_METHOD	"stochastic"

/* default rendering options */
#define DEFAULT_DISPLAY_LISTS		FALSE
#define DEFAULT_SMOOTH_SHADING  	TRUE
#define DEFAULT_BACKFACE_CULLING 	TRUE
#define DEFAULT_OUTLINE_DRAWING 	FALSE
#define DEFAULT_BOUNDING_BOX_DRAWING 	FALSE
#define DEFAULT_CLUSTER_DRAWING 	FALSE

#define DEFAULT_OUTLINE_COLOR 		{0.5, 0.0, 0.0}
#define DEFAULT_BOUNDING_BOX_COLOR 	{0.5, 0.0, 1.0}
#define DEFAULT_CLUSTER_COLOR 		{1.0, 0.5, 0.0}

/* default virtual camera */
#define DEFAULT_EYEP			{10.0, 0.0, 0.0}
#define DEFAULT_LOOKP			{ 0.0, 0.0, 0.0}
#define DEFAULT_UPDIR			{ 0.0, 0.0, 1.0}
#define DEFAULT_FOV			22.5
#define DEFAULT_BACKGROUND_COLOR	{0.0, 0.0, 0.0}

/* MGF defaults */
#define DEFAULT_NQCDIVS			4
#define DEFAULT_FORCE_ONESIDEDNESS	TRUE
#define DEFAULT_MONOCHROME		FALSE

/* Tone mapping defaults */
#ifndef DEFAULT_GAMMA
#define DEFAULT_GAMMA			1.
#endif /*DEFAULT_GAMMA*/
#define DEFAULT_TM_LWA			10.0
#define DEFAULT_TM_LDMAX		100.0
#define DEFAULT_TM_CMAX			50.0

#ifdef __cplusplus
}
#endif

#endif /*_PHY_DEFAULTS_H_*/
