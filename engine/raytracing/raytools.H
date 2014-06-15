/* raytools.H
 *
 * some utility routines for ray intersections and for
 * statistics
 */

#ifndef _RAYTOOLS_H_
#define _RAYTOOLS_H_

#include "ray.h"
#include "bsdf.h"
#include "patch.h"
#include "pathnode.H"

HITREC *FindRayIntersection(RAY *ray, PATCH *patch, 
			    BSDF *currentBsdf, HITREC *hitstore);

bool PathNodesVisible(CPathNode *node1, CPathNode *node2);
bool EyeNodeVisible(CPathNode *eye, CPathNode *node, float *pix_x, float *pix_y);

// Some globals

extern bool interrupt_raytracing;

#endif
