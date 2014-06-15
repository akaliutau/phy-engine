





#include <vertex_removal.h>
#include <geometry.h>
#include <simplify.h>
#include <fill_hole.h>
#include <candidate.h>

#ifdef USE_TRIOCTREE
#include <trioctree.h>
#else
#include <octree.h>
#endif

#include <intersect.h>
#include <values.h>
#include <stdio.h>
#include <stdlib.h>



#define COS_TEST_ANGLE -0.98480775  



#define BORDER_VERT(vert) (vert->num_edges == (vert->num_tris+1))






static void get_hole_boundary(Vertex *vert, Hole *hole);
static void destroy_hole(Hole *hole);
static void update_model(Vertex *center_vert, Hole *hole,
			 Octree *model_octree, OctreeData *model_octree_data);
static int border_condition_violated(Vertex *vert, Octree *tubes,
				     OctreeData **query_result,
				     double fuzz_factor);







int try_removing_vertex(Vertex *vert, Octree *outer_offset,
			Octree *inner_offset, Octree *tubes,
			Octree *model_octree, OctreeData *model_octree_data,
			OctreeData **query_result, double fuzz_factor,
			int exhaustive_filling_flag)
{
    Hole hole;
    int  vertex_removed;

#ifdef REMOVE_BORDER_VERTS
    if ((BORDER_VERT(vert)) &&
	(border_condition_violated(vert, tubes, query_result, fuzz_factor)))
	return FALSE;

#else
    if (BORDER_VERT(vert))
	return FALSE;
#endif

    
    get_hole_boundary(vert, &hole);

    
    compute_candidates(&hole, outer_offset, inner_offset, query_result,
		       fuzz_factor);
    
    
    vertex_removed =
	fill_hole(&hole, outer_offset, inner_offset, model_octree,
		  query_result, fuzz_factor, exhaustive_filling_flag);
    
    
    if (vertex_removed == TRUE)
	update_model(vert, &hole, model_octree, model_octree_data);
    
    
    destroy_hole(&hole);
    
    return vertex_removed;
} 



static void get_hole_boundary(Vertex *vert, Hole *hole)
{
    int       i, j;
    Triangle *tri;
    Edge     *edge;
    
    hole->center_vert = vert;
    
    
    ALLOCN(hole->edges, Edge *, vert->num_tris);
    hole->num_edges = 0;
    for(i = 0; i < vert->num_tris; i++)
    {
	tri = vert->tris[i];
	for(j = 0; j < 3; j++)
	{
	    if ((tri->edges[j]->verts[0] != vert) &&
		(tri->edges[j]->verts[1] != vert))
		hole->edges[hole->num_edges++] = tri->edges[j];
	}
    }
    if (hole->num_edges != vert->num_tris)
    {
	fprintf(stderr, "Error getting hole boundary edges\n");
	exit(1);
    }
    
    
    
    ALLOCN(hole->verts, Vertex *, vert->num_edges);
    hole->num_verts = 0;
    for (i=0; i<vert->num_edges; i++)
    {
	edge = vert->edges[i];
	hole->verts[hole->num_verts++] =
	    (edge->verts[0] == vert) ? edge->verts[1] : edge->verts[0];
    }
    
    return;
} 


static void destroy_hole(Hole *hole)
{
    FREE(hole->verts);
    FREE(hole->edges);
    FREE(hole->ranked_candidate_tris);
    FREE(hole->solution_tris);
    FREE(hole->candidate_tris);
    FREE(hole->candidate_edges);
    return;
} 



static void update_model(Vertex *center_vert, Hole *hole,
			 Octree *model_octree, OctreeData *model_octree_data)
{
    int      	i, j, k, m;
    Edge       *src_edge, *dest_edge;
    int      	src_edge_id, src_tri_id, dest_edge_id, dest_tri_id;
    Triangle   *src_tri, *dest_tri, *tri;
    Vertex     *vert, *other_vert;
    int      	num_edge_copies;
    Triangle   *solution_tri, *boundary_tri;
    Edge       *edge, *solution_edge, *boundary_edge;
    int      	new_num_tris, new_num_edges, done, found;
#ifndef USE_TRIOCTREE
    OctreeData *element;
#endif
    
    
#ifndef REMOVE_BORDER_VERTS
    if (hole->num_solution_tris != (center_vert->num_tris - 2))
    {
	fprintf(stderr, "update_model(): f(i+1) != f(i) - 2\n");
	exit(1);
    }
#else
    if (!BORDER_VERT(center_vert))
    {
	if (hole->num_solution_tris != (center_vert->num_tris - 2))
	{
	    fprintf(stderr, "update_model(): f(i+1) != f(i) - 2\n");
	    exit(1);
	}
    }
    else
    {
	if (hole->num_solution_tris != (center_vert->num_tris - 1))
	{
	    fprintf(stderr, "update_model(): f(i+1) != f(i) - 1\n");
	    exit(1);
	}
    }
#endif


#ifdef USE_TRIOCTREE
    for (i=0; i<center_vert->num_tris; i++)
    {
	tri = center_vert->tris[i];
	trioctree_remove(model_octree, tri);
    }    
#else
    for (i=0; i<center_vert->num_tris; i++)
    {
	tri = center_vert->tris[i];
	octree_remove(model_octree, &(model_octree_data[tri->id]));
    }
#endif


    
    for (i=0; i < hole->num_solution_tris; i++)
	for (j=0; j<3; j++)
	    for (k=0; k<2; k++)
		hole->solution_tris[i]->edges[j]->tris[k] = NULL;
    for (i=0; i < hole->num_solution_tris; i++)
	for (j=0; j<3; j++)
	{
	    tri = hole->solution_tris[i];
	    if (!(tri->edges[j]->tris[0]))
		tri->edges[j]->tris[0] = tri;
	    else if (!(tri->edges[j]->tris[1]))
		tri->edges[j]->tris[1] = tri;
	    else
	    {
		fprintf(stderr, "update_model: too many tris on edge\n");
		exit(1);
	    }
	}
    



    
    

    
    for (i=0; i < hole->num_solution_tris; i++)
	for (j=0; j<3; j++)
	    hole->solution_tris[i]->edges[j]->handy_mark = 1;

#ifdef REMOVE_BORDER_VERTS
    
#endif
    
    for (i=0; i < hole->num_solution_tris; i++)
    {
	solution_tri = hole->solution_tris[i];
		
	for (j=0; j<3; j++)
	{
	    solution_edge = solution_tri->edges[j];

	    for (k=0; k< hole->num_edges; k++)
	    {
		boundary_edge = hole->edges[k];
		
		if (((solution_edge->verts[0] == boundary_edge->verts[0]) &&
		     (solution_edge->verts[1] == boundary_edge->verts[1])) ||
		    
		    ((solution_edge->verts[0] == boundary_edge->verts[1]) &&
		     (solution_edge->verts[1] == boundary_edge->verts[0])))
		{
		    solution_edge->handy_mark = 0;
		    solution_edge->id = (int)boundary_edge;
		    
		    for (m=0; m<2; m++)
		    {
			boundary_tri = boundary_edge->tris[m];
			
			if (!(boundary_tri))
			    continue;
			if ((boundary_tri->verts[0] == center_vert) ||
			    (boundary_tri->verts[1] == center_vert) ||
			    (boundary_tri->verts[2] == center_vert))
			    boundary_edge->tris[m] = solution_tri;
		    }
		}
	    }
	}
    }
    
    
    for (i=0; i < hole->num_edges; i++)
	hole->edges[i]->handy_mark = 0; 
    for (i=0; i < center_vert->num_edges; i++)
	center_vert->edges[i]->handy_mark = 1; 
    
    
    dest_tri_id = 0;
    dest_edge_id = 0;
    src_tri_id = 0;
    src_edge_id = 0;
#ifndef REMOVE_BORDER_VERTS
    num_edge_copies =
	center_vert->num_tris - 3;
#else
    num_edge_copies =
        (BORDER_VERT(center_vert)) ? center_vert->num_tris-1: center_vert->num_tris-3;
#endif
    for (i=0; i<num_edge_copies; i++)
    {

	
	src_edge = NULL;
	while (!(src_edge))
	{
	    edge = hole->solution_tris[src_tri_id]->edges[src_edge_id];
	    
	    if (edge->handy_mark)
	    {
		src_edge = edge;
		src_edge->handy_mark = 0;
	    }
	    src_edge_id = (src_edge_id+1)%3;
	    if (!(src_edge_id))
		src_tri_id++;
	}
	
	
	dest_edge = NULL;
	while (!(dest_edge))
	{
	    edge = center_vert->tris[dest_tri_id]->edges[dest_edge_id];
	    
	    if (edge->handy_mark)
	    {
		dest_edge = edge;
		dest_edge->handy_mark = 0;
	    }
	    dest_edge_id = (dest_edge_id+1)%3;
	    if (!(dest_edge_id))
		dest_tri_id++;
	}
	
	
	
	

	
	src_edge->id = (int)dest_edge;
	
	
	dest_edge->verts[0] = src_edge->verts[0];
	dest_edge->verts[1] = src_edge->verts[1];

	
	dest_edge->tris[0] = src_edge->tris[0];
	dest_edge->tris[1] = src_edge->tris[1];
	
	
	dest_edge->handy_mark = src_edge->handy_mark;
    }

#ifdef REMOVE_BORDER_VERTS
    if ((BORDER_VERT(center_vert)) && (center_vert->num_tris == 1))
    {
	tri = center_vert->tris[0];
	for (i=0; i<3; i++)
	{
	    edge = tri->edges[i];
	    if (edge->tris[0] == tri)
	    {
		edge->tris[0] = edge->tris[1];
		edge->tris[1] = NULL;
	    }
	    else if (edge->tris[1] == tri)
		edge->tris[1] = NULL;
	}
    }
#endif

    
    
    for (i=0; i < hole->num_solution_tris; i++)
    {
	dest_tri = center_vert->tris[i];
	src_tri = hole->solution_tris[i];

	

	
	src_tri->id = dest_tri->id;
	
	
	VEC3_ASN_OP(dest_tri->verts, =, src_tri->verts);

	
	for (j=0; j<3; j++)
	{
	    dest_tri->edges[j] = (Edge *)src_tri->edges[j]->id;
	    for (k=0; k<2; k++)
		
		if (dest_tri->edges[j]->tris[k] == src_tri) 
		    dest_tri->edges[j]->tris[k] = dest_tri;
	}
	
	
	VEC_ASN_OP(dest_tri->plane_eq, =, src_tri->plane_eq, 4);

	
	dest_tri->handy_mark = src_tri->handy_mark;
    }



    
    


    
    for (i=0; i<hole->num_verts; i++)
	hole->verts[i]->handy_mark = 0;
    for (i=0; i<hole->num_solution_tris; i++)
	for (j=0; j<3; j++)
	    hole->solution_tris[i]->verts[j]->handy_mark++;

    
    for (i=0; i < hole->num_verts; i++)
    {
	vert = hole->verts[i];
	new_num_tris = vert->num_tris - 2 + vert->handy_mark;
	if (BORDER_VERT(vert))
	    new_num_edges = new_num_tris + 1;
	else
	    new_num_edges = new_num_tris;

#ifdef REMOVE_BORDER_VERTS
	if ((BORDER_VERT(center_vert)) && ((i==0) || (i==hole->num_verts-1)))
	{
	    new_num_tris = vert->num_tris - 1 + vert->handy_mark;
	    if (BORDER_VERT(vert))
		new_num_edges = new_num_tris + 1;
	    else
		new_num_edges = new_num_tris;	    
	}
#endif

	
	if (!(BORDER_VERT(vert)))
	{
	    
	    edge = hole->edges[i];

	    if (edge->verts[0] == vert)
		other_vert = edge->verts[1];
	    else if (edge->verts[1] == vert)
		other_vert = edge->verts[0];
	    else
	    {
		fprintf(stderr,
		   "update_model: couldn't find vert on first edge\n");
		exit(1);
	    }

	    for (j=0; j<2; j++)
	    {
		tri = edge->tris[j];
		
		for (k=0, done = FALSE; k<3; k++)
		    if ((tri->verts[k] == vert) &&
			(tri->verts[(k+1)%3] == other_vert))
		    {
			done = TRUE;
			break;
		    }
		if (done == TRUE)
		    break;
	    }
	    
	    if (done != TRUE)
	    {
		fprintf(stderr,
		      "update_model: can't find first face to trace vertex\n");
		exit(1);
	    }
	}
	else
	{
	    
	    edge = vert->edges[0];

#ifdef REMOVE_BORDER_VERTS
	    
	    if ((BORDER_VERT(center_vert)) &&
		((edge->verts[0] == center_vert) ||
		 (edge->verts[1] == center_vert)))
	    {
		if (i != (hole->num_verts-1))
		{
		    fprintf(stderr,
		       "update_model: bad border condition\n");
		    exit(1);
		}

		if (center_vert->num_tris == 1)
		{
		    
		    edge = vert->edges[1];
		    if (!(((edge->verts[0] == hole->verts[0]) &&
			   (edge->verts[1] ==
			    hole->verts[hole->num_verts-1])) ||
			  ((edge->verts[1] == hole->verts[0]) &&
			   (edge->verts[0] ==
			    hole->verts[hole->num_verts-1]))))
		    {
			fprintf(stderr, "update_model: couldn't find new ");
			fprintf(stderr, "boundary edge for special case\n");
			exit(1);
		    }
		}
		else
		{
		    for (j=0, found=FALSE; j<center_vert->num_edges; j++)
		    {
			edge = center_vert->edges[j];
			if (((edge->verts[0] == hole->verts[0]) &&
			     (edge->verts[1] ==
			      hole->verts[hole->num_verts-1])) ||
			    ((edge->verts[1] == hole->verts[0]) &&
			     (edge->verts[0] ==
			      hole->verts[hole->num_verts-1])))
			{
			    found = TRUE;
			    break;
			}
		    }
		    if (found != TRUE)
		    {
			fprintf(stderr,
				"update_model: couldn't find new boundary edge\n");
			exit(1);
		    }
		}
	    }
#endif
	    tri = edge->tris[0];
	    if (edge->tris[1] != NULL)
	    {
		fprintf(stderr, "update_model: border edge with 2 tris\n");
		exit(1);
	    }
	}
	
	
	REALLOCN(vert->tris, Triangle *, vert->num_tris, new_num_tris);
	REALLOCN(vert->edges, Edge *, vert->num_edges, new_num_edges);

	if (BORDER_VERT(vert))
	{
	    vert->edges[0] = edge;
	    vert->num_edges = 1;
	}
	else
	    vert->num_edges = 0;

	vert->tris[0] = tri;
	vert->num_tris = 1;
	
	for (j=0, found=FALSE; j<3; j++)
	{
	    if (tri->edges[j] == edge)
	    {
		edge = tri->edges[(j+2)%3];
		found = TRUE;
		break;
	    }
	}
	if (found != TRUE)
	{
	    fprintf(stderr, "update_model: couldn't find next edge\n");
	    exit(1);
	}
	vert->edges[vert->num_edges++] = edge;

	
	while (vert->num_edges != new_num_edges)
	{
	    if (edge->tris[0] == tri)
		tri = edge->tris[1];
	    else if (edge->tris[1] == tri)
		tri = edge->tris[0];
	    else
	    {
		fprintf(stderr, "update_model: couldn't find next tri\n");
		exit(1);
	    }
	    if (tri == NULL)
	    {
		fprintf(stderr, "update_model: next tri is NULL pointer\n");
		exit(1);
	    }
	    
	    vert->tris[vert->num_tris++] = tri;
		

	    for (j=0, found=FALSE; j<3; j++)
	    {
		if (tri->edges[j] == edge)
		{
		    edge = tri->edges[(j+2)%3];
		    found = TRUE;
		    break;
		}
	    }
	    if (found != TRUE)
	    {
		fprintf(stderr, "update_model: couldn't find next edge\n");
		exit(1);
	    }
	    vert->edges[vert->num_edges++] = edge;

	}
    }



    

#ifdef USE_TRIOCTREE
    for (i=0; i<hole->num_solution_tris; i++)
    {
	tri = center_vert->tris[i];
	trioctree_insert(model_octree, tri);
    }
#else
    for (i=0; i<hole->num_solution_tris; i++)
    {
	tri = center_vert->tris[i];
	element = &(model_octree_data[tri->id]);
	for (j=0; j<3; j++)
	{
	    element->bbox[LO][j] = MAXDOUBLE;
	    element->bbox[HI][j] = -MAXDOUBLE;
	    for (k=0; k<3; k++)
	    {
		element->bbox[LO][j] =
		    FMIN(element->bbox[LO][j],
			 tri->verts[k]->coord[j]);
		element->bbox[HI][j] =
		    FMAX(element->bbox[HI][j],
			 tri->verts[k]->coord[j]);
	    }
	}
	element->data = tri;

	octree_insert(model_octree, element);
    }
#endif    




    
    FREE(center_vert->tris);
    FREE(center_vert->edges);
    center_vert->num_tris = center_vert->num_edges = 0;

    return;
} 




static int border_condition_violated(Vertex *vert, Octree *tubes,
				     OctreeData **query_result,
				     double fuzz_factor)
{
    Edge   *edge;
    Vertex *vert1, *vert2;
#if (BORDER_TEST == TUBES_TEST)
    Edge    an_edge;
#else
    Vector  vec1, vec2;
    double  dot;
#endif
    
    
    
    edge = vert->edges[0];
    if (edge->verts[0] == vert)
	vert1 = edge->verts[1];
    else if (edge->verts[1] == vert)
	vert1 = edge->verts[0];
    else
    {
	fprintf(stderr,
	 "border_condition_violated: couldn't find vert on border edge\n");
	exit(1);
    }

    edge = vert->edges[vert->num_edges-1];
    if (edge->verts[0] == vert)
	vert2 = edge->verts[1];
    else if (edge->verts[1] == vert)
	vert2 = edge->verts[0];
    else
    {
	fprintf(stderr,
	 "border_condition_violated: couldn't find vert on border edge\n");
	exit(1);
    }

    
    
#if (BORDER_TEST == TUBES_TEST)
    an_edge.verts[0] = vert1;
    an_edge.verts[1] = vert2;
    return edge_tubes_intersect(&an_edge, tubes,
				query_result, fuzz_factor);
#elif (BORDER_TEST == ANGLE_TEST)
    VEC3_V_OP_V(vec1, vert1->coord, -, vert->coord);
    VEC3_V_OP_V(vec2, vert2->coord, -, vert->coord);
    NORMALIZE3(vec1);
    NORMALIZE3(vec2);
    dot = DOTPROD3(vec1, vec2);
    return ((dot > COS_TEST_ANGLE) ? TRUE : FALSE);
#elif (BORDER_TEST == XY_ANGLE_TEST)
    VEC3_V_OP_V(vec1, vert1->coord, -, vert->coord);
    VEC3_V_OP_V(vec2, vert2->coord, -, vert->coord);
    vec1[Z] = vec2[Z] = 0.0;
    NORMALIZE3(vec1);
    NORMALIZE3(vec2);
    dot = DOTPROD3(vec1, vec2);
    return ((dot > COS_TEST_ANGLE) ? TRUE : FALSE);
#else
    INVALID_CHOICE_OF_BORDER_TEST!!!!!
#endif
} 



