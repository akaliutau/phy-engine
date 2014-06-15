
  /* **** Split BSDF : the most commonly used BSDF consisting of
     one brdf and one btdf **** */

#ifndef _SPLITBSDF_H_
#define _SPLITBSDF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bsdf.h"
#include "brdf.h"
#include "btdf.h"
#include "texture.h"

typedef struct SPLIT_BSDF
{
  BRDF *brdf;
  BTDF *btdf;
  TEXTURE *texture;
} SPLIT_BSDF;

extern SPLIT_BSDF *SplitBSDFCreate(BRDF *brdf, BTDF *btdf, TEXTURE* texture);

extern BSDF_METHODS SplitBsdfMethods;

  extern void SplitBsdfPrint(FILE *out, SPLIT_BSDF *bsdf);

#ifdef __cplusplus
}
#endif
#endif /* _SPLITBSDF_H_ */
