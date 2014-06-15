/* patch.h: PATCH structure and some routines to operate on patches. */

#ifndef _PATCH_H_
#define _PATCH_H_ 

#ifdef __cplusplus
extern "C" {
#endif

#include "patch_type.h"

/* return true if patch is virtual */
extern int IsPatchVirtual(PATCH *patch); 

/* Creates a VIRTUAL patch with no vertices */
extern PATCH *PatchCreateVirtual(void);

/* Creates a patch structure for a patch with given vertices and sidesness. */
extern PATCH *PatchCreate(int nrvertices, 
			  struct VERTEX *v1, struct VERTEX *v2, 
			  struct VERTEX *v3, struct VERTEX *v4);

/* disposes the memory allocated for the patch, does not remove
 * the pointers to the patch in the VERTEXes of the patch. */
extern void PatchDestroy(PATCH *patch);

/* computes a bounding box for the patch. fills it in in 'bounds' and returns
 * a pointer to 'bounds'. */
extern float *PatchBounds(PATCH *patch, float *bounds);

/* ray-patch intersection test, for computing formfactors, creating raycast 
 * images ... Returns NULL if the RAY doesn't hit the patch. Fills in the given
 * hit record 'the_hit' and returns a pointer to it if a hit is found.
 * Fills in the distance to the patch in maxdist if the patch
 * is hit. Intersections closer than mindist or further than *maxdist are
 * ignored. The hitflags determine what information to return about an
 * intersection and whether or not front/back facing patches are to be 
 * considered and are described in ray.h. */
#include "ray.h"
extern HITREC *PatchIntersect(PATCH *patch, RAY *ray, float mindist, float *maxdist, int hitflags, HITREC *hitstore);

/* Specify up to MAX_EXCLUDED_PATCHES patches not to test for intersections with. 
 * Used to avoid selfintersections when raytracing. First argument is number of
 * patches to include. Next arguments are pointers to the patches to exclude.
 * Call with first parameter == 0 to clear the list. */
extern void PatchDontIntersect(int n, ...);

/* for debugging */
#include <stdio.h>

extern void PatchPrint(FILE *out, PATCH *patch);
extern void PatchPrintID(FILE *out, PATCH *patch);

/* for debugging and ID rendering: */
/* This routine returns the ID number the next patch would get */
extern int PatchGetNextID(void);

/* With this routine, the ID nummer is set that the next patch will get. 
 * Note that patch ID 0 is reserved. The smallest patch ID number should be 1. */
extern void PatchSetNextID(int id);

/* Given the parameter (u,v) of a point on the patch, this routine 
 * computes the 3D space coordinates of the same point, using barycentric
 * mapping for a triangle and bilinear mapping for a quadrilateral. 
 * u and v should be numbers in the range [0,1]. If u+v>1 for a triangle,
 * (1-u) and (1-v) are used instead. */
extern VECTOR *PatchPoint(PATCH *patch, double u, double v, VECTOR *point);

/* Like above, except that always a uniform mapping is used (one that
 * preserves area, with this mapping you'll have more points in "stretched"
 * regions of an irregular quadrilateral, irregular quadrilaterals are the
 * only onces for which this routine will yield other points than the above
 * routine). */
extern VECTOR *PatchUniformPoint(PATCH *patch, double u, double v, VECTOR *point);

/* computes (u,v) parameters of the point on the patch (barycentric or bilinear
 * parametrisation). Returns TRUE if the point is inside the patch and FALSE if 
 * not. 
 * WARNING: The (u,v) coordinates are correctly computed only for points inside 
 * the patch. For points outside, they can be garbage!!! */
extern int PatchUV(PATCH *poly, VECTOR *point, double *u, double *v);

/* Like above, but returns uniform coordinates (inverse of PatchUniformPoint()). */
extern int PatchUniformUV(PATCH *poly, VECTOR *point, double *u, double *v);

/* Converts bilinear to uniform coordinates and vice versa. Use this routines
 * only for patches with explicitely given jacobian! */
extern void BilinearToUniform(PATCH *patch, double *u, double *v);
extern void UniformToBilinear(PATCH *patch, double *u, double *v);

/* Get the normal of a polygon at a specified point on the polygon
 * This will be a phong interpolated normal if all patch vertices have a normal. */
extern VECTOR PatchInterpolatedNormalAtPoint(PATCH *patch, POINT *point);

/* same, but directly given the bilinear coordinates of the point on the patch */
extern VECTOR PatchInterpolatedNormalAtUV(PATCH *patch, double u, double v);

/* computes a interpolated (shading) frame at the uv or point with
 * given parameters on the patch. The frame is consistent over the
 * complete patch if the shading normals in the vertices do not differ
 * too much from the geometric normal. The Z axis is the interpolated
 * normal The X is determined by Z and the projection of the patch by
 * the dominant axis (patch->index).  
 */
extern void PatchInterpolatedFrameAtUV(PATCH *patch, double u, double v,
				  VECTOR *X, VECTOR *Y, VECTOR *Z);
extern void PatchInterpolatedFrameAtPoint(PATCH *patch, POINT *point, 
					  VECTOR *X, VECTOR *Y, VECTOR *Z);

/* returns texture coordinates determined from vertex texture coordinates and
 * given u and v bilinear of barycentric coordinates on the patch. */
extern VECTOR PatchTextureCoordAtUV(PATCH *patch, double u, double v);

/* Use next function (with PatchListIterate) to close any open
   files of the pathc use for recording */

#ifdef RECORD_MONITORED
void PatchCloseAllRecordFiles(PATCH *patch);
#endif

/* Computes average scattered power and emittance of the PATCH */
extern COLOR PatchAverageNormalAlbedo(PATCH *patch, BSDFFLAGS components);
extern COLOR PatchAverageEmittance(PATCH *patch, XXDFFLAGS components);

#ifdef __cplusplus
}
#endif

#endif /*_PATCH_H_*/

