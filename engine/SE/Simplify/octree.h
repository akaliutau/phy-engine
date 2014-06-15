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
#ifndef INCLUDED_OFFSET_OCTREE_H
#define INCLUDED_OFFSET_OCTREE_H


/*------------------ Includes Needed for Definitions Below ------------------*/

#include <geometry.h>

/*-------------------------------- Constants --------------------------------*/

#define LXLYLZ 0
#define LXLYHZ 1
#define LXHYLZ 2
#define LXHYHZ 3
#define HXLYLZ 4
#define HXLYHZ 5
#define HXHYLZ 6
#define HXHYHZ 7

/*--------------------------------- Macros ----------------------------------*/


/*---------------------------------- Types ----------------------------------*/

typedef struct OctreeData
{
    Extents  bbox;
    void    *data;
    char     found;   /* used to identify duplicates during range queries */
} OctreeData;

typedef struct OctreeNode
{
    struct OctreeNode *parent;
    Point              center;
    Point              sides;
    int                num_elements;
    int                max_elements;
    OctreeData       **elements;
    int                child_elements;
    struct OctreeNode *child;
} OctreeNode;

typedef struct Octree
{
    OctreeNode *root;
    float      	min_side;
    float      	delta;
    int        	max_node_objs;
} Octree;

/*---------------------------- Function Prototypes --------------------------*/

void octree_create(Octree **tree, Extents bbox,
		   double min_side, int max_node_objects);
void octree_insert(Octree *tree, OctreeData *element);
void octree_remove(Octree *tree, OctreeData *element);
void octree_destroy(Octree **tree);
void octree_range_query(Octree *tree, Extents bbox,
			OctreeData **element_list, int *num_elements);
void octree_print_stats(Octree *tree);

OctreeNode *octree_point_query(Octree *tree, Point point);

/*---------------------------Globals (externed)------------------------------*/



/* Protection from multiple includes. */
#endif /*INCLUDED_OFFSET_OCTREE_H*/



