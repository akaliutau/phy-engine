





#include <fill_hole.h>
#include <vertex_removal.h>
#include <simplify.h>
#include <intersect.h>
#include <stdio.h>
#include <stdlib.h>



#define MAX_RETRIANGULATIONS 200



#define NON_MANIFOLD_EDGE(tri)			\
        (((tri)->edges[0]->handy_mark > 1) ||	\
	 ((tri)->edges[1]->handy_mark > 1) ||	\
	 ((tri)->edges[2]->handy_mark > 1))






static void create_subholes(Hole *hole, Hole *subholes, int tri_index);
static void destroy_subholes(Hole *subholes);
static void add_tri_to_hole_solution(Hole *hole, Triangle *tri);
static void revert_hole_solution(Hole *hole, int prev_solution_size);









int fill_hole(Hole *hole, Octree *outer_offset, Octree *inner_offset,
	      Octree *model_octree, OctreeData **query_result,
	      double fuzz_factor, int exhaustive_filling_flag)
{
    int         i;
    Hole        subholes[3];
    Triangle   *tri;
    int         success;
    int         tri_index;
    int         prev_solution_size;
    static int  recursion_level = 0;
    static int  retriangulations_tried;
    
    if (recursion_level == 0)
    {
	if (hole->num_verts > 2)
	{
	    ALLOCN(hole->solution_tris, Triangle *, hole->num_verts-2);
	}
	else
	    hole->solution_tris = NULL;
	hole->num_solution_tris = 0;
	hole->num_solution_tris_ptr = &(hole->num_solution_tris);
	retriangulations_tried = 0;
    }

    if (hole->num_verts < 3)
	return TRUE;
    if (hole->num_candidate_tris < (hole->num_verts - 2))
	return FALSE;

    recursion_level++;
    
    prev_solution_size = *hole->num_solution_tris_ptr;
    
    for (tri_index=0; tri_index<hole->num_candidate_tris; tri_index++)
    {
	tri = hole->ranked_candidate_tris[tri_index];
	
#ifdef REMOVE_BORDER_VERTS
	
	if (hole->num_verts == (hole->num_edges+1))
	{
	    if (((tri->verts[0] != hole->verts[0]) &&
		 (tri->verts[1] != hole->verts[0]) &&
		 (tri->verts[2] != hole->verts[0])) ||
		((tri->verts[0] != hole->verts[hole->num_verts-1]) &&
		 (tri->verts[1] != hole->verts[hole->num_verts-1]) &&
		 (tri->verts[2] != hole->verts[hole->num_verts-1])))
		continue;
	}
#endif 
	
	if ((NON_MANIFOLD_EDGE(tri)) ||
	    (tri_solution_interference(hole, tri, fuzz_factor)) ||
	    (tri_model_interference(tri, hole->center_vert, model_octree,
				    query_result, fuzz_factor)) ||
	    (tri_offsets_intersect(tri, outer_offset, inner_offset,
				   query_result, fuzz_factor)))
	    continue;  

	add_tri_to_hole_solution(hole, tri);
	
	create_subholes(hole, subholes, tri_index);

	for (i=0, success = TRUE; i<3; i++)
	    if (!(fill_hole(&(subholes[i]), outer_offset, inner_offset,
			    model_octree, query_result, fuzz_factor,
			    exhaustive_filling_flag)))
	    {
		success = FALSE;
		break;
	    }

	
	destroy_subholes(subholes);

	if (success == TRUE)
	{
	    recursion_level--;
	    return TRUE;
	}
	else
	{
	    revert_hole_solution(hole, prev_solution_size);
	    if ((exhaustive_filling_flag == TRUE) &&
		((retriangulations_tried++) < MAX_RETRIANGULATIONS))
		continue;
	    else
		break;
	}
    }

    recursion_level--;
    
    return FALSE;
    
} 



static void create_subholes(Hole *hole, Hole *subholes, int tri_index)
{
    int       i, j, k;
    Triangle *new_tri;
    int       tri_vert_id;
    Vertex   *vert;
    Edge     *edge;
    Triangle *tri;
    int       start_tri;

    
    new_tri = hole->ranked_candidate_tris[tri_index];
    
    for (i=0; i<3; i++)
    {
	subholes[i].center_vert = hole->center_vert;
	
	ALLOCN(subholes[i].verts, Vertex *, hole->num_verts);
	ALLOCN(subholes[i].edges, Edge *, hole->num_edges);
	ALLOCN(subholes[i].ranked_candidate_tris, Triangle *,
	       hole->num_candidate_tris);
	subholes[i].num_verts = 0;
	subholes[i].num_edges = 0;
	subholes[i].num_candidate_tris = 0;
	
	subholes[i].candidate_edges = NULL;
	subholes[i].candidate_tris = NULL;
	subholes[i].num_candidate_edges = 0;
	

	subholes[i].solution_tris = hole->solution_tris;
	subholes[i].num_solution_tris = -1;
	subholes[i].num_solution_tris_ptr = hole->num_solution_tris_ptr;
    }

    for (i=0, tri_vert_id = -1; i < hole->num_verts; i++)
    {
	hole->verts[i]->handy_mark = 0;
	if (hole->verts[i] == new_tri->verts[0])
	    tri_vert_id = i;
    }

    if (tri_vert_id == -1)
    {
	printf("recursive_fill_hole(): couldn't find aptri_vert in bverts\n");
	exit(-1);
    }

    for (i=0; i<3; i++)
    {
	for (j=tri_vert_id, k = 0, vert = NULL;
	     vert  != new_tri->verts[(i+1)%3];
	     j=(j+1)%hole->num_verts, k++)
	{
	    vert = hole->verts[j];
#ifndef REMOVE_BORDER_VERTS
	    if (vert != new_tri->verts[(i+1)%3])
#else
	    if ((vert != new_tri->verts[(i+1)%3]) && (j < hole->num_edges))
#endif
		edge = hole->edges[j];
	    else
	    {
		edge = new_tri->edges[i];
		tri_vert_id = j;
	    }
	    subholes[i].verts[k] = vert;
	    subholes[i].edges[k] = edge;
	    vert->handy_mark |= 1<<i;
	}
	subholes[i].num_verts = k;
	if (k>2)
	    subholes[i].num_edges = k;
	else if (k==2)
	    subholes[i].num_edges = 1;
	else
	{
	    printf("recursive_fill_hole(): not enough bverts\n");
	    exit(-1);
	}
    }

    
    subholes[0].num_candidate_tris =
	subholes[1].num_candidate_tris =
	    subholes[2].num_candidate_tris = 0;
#ifdef REMOVE_BORDER_VERTS
    if (hole->num_verts == (hole->num_edges+1))
	start_tri = 0;
    else
#endif
	start_tri = tri_index+1;
    for (i = start_tri; i<hole->num_candidate_tris; i++)
    {
	tri = hole->ranked_candidate_tris[i];
	j = (tri->verts[0]->handy_mark &
	     tri->verts[1]->handy_mark &
	     tri->verts[2]->handy_mark);
	if (j)
	{
	    k = (j==1) ? 0 : ((j==2) ? 1 : 2);
	    subholes[k].ranked_candidate_tris[subholes[k].num_candidate_tris++]
		= tri;
	}
    }
    
    return;

} 



static void destroy_subholes(Hole *subholes)
{
    int i;

    for (i=0; i<3; i++)
    {
	subholes[i].center_vert = NULL;
	FREE(subholes[i].verts);
	FREE(subholes[i].edges);
	subholes[i].candidate_edges = NULL;
	subholes[i].candidate_tris = NULL;
	FREE(subholes[i].ranked_candidate_tris);
	subholes[i].solution_tris = NULL;
	subholes[i].num_verts = 0;
	subholes[i].num_edges = 0;
	subholes[i].num_candidate_edges = 0;
	subholes[i].num_candidate_tris = 0;
	subholes[i].num_solution_tris = 0;
	subholes[i].num_solution_tris_ptr = NULL;
    }
} 




static void add_tri_to_hole_solution(Hole *hole, Triangle *tri)
{
    int i;
    
    for (i=0; i<3; i++)
	tri->edges[i]->handy_mark++;
    hole->solution_tris[(*hole->num_solution_tris_ptr)++] = tri;
    return;
} 



static void revert_hole_solution(Hole *hole, int prev_solution_size)
{
    int       i;
    Triangle *tri;

    while (*hole->num_solution_tris_ptr > prev_solution_size)
    {
	tri = hole->solution_tris[(*hole->num_solution_tris_ptr)-1];
	for (i=0; i<3; i++)
	    tri->edges[i]->handy_mark--;
	(*hole->num_solution_tris_ptr)--;
    }
    return;
} 




