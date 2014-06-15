

#include "geom.h"
#include "patch.h"
#include "material.h"
#include "hit.h"

int HitInitialised(HITREC *hit)
{
  return ((hit->flags & HIT_PATCH) || (hit->flags & HIT_GEOM))
    && (hit->flags & HIT_POINT)
    && (hit->flags & HIT_GNORMAL)
    && (hit->flags & HIT_MATERIAL)
    && (hit->flags & HIT_DIST);
}

int InitHit(HITREC *hit, PATCH *patch, GEOM *geom, 
	    VECTOR *point, VECTOR *gnormal,
	    MATERIAL *material, float dist)
{
  hit->flags = 0;
  hit->patch = patch;	if (patch) hit->flags |= HIT_PATCH;
  hit->geom = geom;	if (geom) hit->flags |= HIT_GEOM;
  if (point) { 
    hit->point = *point;
    hit->flags |= HIT_POINT;
  }
  if (gnormal) { 
    hit->gnormal = *gnormal;
    hit->flags |= HIT_GNORMAL;
  }
  hit->material = material;	hit->flags |= HIT_MATERIAL;
  hit->dist = dist;		hit->flags |= HIT_DIST;
  VECTORSET(hit->normal, 0, 0, 0);
  hit->texCoord = hit->X = hit->Y= hit->Z = hit->normal;
  hit->uv.u = hit->uv.v = 0.;
  return HitInitialised(hit);
}

int HitUV(HITREC *hit, DVEC2D *uv)
{
  if (hit->flags & HIT_UV) {
    *uv = hit->uv;
    return TRUE;
  }

  if ((hit->flags & HIT_PATCH) && (hit->flags && HIT_POINT)) {
    PatchUV(hit->patch, &hit->point, &hit->uv.u, &hit->uv.v);
    *uv = hit->uv;
    hit->flags |= HIT_UV;
    return TRUE;
  }

  return FALSE;
}

int HitTexCoord(HITREC *hit, VECTOR *texCoord)
{
  if (hit->flags & HIT_TEXCOORD) {
    *texCoord = hit->texCoord;
    return TRUE;
  }

  if (!HitUV(hit, &hit->uv))
    return FALSE;

  if (hit->flags & HIT_PATCH) {
    *texCoord = hit->texCoord = PatchTextureCoordAtUV(hit->patch, hit->uv.u, hit->uv.v);
    hit->flags |= HIT_TEXCOORD;
    return TRUE;    
  }

  return FALSE;
}

int HitShadingFrame(HITREC *hit, VECTOR *X, VECTOR *Y, VECTOR *Z)
{
  if (hit->flags & HIT_SHADINGFRAME) {
    *X = hit->X; *Y = hit->Y; *Z = hit->Z;
    return TRUE;
  }

  if (!MaterialShadingFrame(hit, &hit->X, &hit->Y, &hit->Z))
    return FALSE;

  hit->normal = hit->Z;	
  hit->flags |= HIT_SHADINGFRAME|HIT_NORMAL;

  *X = hit->X; *Y = hit->Y; *Z = hit->Z;
  return TRUE;
}

int HitShadingNormal(HITREC *hit, VECTOR *normal)
{
  if (hit->flags & HIT_SHADINGFRAME || hit->flags & HIT_NORMAL) {
    *normal = hit->normal;
    return TRUE;
  }

  if (!MaterialShadingFrame(hit, NULL, NULL, &hit->normal))
    return FALSE;

  hit->flags |= HIT_NORMAL;
  *normal = hit->Z = hit->normal;
  return TRUE;
}
