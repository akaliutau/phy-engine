

#include <stdlib.h>

#include "cluster.h"
#include "pools.h"
#include "patch.h"
#include "patchlist_geom.h"
#include "compound.h"
#include "geom.h"


#ifndef MIN_PATCHES_IN_CLUSTER
#define MIN_PATCHES_IN_CLUSTER 3
#endif


typedef struct CLUSTER {
  BOUNDINGBOX bounds;		
  VECTOR mid;			
  PATCHLIST *patches;		
  int nrpatches;		
  struct CLUSTER *children[8];	
  void *radiosity_data;		
} CLUSTER;

#ifndef NOPOOLS
static POOL *clusterPool = (POOL *)NULL;
#define NEWCLUSTER()  	(CLUSTER *)NewPoolCell(sizeof(CLUSTER), 0, "clusters", &clusterPool)
#define DISPOSECLUSTER(ptr) Dispose((unsigned char *)(ptr), &clusterPool)
#else 
#define NEWCLUSTER()	(CLUSTER *)Alloc(sizeof(CLUSTER))
#define DISPOSECLUSTER(ptr) Free((char *)ptr, sizeof(CLUSTER))
#endif 


static CLUSTER *CreateCluster(void)
{
  int i;
  CLUSTER *clus;

  clus = NEWCLUSTER();
  BoundsInit(clus->bounds);
  VECTORSET(clus->mid, 0., 0., 0.);
  clus->patches = PatchListCreate();
  clus->nrpatches = 0;

  for (i=0; i<8; i++)
    clus->children[i] = (CLUSTER *)NULL;

  clus->radiosity_data = (void *)NULL;

  return clus;
}

#ifdef DEBUG

void PrintCluster(FILE *out, CLUSTER *clus, char *level)
{
  int i, len = strlen(level);   

  fprintf(out, "cluster '%s': bounds:\n", level);
  BoundsPrint(out, clus->bounds);
  fprintf(out, "%d patches: ", clus->nrpatches);
  PatchListIterate1B(clus->patches, PatchPrintID, out);
  fprintf(out, "\n");

  level[len+1] = '\0';
  for (i=0; i<8; i++) {
    if (clus->children[i]) {
      level[len] = '1'+i;
      PrintCluster(out, clus->children[i], level);
    }
  }
  level[len] = '\0';
}
#endif


static void ClusterAddPatch(PATCH *patch, CLUSTER *clus)
{
  BOUNDINGBOX patchbounds;
  clus->patches = PatchListAdd(clus->patches, patch);
  clus->nrpatches ++;
  BoundsEnlarge(clus->bounds, patch->bounds ? patch->bounds : PatchBounds(patch, patchbounds));
}


static CLUSTER *CreateToplevelCluster(PATCHLIST *patches)
{
  CLUSTER *clus;

  clus = CreateCluster();
  PatchListIterate1A(patches, ClusterAddPatch, clus);
  VECTORSET(clus->mid,
	    (clus->bounds[MIN_X] + clus->bounds[MAX_X])* 0.5,
	    (clus->bounds[MIN_Y] + clus->bounds[MAX_Y])* 0.5,
	    (clus->bounds[MIN_Z] + clus->bounds[MAX_Z])* 0.5);

  return clus;
}


static int ClusterCheckMovePatch(PATCHLIST *fl, PATCHLIST *prevfl, CLUSTER *clus)
{
  int subi;
  VECTOR patchmid;
  float *patchbounds;
  CLUSTER *subclus;	
  float dx, dy, dz;

  
  patchbounds = fl->patch->bounds;

  
  dx = patchbounds[MAX_X] - patchbounds[MIN_X];
  dy = patchbounds[MAX_Y] - patchbounds[MIN_Y];
  dz = patchbounds[MAX_Z] - patchbounds[MIN_Z];
  if ((dx > 10*EPSILON && dx > (clus->bounds[MAX_X] - clus->bounds[MIN_X]) * 0.5) ||
      (dy > 10*EPSILON && dy > (clus->bounds[MAX_Y] - clus->bounds[MIN_Y]) * 0.5) ||
      (dz > 10*EPSILON && dz > (clus->bounds[MAX_Z] - clus->bounds[MIN_Z]) * 0.5))
    return FALSE;

  
  VECTORSET(patchmid,
	    (patchbounds[MIN_X] + patchbounds[MAX_X]) * 0.5,
	    (patchbounds[MIN_Y] + patchbounds[MAX_Y]) * 0.5,
	    (patchbounds[MIN_Z] + patchbounds[MAX_Z]) * 0.5);  
  subi = VectorCompare(&clus->mid, &patchmid, 0.);

  
  if (subi == 8)
    return FALSE;

  
  subclus = clus->children[subi];

  if (fl == clus->patches)
    clus->patches = clus->patches->next;
  else
    prevfl->next = fl->next;
  clus->nrpatches --;

  fl->next = subclus->patches;
  subclus->patches = fl;
  subclus->nrpatches ++;

  
  BoundsEnlarge(subclus->bounds, patchbounds);

  return TRUE;		
}


static void SplitCluster(CLUSTER *clus)
{
  int i;
  PATCHLIST *fl, *next, *prev;

  
  if (clus == (CLUSTER *)NULL || clus->nrpatches <= MIN_PATCHES_IN_CLUSTER)
    return;

  
  for (i=0; i<8; i++) 
    clus->children[i] = CreateCluster();

  
  fl = clus->patches; prev = (PATCHLIST *)NULL;
  while (fl) {
    next = fl->next;	
    if (!ClusterCheckMovePatch(fl, prev, clus))
      prev = fl;	
    fl = next;
  }

  
  for (i=0; i<8; i++) {
    if (clus->children[i]->nrpatches == 0) {
      DISPOSECLUSTER(clus->children[i]);
      clus->children[i] = (CLUSTER *)NULL;
    } else {
      VECTORSET(clus->children[i]->mid, 
		(clus->children[i]->bounds[MIN_X] + clus->children[i]->bounds[MAX_X])* 0.5,
		(clus->children[i]->bounds[MIN_Y] + clus->children[i]->bounds[MAX_Y])* 0.5,
		(clus->children[i]->bounds[MIN_Z] + clus->children[i]->bounds[MAX_Z])* 0.5);
      
      SplitCluster(clus->children[i]);
    }
  }
}


static GEOM *ConvertClusterToGeom(CLUSTER *clus)
{
  GEOMLIST *children; 
  GEOM *child, *the_patches;
  int i;

  if (!clus)
    return (GEOM *)NULL;
  
  the_patches = (GEOM *)NULL;
  if (clus->patches)
    the_patches = GeomCreate(clus->patches, PatchListMethods());

  children = GeomListCreate();
  for (i=0; i<8; i++)
    if ((child = ConvertClusterToGeom(clus->children[i])))
      children = GeomListAdd(children, child);

#ifdef NOPOOLS
  DISPOSECLUSTER(clus);
#endif

  if (!children)
    return the_patches;

  
  children = GeomListAdd(children, the_patches);
  return GeomCreate(children, CompoundMethods());
}


GEOM *CreateClusterHierarchy(PATCHLIST *patches)
{
  CLUSTER *top;
  GEOM *topgeom;

  
  top = CreateToplevelCluster(patches);
  
  
  SplitCluster(top);

  
  topgeom = ConvertClusterToGeom(top);
#ifndef NOPOOLS
  DisposeAll(&clusterPool);
#endif

  return topgeom;
}

