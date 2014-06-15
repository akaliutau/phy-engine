/* scene.h: scene data structures - defined in scene.c */

#ifndef _SCENE_H_
#define _SCENE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "materiallist.h"
#include "geomlist.h"
#include "patchlist.h"
#include "grid.h"
#include "background.h"

/* the current scene */
extern GEOMLIST *World;

/* the list of all materials present in the current scene */
extern MATERIALLIST *MaterialLib;

/* the current background (sky, environment map, ...) for the scene */
extern BACKGROUND *Background;

/* the list of all PATCHes in the current scene. Automatically derived from
 * 'World' when loading a scene.*/
extern PATCHLIST *Patches;

/* the light of all patches on light sources, useful for e.g. next event estimation in
 * Monte Carlo raytracing etc... */
extern PATCHLIST *LightSourcePatches;

/* the top of the patch cluster hierarchy for the scene. Automatically derived 
 * from 'Patches' when loading a new scene. */
extern GEOMLIST *ClusteredWorld;
extern GEOM *ClusteredWorldGeom;	/* single GEOM containing the above */

/* voxel grid containing the whole world. */
extern GRID *WorldGrid;

#ifdef __cplusplus
}
#endif

#endif /*_SCENE_H_*/


