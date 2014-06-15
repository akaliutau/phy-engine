





#include <stdio.h>
#include <math.h>
#include <values.h>
#include <simplify.h>

#ifdef USE_TRIOCTREE
#include <trioctree.h>
#else
#include <octree.h>
#endif

#include <offset.h>
#include <ply.h>
#include <geometry.h>
#include <intersect.h>



#define MIN_STEPS (10)
#define MAX_SUBDIVISIONS (20)

#define COS_89_5 (0.0087265355)



#define ABS(a) (((a) >= 0) ? (a) : (-a))






void init_offset_surface(Surface *model, Surface *offset);
int bad_normal(Vertex *vert);
void offset_vertex(Vertex *orig_vert, Vertex *offset_vert, double epsilon,
		   Octree *octree, OctreeData *octdata,
		   OctreeData **query_result, double fuzz_factor,
		   char *frozen, double *vert_epsilon);















void init_offset_surface(Surface *model, Surface *offset)
{
    int      	 i, j;
    Vertex   	*src_vert, *dest_vert;
    Edge     	*src_edge, *dest_edge;
    Triangle 	*src_tri, *dest_tri;
    
    ALLOCN(offset->verts, Vertex, model->num_verts);
    ALLOCN(offset->tris, Triangle, model->num_tris);
    ALLOCN(offset->edges, Edge, model->num_edges);
    
    for (i=0; i<model->num_verts; i++)
    {
	src_vert = &(model->verts[i]);
	dest_vert = &(offset->verts[i]);
	dest_vert->id = src_vert->id;
	VEC3_ASN_OP(dest_vert->coord, =, src_vert->coord);
	VEC3_ASN_OP(dest_vert->normal, =, src_vert->normal);
	ALLOCN(dest_vert->tris, Triangle *, src_vert->num_tris);
	for (j=0; j<src_vert->num_tris; j++)
	    dest_vert->tris[j] = &(offset->tris[src_vert->tris[j]->id]);
	dest_vert->num_tris = src_vert->num_tris;
	ALLOCN(dest_vert->edges, Edge *, src_vert->num_edges);
	for (j=0; j<src_vert->num_edges; j++)
	    dest_vert->edges[j] = &(offset->edges[src_vert->edges[j]->id]);
	dest_vert->num_edges = src_vert->num_edges;
	dest_vert->other_props = src_vert->other_props;
#ifdef VERTEX_EPSILONS
        dest_vert->epsilon = src_vert->epsilon;
#endif
    }
    offset->num_verts = model->num_verts;

    for (i=0; i<model->num_edges; i++)
    {
	src_edge = &(model->edges[i]);
	dest_edge = &(offset->edges[i]);
	dest_edge->id = src_edge->id;
	for (j=0; j<2; j++)
	{
	    dest_edge->verts[j] = &(offset->verts[src_edge->verts[j]->id]);
	    if (src_edge->tris[j] != NULL)
		dest_edge->tris[j] = &(offset->tris[src_edge->tris[j]->id]);
	    else
		dest_edge->tris[j] = NULL;
	}
    }
    offset->num_edges = model->num_edges;

    for (i=0; i<model->num_tris; i++)
    {
	src_tri = &(model->tris[i]);
	dest_tri = &(offset->tris[i]);
	dest_tri->id = src_tri->id;
	for (j=0; j<3; j++)
	{
	    dest_tri->verts[j] = &(offset->verts[src_tri->verts[j]->id]);
	    dest_tri->edges[j] = &(offset->edges[src_tri->edges[j]->id]);
	}
	VEC_ASN_OP(dest_tri->plane_eq, =, src_tri->plane_eq, 4);
    }
    offset->num_tris = model->num_tris;

    return;
} 



int bad_normal(Vertex *vert)
{
    int    i;

    for (i=0; i<vert->num_tris; i++)
	if (DOTPROD3(vert->normal, vert->tris[i]->plane_eq) < COS_89_5)
	    return TRUE;
    return FALSE;
    
} 



void offset_vertex(Vertex *orig_vert, Vertex *offset_vert, double epsilon,
		   Octree *octree, OctreeData *octdata,
		   OctreeData **query_result, double fuzz_factor,
		   char *frozen, double *vert_epsilon)
{
    int      	 i, j;
#ifndef USE_TRIOCTREE
    int          k;
    OctreeData  *element;
    Extents      tri_bbox;
    double       box_fuzz;
#endif
    Point    	 new_coord, prev_coord;
    Vector   	 displacement;
    int      	 query_count;
    Triangle    *tri, *tri2;
    int          intersect;
    
    if (bad_normal(offset_vert) == TRUE)
    {
	*frozen = TRUE;
	return;
    }

    VEC3_V_OP_S(displacement, orig_vert->normal, *, epsilon);
    VEC3_V_OP_V(new_coord, orig_vert->coord, +, displacement);

    VEC3_ASN_OP(prev_coord, =, offset_vert->coord);
    VEC3_ASN_OP(offset_vert->coord, =, new_coord);

    

    
    for (i=0; i<offset_vert->num_tris; i++)
    {
	tri = offset_vert->tris[i];
	find_plane_eq(tri->verts[0]->coord,
		      tri->verts[1]->coord,
		      tri->verts[2]->coord,
		      tri->plane_eq);
    }
    
#if 0
    
    

    for (i=0, intersect = FALSE;
	 ((i<offset_vert->num_tris-1) && (intersect == FALSE)); i++)
	for (j=i+1; ((j<offset_vert->num_tris) && (intersect == FALSE)); j++)
	{
	    intersect =
		index_filtered_tri_tri_intersect(offset_vert->tris[i],
						 offset_vert->tris[j],
						 fuzz_factor,
						 SURFACE_VS_ITSELF);
	}

    
    
    if (intersect == TRUE)
    {
	VEC3_ASN_OP(offset_vert->coord, =, prev_coord);
	for (i=0; i<offset_vert->num_tris; i++)
	{
	    tri = offset_vert->tris[i];
	    find_plane_eq(tri->verts[0]->coord,
			  tri->verts[1]->coord,
			  tri->verts[2]->coord,
			  tri->plane_eq);
	}
	*frozen = TRUE;
	return;
    }
    
#endif

    
    
    for (i=0, intersect=FALSE; ((i<offset_vert->num_tris) && (intersect == FALSE)); i++)
    {
	tri = offset_vert->tris[i];
	
#ifdef USE_TRIOCTREE
	trioctree_tri_range_query(octree, tri, query_result, &query_count);
#else
	tri_bbox[HI][X] = tri_bbox[HI][Y] = tri_bbox[HI][Z] = -MAXDOUBLE;
	tri_bbox[LO][X] = tri_bbox[LO][Y] = tri_bbox[LO][Z] =  MAXDOUBLE;
	for (j=0; j<3; j++)
	    for (k=0; k<3; k++)
	    {
		tri_bbox[HI][k] =
		    FMAX(tri_bbox[HI][k], tri->verts[j]->coord[k]);
		tri_bbox[LO][k] =
		    FMIN(tri_bbox[LO][k], tri->verts[j]->coord[k]);
	    }

	if (fuzz_factor < 0.0)
	    for (j=0; j<3; j++)
	    {
		box_fuzz = (tri_bbox[HI][j] - tri_bbox[LO][j])*-fuzz_factor;
		tri_bbox[HI][j] += box_fuzz;
		tri_bbox[LO][j] -= box_fuzz;
	    }
	
	octree_range_query(octree, tri_bbox, query_result, &query_count);
#endif

	for (j=0; ((j<query_count) && (intersect == FALSE)); j++)
	{
#ifdef USE_TRIOCTREE
	    tri2 = query_result[j];
#else
	    tri2 = (Triangle *)(query_result[j]->data);
#endif
	    
	    if ((tri2->verts[0] == offset_vert) ||
		(tri2->verts[1] == offset_vert) ||
		(tri2->verts[2] == offset_vert))
		continue;
	    
	    intersect =
		index_filtered_tri_tri_intersect(tri, tri2, fuzz_factor,
						 SURFACE_VS_ITSELF);
	}
    }

    
    if (intersect == TRUE)
    {
	VEC3_ASN_OP(offset_vert->coord, =, prev_coord);
	for (i=0; i<offset_vert->num_tris; i++)
	{
	    tri = offset_vert->tris[i];
	    find_plane_eq(tri->verts[0]->coord,
			  tri->verts[1]->coord,
			  tri->verts[2]->coord,
			  tri->plane_eq);
	}
	*frozen = TRUE;
	return;
    }


    
#ifdef USE_TRIOCTREE
    
    VEC3_ASN_OP(offset_vert->coord, =, prev_coord);
    for (i=0; i<offset_vert->num_tris; i++)
    {
	tri = offset_vert->tris[i];
	find_plane_eq(tri->verts[0]->coord,
		      tri->verts[1]->coord,
		      tri->verts[2]->coord,
		      tri->plane_eq);
	trioctree_remove(octree, tri);
    }

    
    VEC3_ASN_OP(offset_vert->coord, =, new_coord);
    for (i=0; i<offset_vert->num_tris; i++)
    {
	tri = offset_vert->tris[i];
	find_plane_eq(tri->verts[0]->coord,
		      tri->verts[1]->coord,
		      tri->verts[2]->coord,
		      tri->plane_eq);
	trioctree_insert(octree, tri);
    }
#else
    for (i=0; i<offset_vert->num_tris; i++)
    {
	tri = offset_vert->tris[i];
	element = &(octdata[tri->id]);

	octree_remove(octree, element);
	
	for (j=0; j<3; j++)
	{
	    element->bbox[HI][j] =
		FMAX(FMAX(tri->verts[0]->coord[j], tri->verts[1]->coord[j]),
		     tri->verts[2]->coord[j]);
	    element->bbox[LO][j] =
		FMIN(FMIN(tri->verts[0]->coord[j], tri->verts[1]->coord[j]),
		     tri->verts[2]->coord[j]);
	}
	octree_insert(octree, element);
    }
#endif
		
    *vert_epsilon = epsilon;
			
    return;
} 




static double min_vert_edge_length(Vertex *vert, double epsilon)
{
    int     i;
    Vertex *other_vert;
    Edge   *edge;
    double  sq_length, min_sq_length, length;
    Vector  edge_vec;
    double  dot;
    
    for (i=0, min_sq_length = MAXDOUBLE; i<vert->num_edges; i++)
    {
	edge = vert->edges[i];
	if (edge->verts[0] == vert)
	    other_vert = edge->verts[1];
	else if (edge->verts[1] == vert)
	    other_vert = edge->verts[0];
	else
	{
	    fprintf(stderr, "min_vert_edge_length: couldn't find other vert\n");
	    exit(-1);
	}

	VEC3_V_OP_V(edge_vec, other_vert->coord, -, vert->coord);
	dot = DOTPROD3(edge_vec, vert->normal);
	if ((dot*epsilon) < 0)
	    continue;
	sq_length = DOTPROD3(edge_vec, edge_vec);
	if (sq_length <= 0.0)
	{
	    fprintf(stderr, "min_vert_edge_length: degenerate edge\n");
	    exit(-1);
	}

	min_sq_length = FMIN(min_sq_length, sq_length);
    }
    length = sqrt(min_sq_length);
    return length;
} 




void offset_surface(Surface *model, Surface *offset, double epsilon,
		    int max_node_objs_hint, double fuzz_factor)
{
    char   	 *vert_frozen;
    double 	 *vert_epsilon;
    Octree       *octree;
    OctreeData   *octdata;
    OctreeData  **query_result;
    int           i, j;
    Vector        box_length;
    double        diag_length, percent_epsilon;
    double       *vert_step_size, max_step_size, min_step_size;
    int           k, num_moving_verts, max_iterations, min_steps, cheated_verts;
    double        new_epsilon, min_edge;
    
    fprintf(stderr, "Epsilon: %f\n", epsilon);
    
    init_offset_surface(model, offset);

    surface_to_octree(offset, &octree, &octdata, fabs(epsilon)*1.01,
		      max_node_objs_hint);

    VEC3_V_OP_S(box_length, octree->root->sides, -, fabs(epsilon)*1.01);
    VEC3_V_OP_S(box_length, box_length, *, 2.0);
    
    diag_length = sqrt(DOTPROD3(box_length, box_length));
    
    percent_epsilon = fabs(epsilon/diag_length * 100.0);



    ALLOCN(query_result, OctreeData *, offset->num_tris);
    
    ALLOCN(vert_frozen, char, offset->num_verts);
    ALLOCN(vert_epsilon, double, offset->num_verts);
    
    ALLOCN(vert_step_size, double, offset->num_verts);
    
    min_steps = FMAX(percent_epsilon*MIN_STEPS, MIN_STEPS);
    max_step_size = (fabs(epsilon) * 0.99)/(double)min_steps;
    min_step_size = max_step_size;
    
    
    for (i=0; i<offset->num_verts; i++)
    {
	vert_frozen[i] = FALSE;
	vert_epsilon[i] = 0.0;

	min_edge = min_vert_edge_length(&(offset->verts[i]), epsilon);
	
#ifdef VERTEX_EPSILONS
	max_step_size = (fabs(offset->verts[i].epsilon)*0.99)/(double)min_steps;
#endif
	
	vert_step_size[i] = FMIN(0.49 * min_edge, max_step_size) *
	    ((epsilon<0) ? -1.0 : 1.0);
	min_step_size = FMIN(min_step_size, fabs(vert_step_size[i]));
    }
    
    
    
    
    max_iterations = (int)(fabs(epsilon)/min_step_size);
    
    fprintf(stderr, "offsetting vertices (maximum of %d iterations): ",
	    max_iterations);
    
    
    for (i=0, num_moving_verts=offset->num_verts, cheated_verts = 0;
	 ((num_moving_verts > 0) && (i < max_iterations)); i++)
    {
	fprintf(stderr, "%d ", i);
	fflush(stderr);
	
	for (j=0; j<offset->num_verts; j++)
	{
	    if (vert_frozen[j] == TRUE)
		continue;
	    new_epsilon = vert_epsilon[j] + vert_step_size[j];
#ifndef VERTEX_EPSILONS
	    if ((fabs(new_epsilon) < fabs(epsilon)) &&
#else 
	    if ((fabs(new_epsilon) < fabs(offset->verts[j].epsilon)) &&
#endif
		(fabs(new_epsilon) > fabs(vert_epsilon[j])))
		offset_vertex(&(model->verts[j]), &(offset->verts[j]),
			      new_epsilon, octree, octdata, query_result,
			      fuzz_factor, &(vert_frozen[j]),
			      &(vert_epsilon[j]));
	    else
		vert_frozen[j] = TRUE;
	    
	    if ((vert_frozen[j] == TRUE) && (i < MIN_STEPS))
	    {
		for (k=0; ((vert_frozen[j]==TRUE) && (k<MAX_SUBDIVISIONS)); k++)
		{
		    vert_step_size[j] /= 2.0;
		    vert_frozen[j] = FALSE;
		    new_epsilon = vert_epsilon[j] + vert_step_size[j];
#ifndef VERTEX_EPSILONS
		    if ((fabs(new_epsilon) < fabs(epsilon)) &&
#else 
		    if ((fabs(new_epsilon) < fabs(offset->verts[j].epsilon)) &&
#endif
			(fabs(new_epsilon) > fabs(vert_epsilon[j])))
			offset_vertex(&(model->verts[j]),
				      &(offset->verts[j]),
				      new_epsilon, octree, octdata,
				      query_result, fuzz_factor,
				      &(vert_frozen[j]), &(vert_epsilon[j]));
		    else
			vert_frozen[j] = TRUE;
		}
		if (vert_frozen[j]==TRUE)
		    cheated_verts++;
	    }
	    if (vert_frozen[j] == TRUE)
		num_moving_verts--;
	}
	
    }
    
    fprintf(stderr, " -- done\n\n");

    fprintf(stderr, "Cheated verts: %d\n\n", cheated_verts);
    fflush(stderr);

    
    
#ifdef USE_TRIOCTREE
    fprintf(stderr, "Destroying trioctree...");
    trioctree_destroy(&octree);
	    
    fprintf(stderr, "done\n");    
#else
    fprintf(stderr, "Destroying octree...");
    octree_destroy(&octree);
	    
    fprintf(stderr, "done\n");
    FREE(octdata);
#endif
	    
    FREE(query_result);
    FREE(vert_frozen);
    FREE(vert_epsilon);
    FREE(vert_step_size);
    return;
} 


void surface_to_octree(Surface *model, Octree **octree, OctreeData **data,
		       double buffer_width, int max_node_objs_hint)
{
    int         i, j, k;
#ifndef USE_TRIOCTREE
    OctreeData *octdata;
    OctreeData *element;
#endif
    Triangle   *tri;
    Extents     global_bbox;
    double      min_max_side, element_max_side, side_length;
    Vertex     *vert;
    double      mean, deviation, sum_of_square_deviations;
    double      variance, std_deviation;
    int         max_node_objs;
    
#ifdef USE_TRIOCTREE
    Extents     element_bbox;
#endif

    fprintf(stderr, "Creating octree...");
    
#ifndef USE_TRIOCTREE
    ALLOCN(octdata, OctreeData, model->num_tris);
    *data = octdata;

    
    for (i=0; i<model->num_tris; i++)
    {
	tri = &(model->tris[i]);
	element = &(octdata[i]);
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
    }

    
    global_bbox[LO][X] = global_bbox[LO][Y] = global_bbox[LO][Z] = MAXDOUBLE;
    global_bbox[HI][X] = global_bbox[HI][Y] = global_bbox[HI][Z] = -MAXDOUBLE;
    for (i=0; i<model->num_tris; i++)
    {
	for (j=0; j<3; j++)
	{
	    global_bbox[LO][j] =
		FMIN(global_bbox[LO][j], octdata[i].bbox[LO][j]);
	    global_bbox[HI][j] =
		FMAX(global_bbox[HI][j], octdata[i].bbox[HI][j]);
	}
    }
#else
    
    global_bbox[LO][X] = global_bbox[LO][Y] = global_bbox[LO][Z] = MAXDOUBLE;
    global_bbox[HI][X] = global_bbox[HI][Y] = global_bbox[HI][Z] = -MAXDOUBLE;
    for (i=0; i<model->num_tris; i++)
    {
	tri = &(model->tris[i]);
	for (j=0; j<3; j++)
	{
	    for (k=0; k<3; k++)
	    {
		global_bbox[LO][j] =
		    FMIN(global_bbox[LO][j], tri->verts[k]->coord[j]);
		global_bbox[HI][j] =
		    FMAX(global_bbox[HI][j], tri->verts[k]->coord[j]);
	    }
	}
    }
#endif
	
    for (i=0; i<3; i++)
    {
	global_bbox[LO][i] -= (buffer_width);
	global_bbox[HI][i] += (buffer_width);
    }


    
    min_max_side = MAXDOUBLE;
    for (i=0; i<model->num_tris; i++)
    {
	tri = &(model->tris[i]);

#ifdef USE_TRIOCTREE
	for (j=0; j<3; j++)
	{
	    element_bbox[LO][j] = MAXDOUBLE;
	    element_bbox[HI][j] = -MAXDOUBLE;
	    for (k=0; k<3; k++)
	    {
		element_bbox[LO][j] =
		    FMIN(element_bbox[LO][j],
			 tri->verts[k]->coord[j]);
		element_bbox[HI][j] =
		    FMAX(element_bbox[HI][j],
			 tri->verts[k]->coord[j]);
	    }
	}

	element_max_side = 0;
	for (j=0; j<3; j++)
	{
	    side_length = element_bbox[HI][j] - element_bbox[LO][j];
	    element_max_side = FMAX(element_max_side, side_length);
	}
	
#else
	element = &(octdata[i]);
	
	element_max_side = 0;
	for (j=0; j<3; j++)
	{
	    side_length = element->bbox[HI][j] - element->bbox[LO][j];
	    element_max_side = FMAX(element_max_side, side_length);
	}
#endif
	
	if (element_max_side <= 0)
	{
	    fprintf(stderr,
		    "surface_to_octree: couldn't find element max side\n");
	    exit(1);
	}
	
	min_max_side =
	    FMIN(min_max_side, element_max_side);
    }

    if (min_max_side == MAXDOUBLE)
    {
	fprintf(stderr,
		"surface_to_octree: couldn't find min_max_side\n");
	exit(1);
    }
	     
    
    if (max_node_objs_hint <= 0)
    {
	
	mean = model->num_tris * 3 / model->num_verts;
	
	for (i=0, sum_of_square_deviations = 0.0; i<model->num_verts; i++)
	{
	    vert = &(model->verts[i]);
	    deviation = mean - vert->num_tris;
	    sum_of_square_deviations += deviation*deviation;
	}
	variance = sum_of_square_deviations / model->num_verts;
	std_deviation = sqrt(variance);

	max_node_objs = (int)(mean + 3*std_deviation + 0.99999999) * 1.5;
    }
    else
	max_node_objs = max_node_objs_hint;

    
#ifdef USE_TRIOCTREE
    trioctree_create(octree, global_bbox, min_max_side*2.0, max_node_objs);

    for (i=0; i<model->num_tris; i++)
	trioctree_insert(*octree, &(model->tris[i]));

    fprintf(stderr, "done\n");

    trioctree_print_stats(*octree);
#else
    octree_create(octree, global_bbox, min_max_side*2.0, max_node_objs);

    for (i=0; i<model->num_tris; i++)
	octree_insert(*octree, &(octdata[i]));

    fprintf(stderr, "done\n");

    octree_print_stats(*octree);
#endif
    
    return;
} 




