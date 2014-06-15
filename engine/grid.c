

#include "grid.h"
#include "gridP.h"
#include "pools.h"
#include "error.h"
#include "hitlist.h"

static POOL **gridItemPoolPtr;  

#define NEWGRIDITEM(grid)	(GRIDITEM *)NewPoolCell(sizeof(GRIDITEM), 0, "grid items", gridItemPoolPtr)





static int IsSmall(float *bounds, GRID *grid)
{
  return ((bounds[MAX_X] - bounds[MIN_X]) <= grid->voxsize.x &&
	  (bounds[MAX_Y] - bounds[MIN_Y]) <= grid->voxsize.y &&
	  (bounds[MAX_Z] - bounds[MIN_Z]) <= grid->voxsize.z);
}

static GRIDITEM *NewGridItem(void *ptr, unsigned flags, GRID *grid)
{
  GRIDITEM *item = NEWGRIDITEM(grid);
  item->ptr = ptr;
  item->flags = flags;
  return item;
}

static void EngridItem(GRIDITEM *item, float *itembounds, GRID *grid)
{
  short mina, minb, minc, maxa, maxb, maxc, a, b, c;

  
  BOUNDINGBOX bounds;
  float xext = (grid->bounds[MAX_X] - grid->bounds[MIN_X]) * 1e-4, 
        yext = (grid->bounds[MAX_Y] - grid->bounds[MIN_Y]) * 1e-4, 
        zext = (grid->bounds[MAX_Z] - grid->bounds[MIN_Z]) * 1e-4;
  BoundsCopy(itembounds, bounds);
  bounds[MIN_X] -= xext; bounds[MAX_X] += xext;
  bounds[MIN_Y] -= yext; bounds[MAX_Y] += yext;
  bounds[MIN_Z] -= zext; bounds[MAX_Z] += zext;

  mina = x2voxel(bounds[MIN_X],grid);
  if (mina >= grid->xsize) mina = grid->xsize-1;
  if (mina < 0) mina = 0;
  maxa = x2voxel(bounds[MAX_X],grid);
  if (maxa >= grid->xsize) maxa = grid->xsize-1;
  if (maxa < 0) maxa = 0;

  minb = y2voxel(bounds[MIN_Y],grid);
  if (minb >= grid->ysize) minb = grid->ysize-1;
  if (minb < 0) minb = 0;
  maxb = y2voxel(bounds[MAX_Y],grid);
  if (maxb >= grid->ysize) maxb = grid->ysize-1;
  if (maxb < 0) maxb = 0;

  minc = z2voxel(bounds[MIN_Z],grid);
  if (minc >= grid->zsize) minc = grid->zsize-1;
  if (minc < 0) minc = 0;
  maxc = z2voxel(bounds[MAX_Z],grid);
  if (maxc >= grid->zsize) maxc = grid->zsize-1;
  if (maxc < 0) maxc = 0;

  for (a=mina; a<=maxa; a++)
    for (b=minb; b<=maxb; b++)
      for (c=minc; c<=maxc; c++) {
	GRIDITEMLIST **items = &grid->items[CELLADDR(grid, a,b,c)];
	(*items) = GridItemListAdd(*items, item);
      }
}

static void EngridPatch(PATCH *patch, GRID *grid)
{
  BOUNDINGBOX bounds;
  EngridItem(NewGridItem(patch, ISPATCH, grid), patch->bounds ? patch->bounds : PatchBounds(patch, bounds), grid);
}

static void EngridGeom(GEOM *geom, GRID *grid)
{
  if (IsSmall(geom->bounds, grid)) {
    if (geom->tmp.i  < 10)
      EngridItem(NewGridItem(geom, ISGEOM, grid), geom->bounds, grid);
    else {
      GRID *subgrid = CreateGrid(geom);
      EngridItem(NewGridItem(subgrid, ISGRID, grid), subgrid->bounds, grid);
    }
  } else {
    if (GeomIsAggregate(geom)) {
      ForAllGeoms(child, GeomPrimList(geom)) {
	EngridGeom(child, grid);
      } EndForAll;
    } else {
      ForAllPatches(patch, GeomPatchList(geom)) {
	EngridPatch(patch, grid);
      } EndForAll;
    }
  }
}

static GRID *Engrid(GEOM *geom, short na, short nb, short nc, int istop)
{
  GRID *grid;
  int i;
  float xext, yext, zext;

  if (!geom)
    return (GRID *)NULL;

  if (!geom->bounds) {
    Error("Engrid", "Can't engrid an unbounded geom");
    return (GRID *)NULL;    
  }

  if (na <= 0 || nb <= 0 || nc <= 0) {
    Error("Engrid", "Invalid grid dimensions");
    return (GRID *)NULL;
  }

  grid = (GRID *)Alloc(sizeof(GRID));
  if (istop)  
    gridItemPoolPtr = &(grid->gridItemPool);

  
  xext = (geom->bounds[MAX_X] - geom->bounds[MIN_X]) * 1e-4;
  yext = (geom->bounds[MAX_Y] - geom->bounds[MIN_Y]) * 1e-4; 
  zext = (geom->bounds[MAX_Z] - geom->bounds[MIN_Z]) * 1e-4;
  BoundsCopy(geom->bounds, grid->bounds);
  grid->bounds[MIN_X] -= xext; grid->bounds[MAX_X] += xext;
  grid->bounds[MIN_Y] -= yext; grid->bounds[MAX_Y] += yext;
  grid->bounds[MIN_Z] -= zext; grid->bounds[MAX_Z] += zext;

  grid->xsize = na; grid->ysize = nb; grid->zsize = nc;
  grid->voxsize.x = (grid->bounds[MAX_X] - grid->bounds[MIN_X]) / (float)na;
  grid->voxsize.y = (grid->bounds[MAX_Y] - grid->bounds[MIN_Y]) / (float)nb;
  grid->voxsize.z = (grid->bounds[MAX_Z] - grid->bounds[MIN_Z]) / (float)nc;
  grid->items = (GRIDITEMLIST **)Alloc(na * nb * nc * sizeof(GRIDITEMLIST *));
  grid->gridItemPool = (POOL *)NULL;
  for (i=0; i<na*nb*nc; i++) grid->items[i] = GridItemListCreate();
  EngridGeom(geom, grid);
  grid->engridded_geom = geom;

  return grid;
}

static int GeomCountItems(GEOM *geom)
{
  int count = 0;

  if (GeomIsAggregate(geom)) {
    GEOMLIST *primlist = GeomPrimList(geom);
    ForAllGeoms(child, primlist) {
      count += GeomCountItems(child);
    } EndForAll;
  } else {
    PATCHLIST *patches = GeomPatchList(geom);
    count = PatchListCount(patches);
  }

  return geom->tmp.i = count;
}


GRID *CreateGrid(GEOM *geom)
{
  static int level = 0;
  int gridsize;
  GRID *grid;

  if (level == 0)
    GeomCountItems(geom);

  gridsize = pow((double)geom->tmp.i , 0.33333) + 1;
  fprintf(stderr, "Engridding %d items in %d^3 cells level %d voxel grid ... \n", geom->tmp.i, gridsize, level);
  level++;

  grid = Engrid(geom, gridsize, gridsize, gridsize, (level==1) ? TRUE : FALSE);

  level--;

  return grid;
}

static void DestroyGridRecursive(GRID *grid)
{
  int i;

  if (!grid) return;

  for (i=0; i<grid->xsize * grid->ysize * grid->zsize; i++) {
    ForAllGridItems(item, grid->items[i]) {
      if (IsGrid(item) && item->ptr) {
	
	DestroyGridRecursive(item->ptr);
	item->ptr = NULL;
      }
    } EndForAll;
    GridItemListDestroy(grid->items[i]);
  }
  Free((char *)grid, sizeof(GRID));
}

void DestroyGrid(GRID *grid)
{
  if (!grid) return;
  DestroyGridRecursive(grid);
  DisposeAll(&grid->gridItemPool);  
}




static int RandomRayId(void)
{
  static int count = 0;
  count++;
  return (count & RAYCOUNT_MASK);
}

#ifdef IDEBUG
extern int idebug;
#endif


static int GridBoundsIntersect( GRID *grid, RAY *ray, float mindist, float maxdist, 
			        float *t0, VECTOR *P)
{
  *t0 = mindist;
  VECTORADDSCALED(ray->pos, *t0, ray->dir, *P);
  if (OutOfBounds(P, grid->bounds)) {
#ifdef IDEBUG
    if (idebug) {
      fprintf(stderr, "%s %d: ray org is out of grid bounds\n", __FILE__, __LINE__);
    }
#endif
    *t0 = maxdist;
    if (!BoundsIntersect(ray, grid->bounds, mindist, t0)) {
#ifdef IDEBUG
      if (idebug) {
	fprintf(stderr, "%s %d: ray doesn't intersect grid bounds\n", __FILE__, __LINE__);
      }
#endif
      return FALSE;
    }
    VECTORADDSCALED(ray->pos, *t0, ray->dir, *P);
  }
  else {
#ifdef IDEBUG
    if (idebug) {
      fprintf(stderr, "%s %d: ray org is inside grid bounds\n", __FILE__, __LINE__);
    }
#endif
  }
#ifdef IDEBUG
  if (idebug) {
    fprintf(stderr, "%s %d: ray intersects grid bounds at t=%f, P=%f,%f,%f\n", __FILE__, __LINE__,
	    *t0, P->x, P->y, P->z);
  }
#endif

  return TRUE;
}


static void GridTraceSetup( GRID *grid, RAY *ray, float t0, VECTOR *P, 
			    int *g, VECTOR *tDelta, VECTOR *tNext, int *step, int *out)
{
  
  g[X] = x2voxel(P->x, grid); if (g[X] >= grid->xsize) g[X] = grid->xsize-1;
  g[Y] = y2voxel(P->y, grid); if (g[Y] >= grid->ysize) g[Y] = grid->ysize-1;
  g[Z] = z2voxel(P->z, grid); if (g[Z] >= grid->zsize) g[Z] = grid->zsize-1;

  
  
  if (fabs(ray->dir.x) > EPSILON) {
    if (ray->dir.x > 0.) {
      tDelta->x = grid->voxsize.x / ray->dir.x;
      tNext->x = t0 + (voxel2x(g[X]+1,grid) - P->x) / ray->dir.x;
      step[X] = 1; out[X] = grid->xsize;
    } else {
      tDelta->x = grid->voxsize.x / -ray->dir.x;
      tNext->x = t0 + (voxel2x(g[X],grid) - P->x) / ray->dir.x;
      step[X] = out[X] = -1;
    }
  } else {
    tDelta->x = 0.;
    tNext->x = HUGE;
  }

  
  if (fabs(ray->dir.y) > EPSILON) {
    if (ray->dir.y > 0.) {
      tDelta->y = grid->voxsize.y / ray->dir.y;
      tNext->y = t0 + (voxel2y(g[Y]+1,grid) - P->y) / ray->dir.y;
      step[Y] = 1; out[Y] = grid->ysize;
    } else {
      tDelta->y = grid->voxsize.y / -ray->dir.y;
      tNext->y = t0 + (voxel2y(g[Y],grid) - P->y) / ray->dir.y;
      step[Y] = out[Y] = -1;
    }
  } else {
    tDelta->y = 0.;
    tNext->y = HUGE;
  }

  
  if (fabs(ray->dir.z) > EPSILON) {
    if (ray->dir.z > 0.) {
      tDelta->z = grid->voxsize.z / ray->dir.z;
      tNext->z = t0 + (voxel2z(g[Z]+1,grid) - P->z) / ray->dir.z;
      step[Z] = 1; out[Z] = grid->zsize;
    } else {
      tDelta->z = grid->voxsize.z / -ray->dir.z;
      tNext->z = t0 + (voxel2z(g[Z],grid) - P->z) / ray->dir.z;
      step[Z] = out[Z] = -1;
    }
  } else {
    tDelta->z = 0.;
    tNext->z = HUGE;
  }
}


static int NextVoxel(float *t0, int *g, VECTOR *tNext, VECTOR *tDelta, int *step, int *out)
{
  int ingrid = TRUE;

  if (tNext->x <= tNext->y && tNext->x <= tNext->z) {  
    g[X] += step[X];
    *t0 = tNext->x;
    tNext->x += tDelta->x;
    ingrid = g[X] - out[X];			   
  } else if (tNext->y <= tNext->z) {		     
    g[Y] += step[Y];
    *t0 = tNext->y;
    tNext->y += tDelta->y;
    ingrid = g[Y] - out[Y];
  } else {					     
    g[Z] += step[Z];
    *t0 = tNext->z;
    tNext->z += tDelta->z;
    ingrid = g[Z] - out[Z];
  }
  return ingrid;
}


static HITREC *VoxelIntersect(GRIDITEMLIST *items, RAY *ray, int counter,
			      float mindist, float *maxdist,
			      int hitflags,
			      HITREC *hitstore)
{
  HITREC *hit = NULL;

  ForAllGridItems(item, items) {
    if (LastRayId(item) != counter) {
      
      HITREC *h = (HITREC *)NULL;
      if (IsPatch(item))
	h = PatchIntersect((PATCH *)item->ptr, ray, mindist, maxdist, hitflags, hitstore);
      else if (IsGeom(item))
      {
	h = GeomDiscretisationIntersect((GEOM *)item->ptr, ray, mindist, maxdist, hitflags, hitstore);
#ifdef IDEBUG
	if (idebug) {
	  GEOM *geom = (GEOM *)item->ptr;
	  fprintf(stderr, "====> %s %d: Test with geom with bounds as follows %s:\n", __FILE__, __LINE__,
		  !h ? "failed" : "succeeded");
	  BoundsPrint(stderr, geom->bounds);
	}
#endif
      }
      else if (IsGrid(item))
      {
	h = GridIntersect((GRID *)item->ptr, ray, mindist, maxdist, hitflags, hitstore);
#ifdef IDEBUG
	if (idebug) {
	  GRID *grid = (GRID *)item->ptr;
	  fprintf(stderr, "====> %s %d: Test with grid with bounds as follows %s:\n", __FILE__, __LINE__,
		  !h ? "failed" : "succeeded");
	  BoundsPrint(stderr, grid->bounds);
	}
#endif
      }
      if (h) hit = h;

      UpdateRayId(item, counter);
    }
    else {
#ifdef IDEBUG
      if (idebug) {
	fprintf(stderr, "====> %s %d: omitting test with ", __FILE__, __LINE__);
	if (IsPatch(item)) {
	  fprintf(stderr, "patch id %d\n", ((PATCH *)item->ptr)->id);
	} else {
	  fprintf(stderr, "geom or grid\n");
	}
      }
#endif
    }
  } EndForAll;

  return hit;
}


struct HITREC *GridIntersect(GRID *grid, RAY *ray, float mindist, float *maxdist, int hitflags, HITREC *hitstore)
{
  VECTOR tNext, tDelta, P;
  int step[3], out[3], g[3];
  HITREC *hit = NULL;
  float t0;
  int counter;

  if (!grid || !GridBoundsIntersect(grid, ray, mindist, *maxdist, &t0, &P)) {
#ifdef IDEBUG
    if (idebug) {
      fprintf(stderr, "%s %d: No grid or grid bounds not intersected\n", __FILE__, __LINE__);
    }
#endif
    return (HITREC *)NULL;
  }

  GridTraceSetup(grid, ray, t0, &P, g, &tDelta, &tNext, step, out);
  

  
  counter = RandomRayId();

  do {
    GRIDITEMLIST *list = grid->items[CELLADDR(grid, g[X],g[Y],g[Z])];
#ifdef IDEBUG
    if (idebug) {
      VECTOR b;
      fprintf(stderr, "=====> %s %d: grid cell %d,%d,%d, bounds = \n", __FILE__, __LINE__, g[X], g[Y], g[Z]);
      fprintf(stderr, "   %f->%f, %f->%f, %f->%f\n",
	      voxel2x(g[X], grid), voxel2x(g[X]+1, grid),
	      voxel2y(g[Y], grid), voxel2y(g[Y]+1, grid),
	      voxel2z(g[Z], grid), voxel2z(g[Z]+1, grid));
      VECTORADDSCALED(ray->pos, t0, ray->dir, b);
      fprintf(stderr, "  t0 = %f, current position = %f,%f,%f\n", t0, b.x, b.y, b.z);
      fprintf(stderr, "   grid cel contents:\n");
      ForAllGridItems(item, list) {
	if (IsPatch(item)) {
	  fprintf(stderr, "patch id %d", ((PATCH *)item->ptr)->id);
	} else
	  fprintf(stderr, "geom or grid");
	fprintf(stderr, " (%s tested), ",
		(LastRayId(item) == counter) ? "already" : "to be");
      } EndForAll;
      fprintf(stderr, "\n");
    }
#endif
    if (list) {
      HITREC *h;
      if ((h=VoxelIntersect(list,ray,counter,t0,maxdist,hitflags,hitstore)))
	hit = h;
    }
#ifdef IDEBUG
    if (idebug) {
      VECTOR b;
      fprintf(stderr, "=====> %s %d: next cell boundaries:\n", __FILE__, __LINE__);
      VECTORADDSCALED(ray->pos, tNext.x, ray->dir, b);
      fprintf(stderr, "     X: t=%f: %f,%f,%f ", tNext.x, b.x, b.y, b.z);
      fprintf(stderr, "next cell %d,%d,%d\n", g[X] + step[X], g[Y], g[Z]);
      VECTORADDSCALED(ray->pos, tNext.y, ray->dir, b);
      fprintf(stderr, "     Y: t=%f: %f,%f,%f ", tNext.y, b.x, b.y, b.z);
      fprintf(stderr, "next cell %d,%d,%d\n", g[X], g[Y] + step[Y], g[Z]);
      VECTORADDSCALED(ray->pos, tNext.z, ray->dir, b);
      fprintf(stderr, "     Z: t=%f: %f,%f,%f ", tNext.z, b.x, b.y, b.z);
      fprintf(stderr, "next cell %d,%d,%d\n", g[X], g[Y], g[Z] + step[Z]);
    }
#endif
  } while (NextVoxel(&t0, g, &tNext, &tDelta, step, out) && t0 <= *maxdist);

  return hit;
}


static HITLIST *AllVoxelIntersections(HITLIST *hitlist, 
			   GRIDITEMLIST *items, RAY *ray, int counter,
			   float mindist, float maxdist,
			   int hitflags)
{
  HITREC hitstore;
  ForAllGridItems(item, items) {
    
    if (LastRayId(item) != counter) {
      if (IsPatch(item)) {
	float tmax = maxdist;
	HITREC *h = PatchIntersect((PATCH *)item->ptr, ray, mindist, &tmax, hitflags, &hitstore);
	if (h) {
	  hitlist = HitListAdd(hitlist, DuplicateHit(h));
	}
      } else if (IsGeom(item)) {
	hitlist = GeomAllDiscretisationIntersections(hitlist, (GEOM *)item->ptr, ray, mindist, maxdist, hitflags);
      } else if (IsGrid(item)) {
	hitlist = AllGridIntersections(hitlist, (GRID *)item->ptr, ray, mindist, maxdist, hitflags);
      }

      UpdateRayId(item, counter);
    }
  } EndForAll;

  return hitlist;
}


HITLIST *AllGridIntersections(HITLIST *hits, GRID *grid, RAY *ray, float mindist, float maxdist, int hitflags)
{
  VECTOR tNext, tDelta, P;
  int step[3], out[3], g[3];
  float t0;
  int counter;

  if (!grid || !GridBoundsIntersect(grid, ray, mindist, maxdist, &t0, &P)) 
    return hits;
  GridTraceSetup(grid, ray, t0, &P, g, &tDelta, &tNext, step, out);

  
  counter = RandomRayId();

  do {
    GRIDITEMLIST *itemlist = grid->items[CELLADDR(grid, g[X],g[Y],g[Z])];
    if (itemlist) 
      hits = AllVoxelIntersections(hits,itemlist,ray,counter,t0,maxdist,hitflags);
  } while (NextVoxel(&t0, g, &tNext, &tDelta, step, out) && t0 <= maxdist);

  return hits;
}
