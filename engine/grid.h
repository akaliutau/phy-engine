/* grid.h: RayTracing acceleration using a uniform grid */

#ifndef _GRID_H_
#define _GRID_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "patch.h"
#include "geom.h"
#include "bounds.h"
#include "pools.h"

typedef struct GRIDITEM {
  void *ptr;		/* PATCH or GEOM pointer */
  unsigned flags;	/* patch or geom? last ray id, ... */
} GRIDITEM;

typedef struct GRIDITEMLIST {	/* same layout as LIST in List.h */
  GRIDITEM *elem;
  struct GRIDITEMLIST *next;
} GRIDITEMLIST;

typedef struct GRID {
  short xsize, ysize, zsize;	/* dimensions */
  VECTOR voxsize;
  GRIDITEMLIST **items;		/* 3D array of item lists */
  BOUNDINGBOX bounds;		/* bounding box */
  POOL *gridItemPool;
  GEOM *engridded_geom;
} GRID;

extern GRID *CreateGrid(GEOM *geom);
extern void DestroyGrid(GRID *grid);
extern struct HITREC *GridIntersect(GRID *grid, RAY *ray, float mindist, float *maxdist, int hitflags, HITREC *hitstore);

/* traces a ray through a voxel grid. Returns a list of all intersections. */
extern HITLIST *AllGridIntersections(HITLIST *hits, GRID *grid, RAY *ray, float mindist, float maxdist, int hitflags);


#ifdef NEVER
/* GEOM-wrapped grids */
extern struct GEOM_METHODS gridMethods;
#define GridMethods() (&gridMethods)

/* Tests if a GEOM is a grid geom. */
#define GeomIsGrid(geom)	(geom->methods == &gridMethods)
#endif

#ifdef __cplusplus
}
#endif

#endif /*_GRID_H_*/
