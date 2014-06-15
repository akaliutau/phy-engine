/* background_edf.h: edf methods for backgrounds */

#ifndef _BACKGROUND_EDF_H_
#define _BACKGROUND_EDF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "vector.h"
#include "color.h"
#include "edf.h"
#include "scene.h"

  EDF *BackgroundEdfCreate(BACKGROUND *bkg);

#ifdef __cplusplus
}
#endif

#endif /* _BACKGROUND_EDF_H_ */
