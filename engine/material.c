

#include <string.h>
#include "material.h"
#include "pools.h"
#include "error.h"
#include "bsdf.h"
#include "splitbsdf.h"
#include "patch.h"

#ifdef NOPOOLS
#define NEWMATERIAL()	(MATERIAL *)Alloc(sizeof(MATERIAL))
#define DISPOSEMATERIAL(ptr) Free((char *)ptr, sizeof(MATERIAL))
#else
static POOL *materialPool = (POOL *)NULL;
#define NEWMATERIAL()	(MATERIAL *)NewPoolCell(sizeof(MATERIAL), 0, "materials", &materialPool)
#define DISPOSEMATERIAL(ptr) Dispose((char *)ptr, &materialPool)
#endif

MATERIAL defaultMaterial = {
  "(default)",
  (EDF *)NULL, (BSDF *)NULL,
  0	
};

MATERIAL *MaterialCreate(char *name, 
			 EDF *edf, BSDF *bsdf,
			 int sided)
{
  MATERIAL *m;

  m = NEWMATERIAL();
  m->name = Alloc(strlen(name) + 1);
  sprintf(m->name, "%s", name);
  m->sided = sided;
  m->edf = edf;
  m->bsdf = bsdf;
  m->radiance_data = (void*)NULL;
  
  return m;
}

MATERIAL *MaterialDuplicate(MATERIAL *mat)
{
  MATERIAL *m;

  m = NEWMATERIAL();
  m->name = Alloc(strlen(mat->name) + 1);
  sprintf(m->name, "%s", mat->name);
  m->sided = mat->sided;
  m->edf = EdfDuplicate(mat->edf);
  m->bsdf = BsdfDuplicate(mat->bsdf);
  m->radiance_data = (void*)NULL;

  return m;
}

void MaterialDestroy(MATERIAL *material)
{
  if (material->name) Free(material->name, strlen(material->name)+1);
  if (material->edf)  EdfDestroy(material->edf);
  if (material->bsdf)  BsdfDestroy(material->bsdf);
  DISPOSEMATERIAL(material);
}

void MaterialPrint(FILE *out, MATERIAL *material)
{
  fprintf(out, "material '%s': %s\n", material->name,
	  material->sided ? "SINGLE SIDED" : "TWOSIDED");

  if (material->edf) {
    fprintf(out, "\t");
    EdfPrint(out, material->edf);
  } else
    fprintf(out, "\tNot a luminaire\n");

  if (material->bsdf) {
    fprintf(out, "\t");
    BsdfPrint(out, material->bsdf);
  } else
    fprintf(out, "\tNot scattering\n");
}

int MaterialShadingFrame(HITREC *hit, VECTOR *X, VECTOR *Y, VECTOR *Z)
{
  int succes = FALSE;

  if (!HitInitialised(hit)) {
    Warning("MaterialShadingFrame", "uninitialised hit structure");
    return FALSE;
  }

  if (hit->material && hit->material->bsdf && hit->material->bsdf->methods->ShadingFrame) {
    succes = BsdfShadingFrame(hit->material->bsdf, hit, X, Y, Z);
  }

  if (!succes && hit->material && hit->material->edf && hit->material->edf->methods->ShadingFrame) {
    succes = EdfShadingFrame(hit->material->edf, hit, X, Y, Z);
  }

  if (!succes && HitUV(hit, &hit->uv)) {
    
#ifdef NEVER
    VECTOR N = PatchInterpolatedNormalAtUV(hit->patch, hit->uv.u, hit->uv.v);
    if (Z) *Z = N;
    if (X && Y && Z) {
      double zz = sqrt(1 - Z->z*Z->z);
      if (zz < EPSILON) {	
	VECTORSET(*X, 1., 0., 0.);
      } else {
	VECTORSET(*X, Z->y/zz, -Z->x/zz, 0.);
      }
      VECTORCROSSPRODUCT(*Z, *X, *Y);
    }
#endif
    
    PatchInterpolatedFrameAtUV(hit->patch, hit->uv.u, hit->uv.v, X, Y, Z);
    succes = TRUE;
  }

  return succes;
}
