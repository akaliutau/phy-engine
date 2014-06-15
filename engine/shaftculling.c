



#include <stdlib.h>
#include "shaftculling.h"
#include "patchlist_geom.h"
#include "geom.h"
#include "patch.h"
#include "error.h"
#include "Boolean.h"
#include "geometry3d.h"


static SHAFTCULLSTRATEGY strategy = OVERLAP_OPEN;


SHAFTCULLSTRATEGY SetShaftCullStrategy(SHAFTCULLSTRATEGY newstrategy)
{
  SHAFTCULLSTRATEGY oldstrategy = strategy;
  strategy = newstrategy;
  return oldstrategy;
}

void ShaftOmit(SHAFT *shaft, GEOM *geom)
{
  shaft->omit[shaft->nromit++] = geom;
}

void ShaftDontOpen(SHAFT *shaft, GEOM *geom)
{
  shaft->dontopen[shaft->nrdontopen++] = geom;
}


SHAFT *ConstructShaft(float *ref1, float *ref2, SHAFT *shaft)
{
  int i, j, hasminmax1[6], hasminmax2[6];
  SHAFTPLANE *plane;

  shaft->nromit = shaft->nrdontopen = 0;
  shaft->cut = FALSE;

  shaft->ref1 = ref1;
  shaft->ref2 = ref2;

  
  shaft->center1.x = 0.5 * (ref1[MIN_X] + ref1[MAX_X]);
  shaft->center1.y = 0.5 * (ref1[MIN_Y] + ref1[MAX_Y]);
  shaft->center1.z = 0.5 * (ref1[MIN_Z] + ref1[MAX_Z]);

  shaft->center2.x = 0.5 * (ref2[MIN_X] + ref2[MAX_X]);
  shaft->center2.y = 0.5 * (ref2[MIN_Y] + ref2[MAX_Y]);
  shaft->center2.z = 0.5 * (ref2[MIN_Z] + ref2[MAX_Z]);

  for (i=0; i<6; i++) {
    hasminmax1[i] = hasminmax2[i] = 0;
  }

  
  for (i=MIN_X; i<=MIN_Z; i++) {
    if (shaft->ref1[i] < shaft->ref2[i]) {
      shaft->extent[i] = shaft->ref1[i];
      hasminmax1[i] = 1;
    } else {
      shaft->extent[i] = shaft->ref2[i];
      if (!FLOATEQUAL(shaft->ref1[i], shaft->ref2[i], EPSILON))
	hasminmax2[i] = 1;
    }
  }

  for (i=MAX_X; i<=MAX_Z; i++) {
    if (shaft->ref1[i] > shaft->ref2[i]) {
      shaft->extent[i] = shaft->ref1[i];
      hasminmax1[i] = 1;
    } else {
      shaft->extent[i] = shaft->ref2[i];
      if (!FLOATEQUAL(shaft->ref1[i], shaft->ref2[i], EPSILON))
	hasminmax2[i] = 1;
    }
  }

  
  plane = &shaft->plane[0];
  for (i=0; i<6; i++) {
    if (!hasminmax1[i]) continue;
    for (j=0; j<6; j++) {
      float u1, u2, v1, v2, du, dv;
      int a=(i%3), b=(j%3); 
      
      if (!hasminmax2[j] ||
	  a == b)  continue;
      
      u1 = shaft->ref1[i];  
      v1 = shaft->ref1[j];
      u2 = shaft->ref2[i];
      v2 = shaft->ref2[j];
      
      if ((i<=MIN_Z && j<=MIN_Z) || (i>=MAX_X && j>=MAX_X)) {
	du = v2-v1;
	dv = u1-u2;
      } else { 
	du = v1-v2;
	dv = u2-u1;
      }
      
      plane->n[a] = du;
      plane->n[b] = dv;
      plane->n[3-a-b] = 0.;
      plane->d = -(du*u1 + dv*v1);
      
      plane->coord_offset[0] = plane->n[0] > 0. ? MIN_X : MAX_X;
      plane->coord_offset[1] = plane->n[1] > 0. ? MIN_Y : MAX_Y;
      plane->coord_offset[2] = plane->n[2] > 0. ? MIN_Z : MAX_Z;
      
      plane++;
    }
  }
  shaft->planes = plane - &shaft->plane[0];
  
  return shaft;
}


#define INSIDE -1
#define OVERLAP 0
#define OUTSIDE 1
#define COPLANAR 2


static int TestPolygonWrtPlane(POLYGON *poly, DVECTOR *normal, double d)
{
  int i, out, in;	
                        

  out = in = FALSE;
  for (i=0; i<poly->nrvertices; i++) {
    double e = VECTORDOTPRODUCT(*normal, poly->vertex[i]) + d,
           tolerance = fabs(d) * EPSILON + VECTORTOLERANCE(poly->vertex[i]);
    out |= (e > tolerance);
    in |= (e < -tolerance);
    if (out && in)
      return OVERLAP;
  }
  return (out ? OUTSIDE : (in ? INSIDE : COPLANAR));
}


int VerifyPolygonWrtPlane(POLYGON *poly, DVECTOR *normal, double d, int side)
{
  int i, out, in;
  
  out = in = FALSE;
  for (i=0; i<poly->nrvertices; i++) {
    double e = VECTORDOTPRODUCT(*normal, poly->vertex[i]) + d,
           tolerance = fabs(d) * EPSILON + VECTORTOLERANCE(poly->vertex[i]);
    out |= (e > tolerance);
    if (out && (side==INSIDE || side==COPLANAR))
      return FALSE;
    in |= (e < -tolerance);
    if (in && (side==OUTSIDE || side==COPLANAR || (out && side!=OVERLAP)))
      return FALSE;
  }

  if (in) {
    if (out) {
      if (side == OVERLAP) return TRUE;
    } else {
      if (side == INSIDE) return TRUE;
    }
  } else {
    if (out) {
      if (side == OUTSIDE) return TRUE;
    } else {
      if (side == COPLANAR) return TRUE;
    }
  }
  return FALSE;
}


int TestPointWrtPlane(POINT *p, DVECTOR *normal, double d)
{
  double e, tolerance=fabs(d*EPSILON) + VECTORTOLERANCE(*p);
  e = VECTORDOTPRODUCT(*normal, *p) + d;
  if (e < -tolerance) return INSIDE;
  if (e > +tolerance) return OUTSIDE;
  return COPLANAR;
}


static int CompareShaftPlanes(SHAFTPLANE *p1, SHAFTPLANE *p2)
{
  double tolerance;

  
  if (p1->n[0] < p2->n[0] - EPSILON)
    return -1;
  else if (p1->n[0] > p2->n[0] + EPSILON)
    return +1;

  if (p1->n[1] < p2->n[1] - EPSILON)
    return -1;
  else if (p1->n[1] > p2->n[1] + EPSILON)
    return +1;

  if (p1->n[2] < p2->n[2] - EPSILON)
    return -1;
  else if (p1->n[2] > p2->n[2] + EPSILON)
    return +1;

  
  tolerance = fabs(MAX(p1->d, p2->d) * EPSILON);
  if (p1->d < p2->d - tolerance)
    return -1;
  else if (p1->d > p2->d + tolerance)
    return +1;
  return 0;
}


static int UniqueShaftPlane(SHAFT *shaft, SHAFTPLANE *plane)
{
  SHAFTPLANE *ref;

  for (ref=&shaft->plane[0]; ref != plane; ref++) 
    if (CompareShaftPlanes(ref, plane)==0)
      return FALSE;
  return TRUE;
}


static void FillInPlane(SHAFTPLANE *plane, double nx, double ny, double nz, double d)
{
  plane->n[0] = nx;
  plane->n[1] = ny;
  plane->n[2] = nz;
  plane->d = d;
  
  plane->coord_offset[0] = plane->n[0] > 0. ? MIN_X : MAX_X;
  plane->coord_offset[1] = plane->n[1] > 0. ? MIN_Y : MAX_Y;
  plane->coord_offset[2] = plane->n[2] > 0. ? MIN_Z : MAX_Z;
}


static void ConstructPolygonToPolygonPlanes(POLYGON *p1, POLYGON *p2, SHAFT *shaft)
{
  SHAFTPLANE *plane = &shaft->plane[shaft->planes];
  POINT *cur, *next, *other;
  DVECTOR normal;
  double d, norm;
  int i, j, k, side, planes_found_for_edge, max_planes_per_edge;

  
  VECTORCOPY(p1->normal, normal);	
  switch (TestPolygonWrtPlane(p2, &normal, p1->plane_constant)) {
  case INSIDE:
    
    FillInPlane(plane, p1->normal.x, p1->normal.y, p1->normal.z, p1->plane_constant); 
    if (UniqueShaftPlane(shaft, plane)) plane++;
    max_planes_per_edge = 1;
    break;
  case OUTSIDE:
    
    FillInPlane(plane, -p1->normal.x, -p1->normal.y, -p1->normal.z, -p1->plane_constant); 
    if (UniqueShaftPlane(shaft, plane)) plane++;
    max_planes_per_edge = 1;
    break;
  case OVERLAP:
    
    max_planes_per_edge = 2;
    break;
  default:	
    
    return;
  }

  for (i=0; i<p1->nrvertices; i++) {
    
    cur = &p1->vertex[i];
    next = &p1->vertex[(i+1)%p1->nrvertices];

    planes_found_for_edge = 0;
    for (j=0; j<p2->nrvertices && planes_found_for_edge < max_planes_per_edge; j++) {
      
      other = &p2->vertex[j];
      
      
      VECTORTRIPLECROSSPRODUCT(*cur, *next, *other, normal);
      norm = VECTORNORM(normal);
      if (norm < EPSILON)
	continue;	
      VECTORSCALEINVERSE(norm, normal, normal);
      d = -VECTORDOTPRODUCT(normal, *cur);

      
      k = (i+2)%p1->nrvertices;
      side = TestPointWrtPlane(&p1->vertex[k], &normal, d);
      for (k=(i+3)%p1->nrvertices; k != i; k=(k+1)%p1->nrvertices) {
	int nside = TestPointWrtPlane(&p1->vertex[k], &normal, d);
	if (side == COPLANAR)
	  side = nside;
	else if (nside != COPLANAR)
	  if (side != nside)	
	    side = OVERLAP;	
      }				
      if (side != INSIDE && side != OUTSIDE)
	continue;	

      
      if (VerifyPolygonWrtPlane(p2, &normal, d, side)) {
	if (side==INSIDE)	
	  FillInPlane(plane, normal.x, normal.y, normal.z, d);
	else
	  FillInPlane(plane, -normal.x, -normal.y, -normal.z, -d);
	if (UniqueShaftPlane(shaft, plane)) plane++;
	planes_found_for_edge++;
      }
    }
  }

  shaft->planes = plane - &shaft->plane[0];
}


SHAFT *ConstructPolygonToPolygonShaft(POLYGON *p1, POLYGON *p2, SHAFT *shaft)
{
  int i;

  
  shaft->ref1 = shaft->ref2 = (float *)NULL;
  
  
  BoundsCopy(p1->bounds, shaft->extent);
  BoundsEnlarge(shaft->extent, p2->bounds);

  
  shaft->omit[0] = shaft->omit[1] = (GEOM *)NULL;
  shaft->dontopen[0] = shaft->dontopen[1] = (GEOM *)NULL;
  shaft->nromit = shaft->nrdontopen = 0;
  shaft->cut = FALSE;

  
  shaft->center1 = p1->vertex[0];
  for (i=1; i<p1->nrvertices; i++) {
    VECTORADD(shaft->center1, p1->vertex[i], shaft->center1);
  }
  VECTORSCALEINVERSE((float)p1->nrvertices, shaft->center1, shaft->center1);

  shaft->center2 = p2->vertex[0];
  for (i=1; i<p2->nrvertices; i++) {
    VECTORADD(shaft->center2, p2->vertex[i], shaft->center2);
  }
  VECTORSCALEINVERSE((float)p2->nrvertices, shaft->center2, shaft->center2);

  
  shaft->planes = 0;
  ConstructPolygonToPolygonPlanes(p1, p2, shaft);
  ConstructPolygonToPolygonPlanes(p2, p1, shaft);

  return shaft;
}


int ShaftBoxTest(float *bounds, SHAFT *shaft)
{
  int i;
  SHAFTPLANE *plane;

  
  if (DisjunctBounds(bounds, shaft->extent))
    return OUTSIDE;

  
  for (i=0, plane=&shaft->plane[0]; i<shaft->planes; i++, plane++)
    if (plane->n[0] * bounds[plane->coord_offset[0]] +
	plane->n[1] * bounds[plane->coord_offset[1]] +
	plane->n[2] * bounds[plane->coord_offset[2]] +
	plane->d > -fabs(plane->d * EPSILON))
      return OUTSIDE;

  
  if ((shaft->ref1 && !DisjunctBounds(bounds, shaft->ref1)) ||
      (shaft->ref2 && !DisjunctBounds(bounds, shaft->ref2)))
    return OVERLAP;

  
  for (i=0, plane=&shaft->plane[0]; i<shaft->planes; i++, plane++)
    if (plane->n[0] * bounds[(plane->coord_offset[0]+3)%6] +
	plane->n[1] * bounds[(plane->coord_offset[1]+3)%6] +
	plane->n[2] * bounds[(plane->coord_offset[2]+3)%6] +
	plane->d > fabs(plane->d * EPSILON))
      return OVERLAP;

  return INSIDE;
}


int ShaftPatchTest(PATCH *patch, SHAFT *shaft)
{
  int i, j, someout, inall[PATCHMAXVERTICES];
  SHAFTPLANE *plane;
  double tmin[PATCHMAXVERTICES], tmax[PATCHMAXVERTICES], 
         ptol[PATCHMAXVERTICES];
  RAY ray; float dist;
  HITREC hitstore;

  
  someout = FALSE;
  for (j=0; j<patch->nrvertices; j++) {
    inall[j] = TRUE;
    tmin[j] = 0.;  
    tmax[j] = 1.;
    ptol[j] = VECTORTOLERANCE(*patch->vertex[j]->point); 
  }

  for (i=0, plane=&shaft->plane[0]; i<shaft->planes; i++, plane++) {
    
    DVECTOR plane_normal;
    double e[PATCHMAXVERTICES], tolerance;
    int in, out, side[PATCHMAXVERTICES];

    VECTORSET(plane_normal, plane->n[0], plane->n[1], plane->n[2]);

    in = out = FALSE; 
    for (j=0; j<patch->nrvertices; j++) {
      e[j] = VECTORDOTPRODUCT(plane_normal, *patch->vertex[j]->point) + plane->d;
      tolerance = fabs(plane->d) * EPSILON + ptol[j];
      side[j] = COPLANAR;
      if (e[j] > tolerance) {
	side[j] = OUTSIDE;
	out = TRUE;
      } else if (e[j] < -tolerance) {
	side[j] = INSIDE;
	in = TRUE;
      }
      if (side[j] != INSIDE) inall[j] = FALSE;	
    }

    if (!in) 		
      return OUTSIDE;

    if (out) {    	
      someout = TRUE;	

      for (j=0; j<patch->nrvertices; j++) {
	
	int k = (j+1)%patch->nrvertices;
	if (side[j] != side[k]) {
	  if (side[k] == OUTSIDE) {	
	    if (side[j] == INSIDE) {	
	      if (tmax[j] > tmin[j]) {
		double t = e[j] / (e[j] - e[k]);
		if (t < tmax[j]) tmax[j] = t;
	      }
	    } else  { 
	      tmax[j] = -EPSILON;
	    }
	  } else if (side[j] == OUTSIDE) { 
	    if (side[k] == INSIDE) {	
	      if (tmin[j] < tmax[j]) {
		double t = e[j] / (e[j] - e[k]);
		if (t > tmin[j]) tmin[j] = t;
	      }
	    } else  { 
	      tmin[j] = 1.+EPSILON;
	    }
	  }
	} else if (side[j] == OUTSIDE) {	
	  tmax[j] = -EPSILON;
	}
      }
    }
  }

  
  if (shaft->ref1 || shaft->ref2)
    return OVERLAP;

  if (!someout)		
    return INSIDE;

  for (j=0; j<patch->nrvertices; j++)
    if (inall[j])	
      return OVERLAP;	

  
  for (j=0; j<patch->nrvertices; j++) 
    if (tmin[j] + EPSILON < tmax[j] - EPSILON)
      return OVERLAP;
  
  
  ray.pos = shaft->center1;
  VECTORSUBTRACT(shaft->center2, shaft->center1, ray.dir);
  dist = 1. - EPSILON;
  if (PatchIntersect(patch, &ray, EPSILON, &dist, HIT_FRONT|HIT_BACK, &hitstore)) {
    shaft->cut = TRUE;
    return OVERLAP;
  }

  return OUTSIDE;
}


static int Omit(SHAFT *shaft, GEOM *geom)
{
  int i;

  for (i=0; i<shaft->nromit; i++)
    if (shaft->omit[i] == geom) return TRUE;
  return FALSE;
}


static int DontOpen(SHAFT *shaft, GEOM *geom)
{
  int i;

  for (i=0; i<shaft->nrdontopen; i++)
    if (shaft->dontopen[i] == geom) return TRUE;
  return FALSE;
}


PATCHLIST *ShaftCullPatchlist(PATCHLIST *pl, SHAFT *shaft, PATCHLIST *culledpatchlist)
{
  int total, kept, bbside; 

  for (kept=total=0; pl && !shaft->cut; pl=pl->next, total++) {
    if (pl->patch->omit || Omit(shaft, (GEOM *)pl->patch))
      continue;

    if (!pl->patch->bounds) { 
      BOUNDINGBOX bounds;
      PatchBounds(pl->patch, bounds);
    }
    if ((bbside=ShaftBoxTest(pl->patch->bounds, shaft))!=OUTSIDE) {
      
      if (bbside==INSIDE || ShaftPatchTest(pl->patch, shaft)!=OUTSIDE) {
	culledpatchlist = PatchListAdd(culledpatchlist, pl->patch);
	kept++;
      }
    }
  }
  return culledpatchlist;
}


static GEOMLIST *Keep(GEOM *geom, GEOMLIST *candlist)
{
  if (geom->omit)
    return candlist;

  if (geom->shaftcullgeom) {
    GEOM *newgeom = GeomDuplicate(geom);
    newgeom->shaftcullgeom = TRUE;
    candlist = GeomListAdd(candlist, newgeom);
  } else {
    candlist = GeomListAdd(candlist, geom); 
  }
  return candlist;
}


static GEOMLIST *Open(GEOM *geom, SHAFT *shaft, GEOMLIST *candlist)
{
  if (geom->omit)
    return candlist;

  if (GeomIsAggregate(geom)) {
    candlist = DoShaftCulling(GeomPrimList(geom), shaft, candlist);
  } else {
    PATCHLIST *patchlist;
    patchlist = GeomPatchList(geom);
    patchlist = ShaftCullPatchlist(patchlist, shaft, NULL);
    if (patchlist) {
      GEOM *newgeom;
      newgeom = GeomCreate(patchlist, PatchListMethods());
      newgeom->shaftcullgeom = TRUE;
      candlist = GeomListAdd(candlist, newgeom);
    }
  }
  return candlist;
}


GEOMLIST *ShaftCullGeom(GEOM *geom, SHAFT *shaft, GEOMLIST *candlist)
{
  if (geom->omit || Omit(shaft, geom)) 
    return candlist;

  
  switch (geom->bounded ? ShaftBoxTest(geom->bounds, shaft) : OVERLAP) {
  case INSIDE:
    if (strategy == ALWAYS_OPEN && !DontOpen(shaft, geom)) 
      candlist = Open(geom, shaft, candlist);
    else
      candlist = Keep(geom, candlist);
    break;
  case OVERLAP:
    if (strategy == KEEP_CLOSED || DontOpen(shaft, geom)) 
      candlist = Keep(geom, candlist);
    else
      candlist = Open(geom, shaft, candlist);
    break;
  default:
    break;
  }

  return candlist;
}




GEOMLIST *DoShaftCulling(GEOMLIST *world, SHAFT *shaft, GEOMLIST *candlist)
{
  for (; world && !shaft->cut; world=world->next) {
    candlist = ShaftCullGeom(world->geom, shaft, candlist);
  }
  
  return candlist;
}

void FreeCandidateList(GEOMLIST *candlist)
{
  GEOMLIST *gl;

  
  for (gl=candlist; gl; gl=gl->next)
    if (gl->geom->shaftcullgeom)
      GeomDestroy(gl->geom);

  GeomListDestroy(candlist);
}
