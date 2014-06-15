/****************************************************************************\

  Copyright 1995 The University of North Carolina at Chapel Hill.
  All Rights Reserved.

  Permission to use, copy, modify and distribute this software and its
  documentation for educational, research and non-profit purposes,
  without fee, and without a written agreement is hereby granted,
  provided that the above copyright notice and the following three
  paragraphs appear in all copies.

  IN NO EVENT SHALL THE UNIVERSITY OF NORTH CAROLINA AT CHAPEL HILL BE
  LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
  CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE
  USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY
  OF NORTH CAROLINA HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
  DAMAGES.


  Permission to use, copy, modify and distribute this software and its
  documentation for educational, research and non-profit purposes,
  without fee, and without a written agreement is hereby granted,
  provided that the above copyright notice and the following three
  paragraphs appear in all copies.

  THE UNIVERSITY OF NORTH CAROLINA SPECIFICALLY DISCLAIM ANY
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
  PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
  NORTH CAROLINA HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
  UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

  The authors may be contacted via:

  US Mail:  Jonathan Cohen                      Amitabh Varshney
            Department of Computer Science      Department of Computer Science 
            Sitterson Hall, CB #3175            State University of New York
            University of N. Carolina           Stony Brook, NY 11794-4400, USA 
            Chapel Hill, NC 27599-3175
	    
  Phone:    (919)962-1749                       Phone: (516)632-8446 
	    
  EMail:    cohenj@cs.unc.edu                   varshney@cs.sunysb.edu

\****************************************************************************/

/* Protection from multiple includes. */
#ifndef INCLUDED_OFFSET_INTERSECT_H
#define INCLUDED_OFFSET_INTERSECT_H


/*------------------ Includes Needed for Definitions Below ------------------*/

#include <geometry.h>

#ifdef USE_TRIOCTREE
#include <trioctree.h>
#else
#include <octree.h>
#endif

#include <vertex_removal.h>

/*-------------------------------- Constants --------------------------------*/

#define SURFACE_VS_ITSELF  0
#define SURFACE_VS_OUTER   1
#define SURFACE_VS_INNER   2

/*--------------------------------- Macros ----------------------------------*/


/*---------------------------------- Types ----------------------------------*/


/*---------------------------- Function Prototypes --------------------------*/

void test_self_intersect(Surface *model, Octree *tree, OctreeData *octdata,
			 OctreeData **query_result, double *fuzz_factor);
int edge_offsets_intersect(Octree *outer_offset, Octree *inner_offset,
			   OctreeData **query_result,
			   Edge *edge, double fuzz_factor);
int tri_offsets_intersect(Triangle *tri,
			  Octree *outer_offset, Octree *inner_offset,
			  OctreeData **query_result, double fuzz_factor);
int tri_solution_interference(Hole *hole, Triangle *tri, double fuzz_factor);
int tri_model_interference(Triangle *tri, Vertex *vert,
			   Octree *model_octree, OctreeData **query_result,
			   double fuzz_factor);
int edge_tubes_intersect(Edge *edge, Octree *tubes,
			 OctreeData **query_result, double fuzz_factor);
int index_filtered_tri_tri_intersect(Triangle *tri1, Triangle *tri2,
				     double fuzz_factor, int test_type);
int coord_filtered_tri_tri_intersect(Triangle *tri1, Triangle *tri2,
				     double fuzz_factor, int test_type);
int coord_filtered_edge_tri_intersect(Edge *edge, Triangle *tri,
				      double fuzz_factor);
int unfiltered_edge_tri_intersect(Edge *edge, Triangle *tri,
				  double fuzz_factor);

/*---------------------------Globals (externed)------------------------------*/



/* Protection from multiple includes. */
#endif /*INCLUDED_OFFSET_INTERSECT_H*/



