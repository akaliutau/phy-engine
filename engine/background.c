

#include "background.h"
#include "background_edf.H"
#include "material.h"
#include "surface.h"
#include "patch.h"
#include "edf.h"
#include "pools.h"
#include "error.h"

#ifdef NOPOOLS
#define NEWBACKGROUND()	(BACKGROUND *)Alloc(sizeof(BACKGROUND))
#define DISPOSEBACKGROUND(ptr) Free((char *)ptr, sizeof(BACKGROUND))
#else
static POOL *backgroundPool = (POOL *)NULL;
#define NEWBACKGROUND()	(BACKGROUND *)NewPoolCell(sizeof(BACKGROUND), 0, "backgrounds", &backgroundPool)
#define DISPOSEBACKGROUND(ptr) Dispose((char *)ptr, &backgroundPool)
#endif

BACKGROUND *BackgroundCreate(void *data, BACKGROUND_METHODS *methods)
{
  EDF *edf;
  MATERIAL *mat;

  BACKGROUND *bkg = NEWBACKGROUND();
  bkg->data = data;
  bkg->methods = methods;

  
  bkg->bkgPatch = PatchCreateVirtual();
  edf = BackgroundEdfCreate(bkg);
  mat = MaterialCreate("Backgroundlight", edf, NULL, 1);
  bkg->bkgPatch->surface = SurfaceCreate(mat, NULL, NULL, NULL, NULL, NULL, NO_COLORS);

  return bkg;
}

void BackgroundDestroy(BACKGROUND *bkg)
{
  if (!bkg) return;
  if (bkg->data && bkg->methods->Destroy)
    bkg->methods->Destroy(bkg->data);
  if(bkg->bkgPatch) PatchDestroy(bkg->bkgPatch);
  DISPOSEBACKGROUND(bkg);
}

void BackgroundPrint(FILE *out, BACKGROUND *bkg)
{
  if (!bkg) {
    fprintf(out, "No background\n");
    return;
  } 
  if (!bkg->methods->Print) {
    Error("BackgroundPrint", "Don't know how to print background data");
    return;
  }
  bkg->methods->Print(out, bkg->data);
}

COLOR BackgroundRadiance(BACKGROUND *bkg, VECTOR *position, VECTOR *direction, float *pdf)
{
  if (!bkg || !bkg->methods->Radiance) {
    COLOR black;
    COLORSETMONOCHROME(black, 0.);
    return black;
  } else
    return bkg->methods->Radiance(bkg->data, position, direction, pdf);
}

VECTOR BackgroundSample(BACKGROUND *bkg, VECTOR *position, float xi1, float xi2, COLOR *radiance, float *pdf)
{
  if (!bkg || !bkg->methods->Sample) {
    VECTOR dir = {0., 0., 0.};
    Fatal(-1, "BackgroundSample", "No background or no background sampling method");
    return dir;
  } else
    return bkg->methods->Sample(bkg->data, position, xi1, xi2, radiance, pdf);
}

COLOR BackgroundPower(BACKGROUND *bkg, VECTOR *position)
{
  if (!bkg || !bkg->methods->Power) {
    COLOR black;
    COLORSETMONOCHROME(black, 0.);
    return black;
  } else
    return bkg->methods->Power(bkg->data, position);
}

