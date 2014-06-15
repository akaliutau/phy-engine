/* packedcolor.h : Cfr. Real Pixels, G.Ward, Graphic Gems II */

#ifndef _REALPIXELS_H_
#define _REALPIXELS_H_

#include "color.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PACKEDCOLOR
{
  unsigned char r,g,b, exp;
} PACKEDCOLOR;

extern PACKEDCOLOR PackColor(COLOR col);
extern COLOR UnpackColor(PACKEDCOLOR pcol);

#ifdef __cplusplus
}
#endif

#endif
