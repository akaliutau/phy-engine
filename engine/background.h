/* background.h: background environment for rendered scene (sky. environment map ...) */

#ifndef _PHY_BACKGROUND_H_
#define _PHY_BACKGROUND_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "background_methods.h"
#include "patch.h"

/* generic background object */
typedef struct BACKGROUND {
  void *data;				/* object state */
  PATCH *bkgPatch;                      /* virtual patch for background */
  struct BACKGROUND_METHODS *methods;	/* class methods operating on state */
} BACKGROUND;


/* see background_methods.h for explication. */

extern COLOR BackgroundRadiance(BACKGROUND *bkg, VECTOR *position, VECTOR *direction, float *pdf);

extern VECTOR BackgroundSample(BACKGROUND *bkg, VECTOR *position, float xi1, float xi2, COLOR *radiance, float *pdf);

extern COLOR BackgroundPower(BACKGROUND *bkg, VECTOR *position);


/* creates/destroys a background object */

extern BACKGROUND *BackgroundCreate(void *data, BACKGROUND_METHODS *methods);

extern void BackgroundDestroy(BACKGROUND *bkg);

#ifdef __cplusplus
}
#endif

#endif /*_PHY_BACKGROUND_H_*/
