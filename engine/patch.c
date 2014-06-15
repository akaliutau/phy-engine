

#include <stdarg.h>

#include "patch.h"
#include "pools.h"
#include "error.h"
#include "polygon.h"
#include "radiance.h"
#include "vertex.h"
#include "patch_flags.h"
#include "mcsampling/nied31.h"

#ifdef NOPOOLS
#define NEWPATCH()	(PATCH *)Alloc(sizeof(PATCH))
#define DISPOSEPATCH(ptr) Free((char *)ptr, sizeof(PATCH))
#else
static POOL *patchPool = (POOL *)NULL;
#define NEWPATCH()	(PATCH *)NewPoolCell(sizeof(PATCH), 0, "patches", &patchPool)
#define DISPOSEPATCH(ptr) Dispose((char *)ptr, &patchPool)
#endif


static int patchid = 1;
int nrelements;


int PatchGetNextID(void)
{
  return patchid;
}


void PatchSetNextID(int id)
{
  patchid = id;
}


void PatchConnectVertex(PATCH *patch, VERTEX *vertex)
{
  vertex->patches = PatchListAdd(vertex->patches, patch);
}


void PatchConnectVertices(PATCH *patch)
{
  int i;

  for (i=0; i<patch->nrvertices; i++)
    PatchConnectVertex(patch, patch->vertex[i]);
}


VECTOR *PatchNormal(PATCH *patch, VECTOR *normal)
{
  double norm;
  DVECTOR prev, cur;
  int i;

  VECTORSET(*normal,0,0,0);
  VECTORSUBTRACT(*patch->vertex[patch->nrvertices-1]->point,
		 *patch->vertex[0]->point,cur);
  for(i=0; i<patch->nrvertices; i++) {
    prev = cur;
    VECTORSUBTRACT(*patch->vertex[i]->point,*patch->vertex[0]->point,cur);
    normal->x += (prev.y - cur.y) * (prev.z + cur.z);
    normal->y += (prev.z - cur.z) * (prev.x + cur.x);
    normal->z += (prev.x - cur.x) * (prev.y + cur.y);
  }

  if ((norm = VECTORNORM(*normal)) < EPSILON) {
    Warning("PatchNormal", "degenerate patch (id %d)", patch->id);
    return (VECTOR *)NULL;
  }
  VECTORSCALEINVERSE(norm, *normal, *normal);

  return normal;
}


float PatchArea(PATCH *patch)
{
  POINT *p1, *p2, *p3, *p4;
  DVECTOR d1, d2, d3, d4, cp1, cp2, cp3;
  double a, b, c;
  
  
  switch (patch->nrvertices) {
  case 3:
    
    patch->jacobian = (JACOBIAN *)NULL;
		
    p1 = patch->vertex[0]->point;
    p2 = patch->vertex[1]->point;
    p3 = patch->vertex[2]->point;
    VECTORSUBTRACT(*p2, *p1, d1);
    VECTORSUBTRACT(*p3, *p2, d2);
    VECTORCROSSPRODUCT(d1, d2, cp1);
    patch->area = 0.5 * VECTORNORM(cp1);
    break;
  case 4:
    p1 = patch->vertex[0]->point;
    p2 = patch->vertex[1]->point;
    p3 = patch->vertex[2]->point;
    p4 = patch->vertex[3]->point;
    VECTORSUBTRACT(*p2, *p1, d1);
    VECTORSUBTRACT(*p3, *p2, d2);
    VECTORSUBTRACT(*p3, *p4, d3);
    VECTORSUBTRACT(*p4, *p1, d4);
    VECTORCROSSPRODUCT(d1, d4, cp1);
    VECTORCROSSPRODUCT(d1, d3, cp2);
    VECTORCROSSPRODUCT(d2, d4, cp3);
    a = VECTORDOTPRODUCT(cp1, patch->normal);
    b = VECTORDOTPRODUCT(cp2, patch->normal);
    c = VECTORDOTPRODUCT(cp3, patch->normal);
    
    patch->area = a + 0.5 * (b + c);
    if (patch->area < 0.) {	
      b = -b;
      c = -c;
      patch->area = -patch->area;
    }

    
    if (fabs(b)/patch->area < EPSILON && fabs(c)/patch->area < EPSILON) 
      patch->jacobian = (JACOBIAN *)NULL;
    else 
      patch->jacobian = JacobianCreate(a, b, c);
    
    break;
  default:
    Fatal(2, "PatchArea", "Can only handle triangular and quadrilateral patches.\n");
    patch->jacobian = (JACOBIAN *)NULL;
    patch->area = 0.0;
  }

  if (patch->area < EPSILON*EPSILON) {
    fprintf(stderr, "Warning: very small patch id %d area = %g\n", patch->id, patch->area);
  }
  
  return patch->area;
}


POINT *PatchMidpoint(PATCH *patch, POINT *p)
{
  int i;

  VECTORSET(*p, 0, 0, 0);
  for (i=0; i<patch->nrvertices; i++)
    VECTORSUM(*p, *(patch->vertex[i]->point), *p);
  VECTORSCALEINVERSE((float)patch->nrvertices, *p, *p);
  
  return p;
}


float PatchTolerance(PATCH *patch)
{
  int i;
  double tolerance;

  
  tolerance = 0.;
  for (i=0; i<patch->nrvertices; i++) {
    POINT *p = patch->vertex[i]->point;
    double e = fabs(VECTORDOTPRODUCT(patch->normal, *p) + patch->plane_constant) 
             + VECTORTOLERANCE(*p);
    if (e > tolerance)
      tolerance = e;
  }

  return tolerance;
}


#define U_DOMINANT 0x04
#define REVERSE_DIST 0x08


static void PatchInitRayTracing(PATCH *patch)
{
#ifndef LOWMEM_INTERSECT
  int i, j;
  DVEC2D p[4], m;
  float *s = patch->eslope, *d = patch->edist;
  unsigned char *flags = patch->eflags;

  VECTORPROJECT(m, patch->midpoint, patch->index);
  for (i=0; i<patch->nrvertices; i++) {
    VECTORPROJECT(p[i], *patch->vertex[i]->point, patch->index);
    p[i].u -= m.u; p[i].v -= m.v;
  }

  for (i=0, j=1; i<patch->nrvertices; i++, j=(j+1)%patch->nrvertices) {
    double a, b, ss;
    a = p[j].v - p[i].v;
    b = p[i].u - p[j].u;
    flags[i] = 0;

    if (fabs(a) > fabs(b)) {
      flags[i] |= U_DOMINANT;
      if (fabs(a) < patch->tolerance) {
	
	s[i] = 0.;
	d[i] = HUGE;
      } else {
	s[i] = ss = b/a;
	d[i] = (p[i].u + ss * p[i].v + p[j].u + ss * p[j].v) * 0.5;
      }
    } else {
      if (fabs(b) < patch->tolerance) {
	
	s[i] = 0.;
	d[i] = HUGE;
      } else {
	s[i] = ss = a/b;
	d[i] = (ss * p[i].u + p[i].v + ss * p[j].u + p[j].v) * 0.5;
      }
    }

    if (fabs(d[i]) < patch->tolerance)
      
      d[i] = -HUGE;
    else
      
      if (d[i] < 0.) flags[i] |= REVERSE_DIST;
  }
#endif 
}

int IsPatchVirtual(PATCH *patch)
{ 
  return(patch->nrvertices == 0);
}



PATCH *PatchCreateVirtual(void)
{
  PATCH *patch;

  patch = NEWPATCH(); nrelements++;
  patch->twin = (PATCH *)NULL;
  patch->id = patchid; patchid++;
  patch->surface = (SURFACE *)NULL;
  patch->nrvertices = 0;
  patch->vertex[0] = (VERTEX *)NULL;
  patch->vertex[1] = (VERTEX *)NULL;
  patch->vertex[2] = (VERTEX *)NULL;
  patch->vertex[3] = (VERTEX *)NULL;
  patch->brep_data = (BBOX_FACE *)NULL;
  patch->bounds = (float *)NULL;
  VECTORSET(patch->normal, 0.0, 0.0, 0.0);
  patch->jacobian = (JACOBIAN *)NULL;
  patch->area = 0.0;
  VECTORSET(patch->midpoint, 0.0, 0.0, 0.0);;
  patch->plane_constant = 0.0;
  patch->tolerance = 0.0;
  patch->index = 0;
  patch->direct_potential = 0.0;
  RGBSET(patch->color, 0.0, 0.0, 0.0);
  patch->omit = TRUE;
  patch->flags = 0;	

  return patch;
}


PATCH *PatchCreate(int nrvertices, 
		   VERTEX *v1, VERTEX *v2, VERTEX *v3, VERTEX *v4)
{
  PATCH *patch;

  
  if (!v1 || !v2 || !v3 || (nrvertices==4 && !v4))
    return (PATCH *)NULL;	

  
  if (nrvertices != 3 && nrvertices != 4) {
    Error("PatchCreate", "Can only handle quadrilateral or triagular patches");
    return (PATCH *)NULL;
  }

  patch = NEWPATCH(); nrelements++;
  patch->twin = (PATCH *)NULL;
  patch->id = patchid; patchid++;

  patch->surface = (SURFACE *)NULL;

  patch->nrvertices = nrvertices;
  patch->vertex[0] = v1;
  patch->vertex[1] = v2;
  patch->vertex[2] = v3;
  patch->vertex[3] = v4;

  patch->brep_data = (BBOX_FACE *)NULL;

  
  patch->bounds = (float *)NULL;

  
  if (!PatchNormal(patch, &patch->normal)) {
    DISPOSEPATCH(patch); nrelements--;
    return (PATCH *)NULL;
  }

  
  patch->area = PatchArea(patch);

  
  PatchMidpoint(patch, &patch->midpoint);
  
  
  patch->plane_constant = -VECTORDOTPRODUCT(patch->normal, patch->midpoint);

  
  patch->tolerance = PatchTolerance(patch);

  
  patch->index = VectorDominantCoord(&patch->normal);     

  
  PatchInitRayTracing(patch);

  
  PatchConnectVertices(patch);

  patch->direct_potential = 0.0;
  RGBSET(patch->color, 0.0, 0.0, 0.0);

  patch->omit = FALSE;
  patch->flags = 0;	

  
  patch->radiance_data = (Radiance && Radiance->CreatePatchData) ? 
                         Radiance->CreatePatchData(patch) : (void *)NULL;
  
  return patch;
}


void PatchDestroy(PATCH *patch)
{
  if (Radiance && patch->radiance_data)
    if (Radiance->DestroyPatchData) 
      Radiance->DestroyPatchData(patch);

  if (patch->bounds) 
    BoundsDestroy(patch->bounds);

  if (patch->jacobian)
    JacobianDestroy(patch->jacobian);

  if (patch->brep_data)	
    BBoxDestroyFace(patch->brep_data);

  if (patch->twin)
    patch->twin->twin = (PATCH *)NULL;

  DISPOSEPATCH(patch); nrelements--;
}


float *PatchBounds(PATCH *patch, float *bounds)
{
  int i;
	
  if (!patch->bounds) {
    patch->bounds = BoundsCreate();
    BoundsInit(patch->bounds);
    for (i=0; i<patch->nrvertices; i++) 
      BoundsEnlargePoint(patch->bounds, patch->vertex[i]->point);
  }

  BoundsCopy(patch->bounds, bounds);

  return bounds;	
}

static int nrofsamples(PATCH *patch)
{
  int nrsamples = 1;
  if (BsdfIsTextured(patch->surface->material->bsdf)) {
    if (patch->vertex[0]->texCoord == patch->vertex[1]->texCoord &&
	patch->vertex[0]->texCoord == patch->vertex[2]->texCoord &&
	(patch->nrvertices == 3 || patch->vertex[0]->texCoord == patch->vertex[3]->texCoord) &&
	patch->vertex[0]->texCoord != NULL)
      
      nrsamples = 1;
    else
      nrsamples = 100;
  }
  return nrsamples;
}

COLOR PatchAverageNormalAlbedo(PATCH *patch, BSDFFLAGS components)
{
  int i, nrsamples;
  COLOR albedo;
  HITREC hit;
  InitHit(&hit, patch, NULL, &patch->midpoint, &patch->normal, patch->surface->material, 0.);

  nrsamples = nrofsamples(patch);
  COLORCLEAR(albedo);
  for (i=0; i<nrsamples; i++) {
    COLOR sample;
    unsigned *xi = Nied31(i);
    hit.uv.u = (double)xi[0] * RECIP;
    hit.uv.v = (double)xi[1] * RECIP;
    hit.flags |= HIT_UV;
    PatchPoint(patch, hit.uv.u, hit.uv.v, &hit.point);
    sample = BsdfScatteredPower(patch->surface->material->bsdf, &hit, &patch->normal, components);
    COLORADD(albedo, sample, albedo);
  }
  COLORSCALEINVERSE((float)nrsamples, albedo, albedo);

  return albedo;
}

COLOR PatchAverageEmittance(PATCH *patch, XXDFFLAGS components)
{
  int i, nrsamples;
  COLOR emittance;
  HITREC hit;
  InitHit(&hit, patch, NULL, &patch->midpoint, &patch->normal, patch->surface->material, 0.);
  
  nrsamples = nrofsamples(patch);
  COLORCLEAR(emittance);
  for (i=0; i<nrsamples; i++) {
    COLOR sample;
    unsigned *xi = Nied31(i);
    hit.uv.u = (double)xi[0] * RECIP;
    hit.uv.v = (double)xi[1] * RECIP;
    hit.flags |= HIT_UV;
    PatchPoint(patch, hit.uv.u, hit.uv.v, &hit.point);
    sample = EdfEmittance(patch->surface->material->edf, &hit, components);
    COLORADD(emittance, sample, emittance);
  }
  COLORSCALEINVERSE((float)nrsamples, emittance, emittance);

  return emittance;
}

void PatchPrintID(FILE *out, PATCH *patch)
{
  fprintf(out, "%d ", patch->id);
}

void PatchPrint(FILE *out, PATCH *patch)
{
  int i;
  COLOR Rd, Ed;

  fprintf(out, "Patch id %d:\n", patch->id);

  fprintf(out, "%d vertices:\n", patch->nrvertices); 
  for (i=0; i<patch->nrvertices; i++)
    VertexPrint(out, patch->vertex[i]);
  fprintf(out, "\n");

  fprintf(out, "midpoint = "); VectorPrint(out, patch->midpoint);
  fprintf(out, ", normal = "); VectorPrint(out, patch->normal);
  fprintf(out, ", plane constant = %g, tolerance = %g\narea = %g, ", 
	  patch->plane_constant, patch->tolerance, patch->area);
  if (patch->jacobian) 
    fprintf(out, "Jacobian: %g %+g*u %+g*v \n",
	    patch->jacobian->A, patch->jacobian->B, patch->jacobian->C);
  else
    fprintf(out, "No explicitely stored jacobian\n");

  fprintf(out, "sided = %d, material = '%s'\n", 
	  patch->surface->material->sided,
	  patch->surface->material->name ? patch->surface->material->name : "(default)");
  COLORCLEAR(Rd);
  if (patch->surface->material->bsdf)
    Rd = PatchAverageNormalAlbedo(patch, BRDF_DIFFUSE_COMPONENT);
  COLORCLEAR(Ed);
  if (patch->surface->material->edf) {
    Ed = PatchAverageEmittance(patch, DIFFUSE_COMPONENT);
  }
  fprintf(out, ", reflectance = "); ColorPrint(out, Rd);
  fprintf(out, ", self-emitted luminosity = %g\n", ColorLuminance(Ed));

  fprintf(out, "color: "); RGBPrint(out, patch->color); fprintf(out, "\n");
  fprintf(out, "directly received potential: %g\n", patch->direct_potential);

  fprintf(out, "flags: %s\n",
	  PATCH_IS_VISIBLE(patch) ? "PATCH_IS_VISIBLE" : "");

  if (Radiance) {
    if (patch->radiance_data && Radiance->PrintPatchData) {
      fprintf(out, "Radiance data:\n");
      Radiance->PrintPatchData(out, patch);
    } else
      fprintf(out, "No radiance data\n");
  }

#ifndef LOWMEM_INTERSECT
  fprintf(out, "RayTracing data:\n");
  for (i=0; i<patch->nrvertices; i++) {
    fprintf(stderr, "edge %d: slope = %g, dist = %g, flags = %02x\n",
	    i, patch->eslope[i], patch->edist[i], patch->eflags[i]);
  }
#endif
}


#define MAX_EXCLUDED_PATCHES 4
static PATCH *excludedPatches[MAX_EXCLUDED_PATCHES] = {NULL, NULL, NULL, NULL};

void PatchDontIntersect(int n, ...)
{
  va_list ap;
  int i;

  if (n>MAX_EXCLUDED_PATCHES) {
    Fatal(-1, "PatchDontIntersect", "Too many patches to exclude from intersection tests (maximum is %d)", MAX_EXCLUDED_PATCHES);
    return;
  }

  va_start(ap, n);
  for (i=0; i<n; i++) {
    excludedPatches[i] = va_arg(ap, PATCH *);
  }
  va_end(ap);

  while (i<MAX_EXCLUDED_PATCHES)
    excludedPatches[i++] = NULL;
}

int IsExcludedPatch(PATCH *p)
{
  PATCH **excl = excludedPatches;
  
  return (*excl == p || *++excl == p || *++excl == p || *++excl == p);
}

static int AllVerticesHaveANormal(PATCH *patch)
{
  int i;

  for (i=0; i<patch->nrvertices; i++) {
    if (!patch->vertex[i]->normal)
      break;
  }
  return i>=patch->nrvertices;
}


static VECTOR GetInterpolatedNormalAtUV(PATCH *patch, double u, double v)
{
  VECTOR normal, *v1, *v2, *v3, *v4;

  v1 = patch->vertex[0]->normal;
  v2 = patch->vertex[1]->normal;
  v3 = patch->vertex[2]->normal;

  switch (patch->nrvertices) {
  case 3:
    PINT(*v1, *v2, *v3, u, v, normal);
    break;
  case 4:
    v4 = patch->vertex[3]->normal;
    PINQ(*v1, *v2, *v3, *v4, u, v, normal);
    break;
  default:
    Fatal(-1, "PatchNormalAtUV", "Invalid number of vertices %d", patch->nrvertices);
  }

  VECTORNORMALIZE(normal);
  return normal;
}


VECTOR PatchInterpolatedNormalAtUV(PATCH *patch, double u, double v)
{
  if (!AllVerticesHaveANormal(patch))
    return patch->normal;
  return GetInterpolatedNormalAtUV(patch, u, v);
}


VECTOR PatchInterpolatedNormalAtPoint(PATCH *patch, POINT *point)
{
  double u, v;

  if (!AllVerticesHaveANormal(patch))
    return patch->normal;

  PatchUV(patch, point, &u, &v);
  return GetInterpolatedNormalAtUV(patch, u, v);
}



void PatchInterpolatedFrameAtUV(PATCH *patch, double u, double v,
			     VECTOR *X, VECTOR *Y, VECTOR *Z)
{
  *Z = PatchInterpolatedNormalAtUV(patch, u, v);

  

  if (X && Y) { 
    double zz = sqrt(1 - Z->z*Z->z);
    if(zz < EPSILON)
    {
      VECTORSET(*X, 1., 0., 0.);
    }
    else
    {
      VECTORSET(*X, Z->y/zz, -Z->x/zz, 0.);
    }
    
    VECTORCROSSPRODUCT(*Z, *X, *Y);                      
  }
#ifdef NEVER
  if(!VectorFrame(Z, patch->index, X, Y))
  {
    
    static int already_printed = FALSE;
    if(!already_printed)
    {
      Warning("PatchInterpolatedFrameAtUV", 
	      "Large differences in vertex normals detected");
      already_printed = TRUE;
    }

    VectorFrame(Z, (patch->index + 1) % 3, X, Y);
  }
#endif
}



void PatchInterpolatedFrameAtPoint(PATCH *patch, POINT *point, VECTOR *X,
			      VECTOR *Y, VECTOR *Z)
{
  double u, v;

  if (!AllVerticesHaveANormal(patch))
    u=v=0.;
  else
    PatchUV(patch, point, &u, &v);

  PatchInterpolatedFrameAtUV(patch, u, v, X, Y, Z);
}

VECTOR PatchTextureCoordAtUV(PATCH *patch, double u, double v)
{
  VECTOR *t0, *t1, *t2, *t3;
  VECTOR texCoord;
  VECTORSET(texCoord, 0., 0., 0.);

  t0 = patch->vertex[0]->texCoord;
  t1 = patch->vertex[1]->texCoord;
  t2 = patch->vertex[2]->texCoord;
  switch (patch->nrvertices) {
  case 3:
    if (!t0 || !t1 || !t2) {
      VECTORSET(texCoord, u, v, 0.);
    } else {
      PINT(*t0, *t1, *t2, u, v, texCoord);
    }
    break;
  case 4:
    t3 = patch->vertex[3]->texCoord;
    if (!t0 || !t1 || !t2 || !t3) {
      VECTORSET(texCoord, u, v, 0.);
    } else {
      PINQ(*t0, *t1, *t2, *t3, u, v, texCoord);
    }
    break;
  default:
    Fatal(-1, "PatchTextureCoordAtUV", "Invalid nr of vertices %d", patch->nrvertices);
  }
  return texCoord;
}

#ifdef IDEBUG
int idebug = 0;
#endif

void PatchIntersectStartDebug(void)
{
#ifdef IDEBUG
  idebug = TRUE;
#endif
}

void PatchIntersectEndDebug(void)
{
#ifdef IDEBUG
  idebug = FALSE;
#endif
}


static void OldTriangleUV(PATCH *poly, VECTOR *point, DVEC2D *uv)
{
  static PATCH *cached_poly;
  DVECTOR d0, d1, d2, cu, cv;

  if (!poly)
    poly = cached_poly;

  
  VECTORSUBTRACT(*poly->vertex[0]->point, *point, d0);
  VECTORSUBTRACT(*poly->vertex[1]->point, *point, d1);
  VECTORSUBTRACT(*poly->vertex[2]->point, *point, d2);
  VECTORCROSSPRODUCT(d2, d0, cu);
  VECTORCROSSPRODUCT(d0, d1, cv);
  uv->u = 0.5 * VECTORNORM(cu) / poly->area;
  uv->v = 0.5 * VECTORNORM(cv) / poly->area;
}


#define QUADUV_CLIPBOUNDS
#define DISTANCE_TO_UNIT_INTERVAL(x)	((x < 0.) ? -x : ((x > 1.) ? (x-1.) : 0.))
#define CLIP_TO_UNIT_INTERVAL(x)	((x < EPSILON) ? EPSILON : ((x > (1.-EPSILON)) ? (1.-EPSILON) : x))

static void OldQuadUV(PATCH *poly, VECTOR *point, DVEC2D *uv)
{
  static double Du0, Du1, Du2, Dux, Duy, Dv0, Dv1, Dv2, Dvx, Dvy;
  static DVECTOR Na, Nb, Nc, Qux, Quy, Qvx, Qvy;
  static PATCH *lastpoly = (PATCH *)NULL;
  DVECTOR Pa, Pb, Pc, Pd;

  
  if (!poly) poly = lastpoly;

  if (!poly->jacobian) {
    static VEC2D v[4];
    static double du2, dv2;
    VEC2D p;

    if (1 || poly != lastpoly) {
      switch (poly->index) {
      case XNORMAL:
	v[0].u = poly->vertex[0]->point->y; v[0].v = poly->vertex[0]->point->z;
	v[1].u = poly->vertex[1]->point->y; v[1].v = poly->vertex[1]->point->z;
	v[3].u = poly->vertex[3]->point->y; v[3].v = poly->vertex[3]->point->z;
	break;
      case YNORMAL:
	v[0].u = poly->vertex[0]->point->x; v[0].v = poly->vertex[0]->point->z;
	v[1].u = poly->vertex[1]->point->x; v[1].v = poly->vertex[1]->point->z;
	v[3].u = poly->vertex[3]->point->x; v[3].v = poly->vertex[3]->point->z;
	break;
      case ZNORMAL:
	v[0].u = poly->vertex[0]->point->x; v[0].v = poly->vertex[0]->point->y;
	v[1].u = poly->vertex[1]->point->x; v[1].v = poly->vertex[1]->point->y;
	v[3].u = poly->vertex[3]->point->x; v[3].v = poly->vertex[3]->point->y;
	break;
      }

      

      v[1].u -= v[0].u; v[3].u -= v[0].u;
      v[1].v -= v[0].v; v[3].v -= v[0].v;

      du2 = (v[1].v * v[3].u - v[1].u * v[3].v);
      
      dv2 = -du2;

      if (du2 < EPSILON && du2 > -EPSILON)
	printf("du2 == 0 !\n");
	
      if (dv2 < EPSILON && dv2 > -EPSILON) 
	printf("dv2 == 0 !\n");

      lastpoly = poly;
    }

    

    VECTORPROJECT(p, *point, poly->index);

    

    p.u -= v[0].u;  p.v -= v[0].v;

    uv->u = uv->v = 0.0;

    if (du2 > EPSILON || du2 < -EPSILON)
      uv->u = (p.v * v[3].u - p.u * v[3].v) / du2;
    if (dv2 > EPSILON || dv2 < -EPSILON)
      uv->v = (p.v * v[1].u - p.u * v[1].v) / dv2;

#ifdef QUADUV_CLIPBOUNDS
    uv->u = CLIP_TO_UNIT_INTERVAL(uv->u);
    uv->v = CLIP_TO_UNIT_INTERVAL(uv->v);
#endif
    return;
  }

  
  if (poly != lastpoly) {
    VECTORCOPY(*poly->vertex[0]->point, Pd);
    VECTORSUBTRACT(*poly->vertex[1]->point, *poly->vertex[0]->point, Pb);
    VECTORSUBTRACT(*poly->vertex[3]->point, *poly->vertex[0]->point, Pc);
    VECTORSUBTRACT(*poly->vertex[2]->point, *poly->vertex[3]->point, Pa);
    VECTORSUBTRACT(Pa, Pb, Pa);
    
    VECTORCROSSPRODUCT(Pa, poly->normal, Na);
    VECTORCROSSPRODUCT(Pb, poly->normal, Nb);
    VECTORCROSSPRODUCT(Pc, poly->normal, Nc);

    Du0 = VECTORDOTPRODUCT(Nc, Pd);
    Du1 = VECTORDOTPRODUCT(Na, Pd) + VECTORDOTPRODUCT(Nc, Pb);
    Du2 = VECTORDOTPRODUCT(Na, Pb);
    
    if (fabs(Du2) > EPSILON) {		
      VECTORSCALEINVERSE(2.*Du2, Na, Qux);
      Dux = - Du1 / (2.*Du2);
      VECTORSCALEINVERSE(-Du2, Nc, Quy);
      Duy = Du0 / Du2;
    }

    Dv0 = VECTORDOTPRODUCT(Nb, Pd);
    Dv1 = VECTORDOTPRODUCT(Na, Pd) + VECTORDOTPRODUCT(Nb, Pc);
    Dv2 = VECTORDOTPRODUCT(Na, Pc);
    
    if (fabs(Dv2) > EPSILON) {		
      VECTORSCALEINVERSE(2.*Dv2, Na, Qvx);
      Dvx = - Dv1 / (2.*Dv2);
      VECTORSCALEINVERSE(-Dv2, Nb, Qvy);
      Dvy = Dv0 / Dv2;
    }

    lastpoly = poly;
  }

  if (fabs(Du2) < EPSILON) {		
    double B, C;
    
    B = Du1 - VECTORDOTPRODUCT(Na, *point);
    if (fabs(B) < EPSILON)
      uv->u = 0.0;		
    else {
      C = VECTORDOTPRODUCT(Nc, *point) - Du0;
      uv->u = C/B;
    }
  } else {				
    double Ka, Kb, D;
    
    Ka = Dux + VECTORDOTPRODUCT(Qux, *point);
    Kb = Duy + VECTORDOTPRODUCT(Quy, *point);
    D = sqrt(Ka*Ka - Kb);
    uv->u = (Ka > D) ? (Ka - D) : (Ka + D); 	

    if (uv->u > 1.) {  			
      double u1 = Ka - D, u2 = Ka + D;
      double d1 = DISTANCE_TO_UNIT_INTERVAL(u1);
      double d2 = DISTANCE_TO_UNIT_INTERVAL(u2);
      uv->u = (d1 < d2) ? u1 : u2;	
    }
  }

  
  if (fabs(Dv2) < EPSILON) {		
    double B, C;
    
    B = Dv1 - VECTORDOTPRODUCT(Na, *point);
    if (fabs(B) < EPSILON)
      uv->v = 0.0;		
    else {
      C = VECTORDOTPRODUCT(Nb, *point) - Dv0;
      uv->v = C/B;
    }
  } else {				
    double Ka, Kb, D;
    
    Ka = Dvx + VECTORDOTPRODUCT(Qvx, *point);
    Kb = Dvy + VECTORDOTPRODUCT(Qvy, *point);
    D = sqrt(Ka*Ka - Kb);
    uv->v = (Ka > D) ? (Ka - D) : (Ka + D);	

    if (uv->v > 1.) {  			
      double v1 = Ka - D, v2 = Ka + D;
      double d1 = DISTANCE_TO_UNIT_INTERVAL(v1);
      double d2 = DISTANCE_TO_UNIT_INTERVAL(v2);
      uv->v = (d1 < d2) ? v1 : v2;	
    }
  }

#ifdef QUADUV_CLIPBOUNDS
  uv->u = CLIP_TO_UNIT_INTERVAL(uv->u);
  uv->v = CLIP_TO_UNIT_INTERVAL(uv->v);
#endif
}



#define REAL double    

#define V2Set(V, a, b) { (V).u = (a) ; (V).v = (b) ; }
#define V2Sub(p, q, r) { (r).u = (p).u - (q).u; (r).v = (p).v - (q).v; }
#define V2Add(p, q, r) { (r).u = (p).u + (q).u; (r).v = (p).v + (q).v; }
#define V2Negate(p)    { (p).u = -(p).u;  (p).v = -(p).v; }
#define DETERMINANT(A,B)    (((REAL)(A).u * (REAL)(B).v - (REAL)(A).v * (REAL)(B).u))
#define ABS(A) ((A)<0. ? -(A) : (A))



static int TriangleUV(PATCH *patch, VECTOR *point, DVEC2D *uv)
{
  static PATCH *cachedpatch = (PATCH *)NULL;
  double u0, v0;
  REAL alpha=-1., beta=1.;
  VERTEX **v;
  DVEC2D p0, p1, p2;

  if (!patch) patch = cachedpatch;
  cachedpatch = patch;

  
  v = patch->vertex;
  switch (patch->index) {
  case XNORMAL:
         u0 = (*v)->point->y; v0 = (*v)->point->z;
    V2Set(p0,       point->y - u0,       point->z - v0); v++;
    V2Set(p1, (*v)->point->y - u0, (*v)->point->z - v0); v++;
    V2Set(p2, (*v)->point->y - u0, (*v)->point->z - v0);
    break;

  case YNORMAL:
         u0 = (*v)->point->x; v0 = (*v)->point->z;
    V2Set(p0,       point->x - u0,       point->z - v0); v++;
    V2Set(p1, (*v)->point->x - u0, (*v)->point->z - v0); v++;
    V2Set(p2, (*v)->point->x - u0, (*v)->point->z - v0);
    break;

  case ZNORMAL:
         u0 = (*v)->point->x; v0 = (*v)->point->y;
    V2Set(p0,       point->x - u0,       point->y - v0); v++;
    V2Set(p1, (*v)->point->x - u0, (*v)->point->y - v0); v++;
    V2Set(p2, (*v)->point->x - u0, (*v)->point->y - v0);
    break;
  }

  if (p1.u < -EPSILON || p1.u > EPSILON) {  
    beta = (p0.v*p1.u - p0.u*p1.v) / (p2.v*p1.u - p2.u*p1.v);
    if (beta >= 0. && beta <= 1.)
      alpha = (p0.u - beta * p2.u) / p1.u;
    else {
#ifdef IDEBUG
      if (idebug) {
	fprintf(stderr, "%s %d: point-in-polygon test fails (alpha=%f, beta=%f)\n",
		__FILE__, __LINE__, alpha, beta);
      }
#endif
      return FALSE;
    }
  } else {
    beta = p0.u / p2.u;
    if (beta >= 0. && beta <= 1.)
      alpha = (p0.v - beta * p2.v) / p1.v;
    else {
#ifdef IDEBUG
      if (idebug) {
	fprintf(stderr, "%s %d: point-in-polygon test fails (alpha=%f, beta=%f)\n",
		__FILE__, __LINE__, alpha, beta);
      }
#endif
      return FALSE;
    }
  }
  uv->u = alpha; uv->v = beta;
  if (alpha < 0. || (alpha + beta) > 1.) {
#ifdef IDEBUG
    if (idebug) {
      fprintf(stderr, "%s %d: point-in-polygon test fails (alpha=%f, beta=%f)\n",
	      __FILE__, __LINE__, alpha, beta);
    }
#endif
    return FALSE;
  }
  return TRUE;
}


static int QuadUV(PATCH *patch, VECTOR *point, DVEC2D *uv)
{
  static PATCH *cachedpatch = (PATCH *)NULL;
  VERTEX **p;
  DVEC2D    A, B, C, D;                   
  DVEC2D    M;                            
  DVEC2D    AB, BC, CD, AD, AM, AE;       
  REAL     u=-1., v=-1.;                 
  REAL     a, b, c, SqrtDelta;           
  DVEC2D    Vector;                       
  int      IsInside = FALSE;

  if (!patch) patch = cachedpatch;
  cachedpatch = patch;

  
  p = patch->vertex;
  switch (patch->index) {
  case XNORMAL:
    V2Set(A, (*p)->point->y, (*p)->point->z); p++;
    V2Set(B, (*p)->point->y, (*p)->point->z); p++;
    V2Set(C, (*p)->point->y, (*p)->point->z); p++;
    V2Set(D, (*p)->point->y, (*p)->point->z);
    V2Set(M,       point->y,       point->z);
    break;

  case YNORMAL:
    V2Set(A, (*p)->point->x, (*p)->point->z); p++;
    V2Set(B, (*p)->point->x, (*p)->point->z); p++;
    V2Set(C, (*p)->point->x, (*p)->point->z); p++;
    V2Set(D, (*p)->point->x, (*p)->point->z);
    V2Set(M,       point->x,       point->z);
    break;

  case ZNORMAL:
    V2Set(A, (*p)->point->x, (*p)->point->y); p++;
    V2Set(B, (*p)->point->x, (*p)->point->y); p++;
    V2Set(C, (*p)->point->x, (*p)->point->y); p++;
    V2Set(D, (*p)->point->x, (*p)->point->y);
    V2Set(M,       point->x,       point->y);
    break;
  }

  V2Sub (B, A, AB); V2Sub (C, B, BC);
  V2Sub (D, C, CD); V2Sub (D, A, AD);
  V2Add (CD, AB, AE); V2Negate (AE); V2Sub (M, A, AM);
  
  if (fabs(DETERMINANT(AB, CD)) < EPSILON)               
    {
      V2Sub (AB, CD, Vector);
      v = DETERMINANT(AM, Vector) / DETERMINANT(AD, Vector);
      if ((v >= 0.0) && (v <= 1.0)) {
	b = DETERMINANT(AB, AD) - DETERMINANT(AM, AE);
	c = DETERMINANT (AM, AD);
	u = ABS(b) < EPSILON ? -1 : c/b;
	IsInside = ((u >= 0.0) && (u <= 1.0));
      }
#ifdef IDEBUG
      if (idebug && !IsInside) {
	fprintf(stderr, "%s %d: point-in-polygon test fails (u=%f, v=%f)\n",
		__FILE__, __LINE__, u, v);
      }
#endif
    }
  else if (fabs(DETERMINANT(BC, AD)) < EPSILON)          
    {
      V2Add (AD, BC, Vector);
      u = DETERMINANT(AM, Vector) / DETERMINANT(AB, Vector);
      if ((u >= 0.0) && (u <= 1.0)) {
	b = DETERMINANT(AD, AB) - DETERMINANT(AM, AE);
	c = DETERMINANT (AM, AB);
	v = ABS(b) < EPSILON ? -1 : c/b;
	IsInside = ((v >= 0.0) && (v <= 1.0));
      }
#ifdef IDEBUG
      if (idebug && !IsInside) {
	fprintf(stderr, "%s %d: point-in-polygon test fails (u=%f, v=%f)\n",
		__FILE__, __LINE__, u, v);
      }
#endif
    }
  else                                                   
    {
      a = DETERMINANT(AB, AE); c = - DETERMINANT (AM,AD);
      b = DETERMINANT(AB, AD) - DETERMINANT(AM, AE);
      a = -0.5/a; b *= a; c *= (a + a); SqrtDelta = b*b + c;
      if (SqrtDelta >= 0.0) {
	SqrtDelta = sqrt(SqrtDelta);
	u = b - SqrtDelta;
	if ((u < 0.0) || (u > 1.0))      
	  u = b + SqrtDelta;
	if ((u >= 0.0) && (u <= 1.0)) {
	  v = AD.u + u * AE.u;
	  if (ABS(v) < EPSILON)
	    v = (AM.v - u * AB.v) / (AD.v + u * AE.v);
	  else
	    v = (AM.u - u * AB.u) / v;
	  IsInside = ((v >= 0.0) && (v <= 1.0));
	}
#ifdef IDEBUG
	if (idebug && !IsInside) {
	  fprintf(stderr, "%s %d: point-in-polygon test fails (u=%f, v=%f)\n",
		  __FILE__, __LINE__, u, v);
	}
#endif
      } else {
	u = v = -1.;
#ifdef IDEBUG
	if (idebug && !IsInside) {
	  fprintf(stderr, "%s %d: point-in-polygon test fails (u=%f, v=%f)\n",
		  __FILE__, __LINE__, u, v);
	}
#endif
      }
    }

  uv->u = u;
  uv->v = v;
#ifdef QUADUV_CLIPBOUNDS
  uv->u = CLIP_TO_UNIT_INTERVAL(uv->u);
  uv->v = CLIP_TO_UNIT_INTERVAL(uv->v);
#endif

  return IsInside;
}

#ifndef LOWMEM_INTERSECT

static int FastPointInPatch(POINT *point, PATCH *patch)
{
  float u=0., v=0., t;
  float *s = patch->eslope, *d = patch->edist;
  unsigned char *flags = patch->eflags;

  switch (patch->index) {
  case XNORMAL:
    u = point->y - patch->midpoint.y;
    v = point->z - patch->midpoint.z;
    break;
  case YNORMAL:
    u = point->x - patch->midpoint.x;
    v = point->z - patch->midpoint.z;
    break;
  case ZNORMAL:
    u = point->x - patch->midpoint.x;
    v = point->y - patch->midpoint.y;
    break;
  }

  t = ((*flags) & U_DOMINANT) ? (u + (*s) * v) : ((*s) * u + v);
  if (((*flags) & REVERSE_DIST) ? (t < (*d)) : (t > (*d))) return FALSE;

  t = ((*++flags) & U_DOMINANT) ? (u + (*++s) * v) : ((*++s) * u + v);
  if (((*flags) & REVERSE_DIST) ? (t < (*++d)) : (t > (*++d))) return FALSE;

  t = ((*++flags) & U_DOMINANT) ? (u + (*++s) * v) : ((*++s) * u + v);
  if (((*flags) & REVERSE_DIST) ? (t < (*++d)) : (t > (*++d))) return FALSE;

  if (patch->nrvertices <= 3) return TRUE;
  t = ((*++flags) & U_DOMINANT) ? (u + (*++s) * v) : ((*++s) * u + v);
  if (((*flags) & REVERSE_DIST) ? (t < (*++d)) : (t > (*++d))) return FALSE;

  return TRUE;
}
#endif 

#ifdef CHECK_INTERSECT


static int HitInPatch(HITREC *hit, PATCH *patch)
{
  int inside, oldinside, problem = FALSE;
  DVEC2D olduv;

  if (patch->nrvertices == 3) {
    inside = TriangleUV(patch, &hit->point, &hit->uv);
#ifdef NEVER
    if (inside) {
      OldTriangleUV(patch, &hit->point, &olduv);
      if (!FLOATEQUAL(hit->uv.u, olduv.u, 1e-4) ||
	  !FLOATEQUAL(hit->uv.v, olduv.v, 1e-4)) {
	fprintf(stderr, "%s %d: WARNING: hit->uv.u = %f != olduv.u = %f,  hit->uv.v = %f != olduv.v = %f\n", __FILE__, __LINE__, hit->uv.u, olduv.u, hit->uv.v, olduv.v); 
	problem = TRUE;
      }
    }
#endif
  } else {
    inside = QuadUV(patch, &hit->point, &hit->uv);
#ifdef NEVER
    if (inside) {
      OldQuadUV(patch, &hit->point, &olduv);
      if (!FLOATEQUAL(hit->uv.u, olduv.u, 1e-4) ||
	  !FLOATEQUAL(hit->uv.v, olduv.v, 1e-4)) {
	fprintf(stderr, "%s %d: WARNING: hit->uv.u = %f != olduv.u = %f,  hit->uv.v = %f != olduv.v = %f\n", __FILE__, __LINE__, hit->uv.u, olduv.u, hit->uv.v, olduv.v); 
	problem = TRUE;
      }
    }
#endif
  }

  oldinside = FastPointInPatch(&hit->point, patch);
  if (inside != oldinside) {
    fprintf(stderr, "%s %d: WARNING: inside = %d != oldinside =%d\n", __FILE__, __LINE__, inside, oldinside);
    problem = TRUE;
  }

  if (problem) {
    VECTOR newpoint, oldpoint;
    fprintf(stderr, "point = "); VectorPrint(stderr, hit->point); fprintf(stderr, "\n");
    fprintf(stderr, "uv = %f,%f\n", hit->uv.u, hit->uv.v);
    PatchPoint(patch, hit->uv.u, hit->uv.v, &newpoint);
    fprintf(stderr, "new point = "); VectorPrint(stderr, newpoint); fprintf(stderr, "\n");
    
    fprintf(stderr, "patch id = %d ", patch->id);
    fprintf(stderr, "\n");
  }

  return inside;
}

#else 

#ifndef LOWMEM_INTERSECT

static int HitInPatch(HITREC *hit, PATCH *patch)
{
  return FastPointInPatch(&hit->point, patch);
}

#else 

static int HitInPatch(HITREC *hit, PATCH *patch)
{
  hit->flags |= HIT_UV;		
  return (patch->nrvertices == 3)
    ? TriangleUV(patch, &hit->point, &hit->uv)
    : QuadUV(patch, &hit->point, &hit->uv);
}

#endif 
#endif 


HITREC *PatchIntersect(PATCH *patch, RAY *ray, float mindist, float *maxdist, int hitflags, HITREC *hitstore)
{
  HITREC hit;
  float dist;

#ifdef IDEBUG
  if (idebug) {
    fprintf(stderr, "=====> PatchIntersect:\n");
    fprintf(stderr, "%s %d: mindist = %f, *maxdist = %f, patch = ", __FILE__, __LINE__, mindist, *maxdist);
    PatchPrint(stderr, patch);
  }
#endif

  if (IsExcludedPatch(patch)) {
#ifdef IDEBUG
    if (idebug)
      fprintf(stderr, "%s %d: excluded patch -> no intersection\n", __FILE__, __LINE__);
#endif
    return (HITREC *)NULL;
  }

  if ((dist = VECTORDOTPRODUCT(patch->normal, ray->dir)) > EPSILON) {
    
    if (!(hitflags & HIT_BACK)) {
#ifdef IDEBUG
      if (idebug)
	fprintf(stderr, "%s %d: wrong side -> no intersection\n", __FILE__, __LINE__);
#endif
      return (HITREC *)NULL;
    } else
      hit.flags = HIT_BACK;
  } else if (dist < -EPSILON){
    
    if (!(hitflags & HIT_FRONT)) {
#ifdef IDEBUG
      if (idebug)
	fprintf(stderr, "%s %d: wrong side -> no intersection\n", __FILE__, __LINE__);
#endif
      return (HITREC *)NULL;
    } else
      hit.flags = HIT_FRONT;
  } else {
    
#ifdef IDEBUG
    if (idebug)
      fprintf(stderr, "%s %d: parallel ray -> no intersection\n", __FILE__, __LINE__);
#endif
    return (HITREC *)NULL;
  }

  dist = - (VECTORDOTPRODUCT(patch->normal, ray->pos) + patch->plane_constant) / dist;
#ifdef IDEBUG
  if (idebug)
    fprintf(stderr, "%s %d: dist = %f\n", __FILE__, __LINE__, dist);
#endif

  if (dist > *maxdist || dist < mindist) {
    
#ifdef IDEBUG
    if (idebug)
      fprintf(stderr, "%s %d: too far or too close -> no intersection\n", __FILE__, __LINE__);
#endif
    return (HITREC *)NULL;
  }

  
  hit.dist = dist;
  VECTORADDSCALED(ray->pos, dist, ray->dir, hit.point);

#ifdef IDEBUG
  if (idebug) {
    fprintf(stderr, "%s %d: intersection point = ", __FILE__, __LINE__);
    fprintf(stderr, "%f %f %f", hit.point.x, hit.point.y, hit.point.z);
    fprintf(stderr, "\n");
  }
#endif
  
  if (HitInPatch(&hit, patch)) {
    hit.geom = (GEOM *)NULL;	
    hit.patch = patch;
    hit.material = patch->surface->material;
    hit.gnormal = patch->normal;
    hit.flags |= HIT_PATCH | HIT_POINT | HIT_MATERIAL | HIT_GNORMAL | HIT_DIST;
    if (hitflags & HIT_UV) {
      if (!(hit.flags & HIT_UV)) {
	PatchUV(hit.patch, &hit.point, &hit.uv.u, &hit.uv.v);
	hit.flags &= HIT_UV;
      }
    }
    *hitstore = hit;
    *maxdist = dist;

#ifdef IDEBUG
    if (idebug) {
      fprintf(stderr, "%s %d: -> intersection, new maxdist = %f\n", __FILE__, __LINE__, *maxdist);
    }
#endif
    return hitstore;
  }

#ifdef IDEBUG
  if (idebug)
    fprintf(stderr, "%s %d: point-in-polygon test fails -> no intersection\n", __FILE__, __LINE__);
#endif
  return (HITREC *)NULL;
}


void BilinearToUniform(PATCH *patch, double *u, double *v)
{
  double a=patch->jacobian->A, b=patch->jacobian->B, c=patch->jacobian->C;
  *u = ((a + 0.5*c) + 0.5*b*(*u)) * (*u) / patch->area;
  *v = ((a + 0.5*b) + 0.5*c*(*v)) * (*v) / patch->area;
}


static int SolveQuadraticUnitInterval(double A, double B, double C, double *x)
{
  double D = B*B - 4.*A*C, x1, x2;
#define TOLERANCE 1e-5

  if (A<TOLERANCE && A>-TOLERANCE) {
    
    x1 = -1.;
    x2 = -C / B;
  } else {
    if (D < -TOLERANCE*TOLERANCE) {
      *x = -B / (2. * A);
      Error(NULL, "Bilinear->Uniform mapping has negative discriminant D = %g.\nTaking 0 as discriminant and %g as solution.", D, *x);
      return FALSE;
    }

    D = D > TOLERANCE*TOLERANCE ? sqrt(D) : 0.;
    A = 1. / (2. * A);
    x1 = (-B + D) * A;
    x2 = (-B - D) * A;
  
    if (x1 > -TOLERANCE && x1 < 1.+TOLERANCE) {
      *x = x1;
      if (x2 > -TOLERANCE && x2 < 1.+TOLERANCE) {
	
	return FALSE;
      } else
	return TRUE;
    }
  }

  if (x2 > -TOLERANCE && x2 < 1.+TOLERANCE) {
    *x = x2;
    return TRUE;
  } else {
    double d;

    
    
    if (x1>1.) 
      d = x1-1.;
    else  
      d = -x1;
    *x = x1;
    if (x2>1.) {
      if (x2-1. < d) *x = x2; 
    } else {
      if (0.-x2 < d) *x = x2;
    }

    
    if (*x < 0.) *x=0.;
    if (*x > 1.) *x=1.;
    
    return FALSE;
  }

  
}


void UniformToBilinear(PATCH *patch, double *u, double *v)
{
  double a=patch->jacobian->A, b=patch->jacobian->B, c=patch->jacobian->C, A, B, C;

  A = 0.5*b/patch->area; B = (a+0.5*c)/patch->area; C = -(*u);
  if (!SolveQuadraticUnitInterval(A, B, C, u)) {
    
  }

  A = 0.5*c/patch->area; B = (a+0.5*b)/patch->area; C = -(*v);
  if (!SolveQuadraticUnitInterval(A, B, C, v)) {
    
  }
}


VECTOR *PatchPoint(PATCH *patch, double u, double v, VECTOR *point)
{
  VECTOR *v1, *v2, *v3, *v4;

  if(IsPatchVirtual(patch)) return(NULL);

  v1 = patch->vertex[0]->point;
  v2 = patch->vertex[1]->point;
  v3 = patch->vertex[2]->point;

  if (patch->nrvertices == 3) {
    if (u+v > 1.) {
      u=1.-u; v=1.-v;
      
    }
    PINT(*v1, *v2, *v3, u, v, *point);
  } else if (patch->nrvertices == 4) {
    v4 = patch->vertex[3]->point;
    PINQ(*v1, *v2, *v3, *v4, u, v, *point);
  } else
    Fatal(4, "PatchPoint", "Can only handle triangular or quadrilateral patches");
  
  return point;
}


VECTOR *PatchUniformPoint(PATCH *patch, double u, double v, VECTOR *point)
{
  if (patch->jacobian)
    UniformToBilinear(patch, &u, &v);
  return PatchPoint(patch, u, v, point);
}


int PatchUV(PATCH *poly, VECTOR *point, double *u, double *v)
{
  static PATCH *cached;
  PATCH *thepoly;
  DVEC2D uv;
  int inside = FALSE;

  thepoly = poly ? poly : cached;
  cached = thepoly;

  switch (thepoly->nrvertices) {
  case 3:
    inside = TriangleUV(thepoly, point, &uv);
    break;
  case 4:
    inside = QuadUV(thepoly, point, &uv);
    break;
  default:
    Fatal(3, "PatchUV", "Can only handle triangular or quadrilateral patches");
  }

  *u = uv.u;
  *v = uv.v;

  return inside;
}


int PatchUniformUV(PATCH *poly, VECTOR *point, double *u, double *v)
{
  int inside = PatchUV(poly, point, u, v);
  if (poly->jacobian)
    BilinearToUniform(poly, u, v);
  return inside;
}


#ifdef RECORD_MONITORED
void PatchCloseAllRecordFiles(PATCH *patch)
{
  int i;

  for(i = 0; i < MAX_RECORD_FILES; i++)
  {
    if(patch->monitor_file[i] != NULL)
    {
      fclose(patch->monitor_file[i]);
      patch->monitor_file[i] = NULL;
    }
  }
}
#endif

