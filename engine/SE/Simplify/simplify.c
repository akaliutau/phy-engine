





#include <stdio.h>
#include <simplify.h>
#include <removal_queue.h>
#include <vertex_removal.h>

#ifdef USE_TRIOCTREE
#include <trioctree.h>
#else
#include <octree.h>
#endif

#include <offset.h>
#include <intersect.h>
#include <tubes.h>










typedef struct neighbor_list_struct
{
    int *neighbors;
    int  num_neighbors;
} NeighborList;



static void init_solution(Surface *orig_model, Surface *solution);
static void build_removal_queue(Surface *model, RemovalQueue *queue);
static void update_removal_queue(RemovalQueue *queue, int vert_removed,
				 NeighborList *neighbors);
static void get_neighbor_list(Vertex *vert, NeighborList *list);
static void destroy_neighbor_list(NeighborList *list);
static void finalize_solution(Surface *model);

#ifdef VERTEX_EPSILONS
static double max_vertex_epsilon(Surface *model);
#endif









void simplify(Surface *orig_model, Surface **simp_model, double epsilon)
{
    Vertex       *vert;
    double        fuzz_factor;
    Surface       outer_offset, inner_offset;
    Octree       *outer_octree, *inner_octree, *simp_octree;
    OctreeData   *outer_octree_data, *inner_octree_data, *simp_octree_data; 
    OctreeData  **query_result;
    int           prev_queue_size;
    RemovalQueue  removal_queue;
    NeighborList  neighbor_list;
    int           vert_id;
    int           vert_removed, verts_removed;
#ifdef REMOVE_BORDER_VERTS
    Surface       initial_tubes, tubes;
    OctreeData   *tube_octree_data;
    int           num_tube_sides;
#endif
    Octree       *tube_octree;

#ifdef VERTEX_EPSILONS
    epsilon = max_vertex_epsilon(orig_model);
#endif
    
    
    *simp_model = orig_model;
    
    ALLOCN(query_result, OctreeData *, orig_model->num_tris);

    
    fprintf(stderr, "Testing for Self-Intersectsion in Original Surface\n");
    fprintf(stderr, "--------------------------------------------------\n");
    surface_to_octree(*simp_model, &simp_octree, &simp_octree_data,
		      epsilon*1.01, 0);
    test_self_intersect(*simp_model, simp_octree, simp_octree_data,
			query_result, &fuzz_factor);
    fprintf(stderr, "\n\n");
    
    fprintf(stderr, "Creating Outer Offset Surface\n");
    fprintf(stderr, "-----------------------------\n");
    offset_surface(orig_model, &outer_offset, epsilon, 0, fuzz_factor);
    fprintf(stderr, "\n\n");

    fprintf(stderr, "Testing Outer Offset Surface for Self-Intersections\n");
    fprintf(stderr, "---------------------------------------------------\n");
    surface_to_octree(&outer_offset, &outer_octree, &outer_octree_data,
		      epsilon*1.01, 0);
    test_self_intersect(&outer_offset, outer_octree, outer_octree_data,
			query_result, &fuzz_factor);
    fprintf(stderr, "\n\n");

#if 0

    write_file(&outer_offset);
    exit(-1);

#endif
    
    fprintf(stderr, "Creating Inner Offset Surface\n");
    fprintf(stderr, "-----------------------------\n");
    offset_surface(orig_model, &inner_offset, -epsilon, 0, fuzz_factor);
    fprintf(stderr, "\n\n");


    fprintf(stderr, "Testing Inner Offset Surface for Self-Intersections\n");
    fprintf(stderr, "---------------------------------------------------\n");
    surface_to_octree(&inner_offset, &inner_octree, &inner_octree_data,
		      epsilon*1.01, 0);
    test_self_intersect(&inner_offset, inner_octree, inner_octree_data,
			query_result, &fuzz_factor);
    fprintf(stderr, "\n\n");
    

#if 0

    write_file(&inner_offset);
    exit(-1);

#endif



#ifdef REMOVE_BORDER_VERTS
#if (BORDER_TEST == TUBES_TEST)
    fprintf(stderr, "Creating Initial Border Tubes\n");
    fprintf(stderr, "-----------------------------\n");
    num_tube_sides = 6;

    init_tubes(*simp_model, &initial_tubes, epsilon/1000.0, num_tube_sides);
    if (initial_tubes.num_tris > 0)
    {
	fprintf(stderr, "\n\n");
	
	if (initial_tubes.num_tris > orig_model->num_tris)
	{
	    REALLOCN(query_result, OctreeData *, orig_model->num_tris,
		     initial_tubes.num_tris);
	}

	fprintf(stderr,
		"Testing Initial Border Tubes for Self-Intersectsions\n");
	fprintf(stderr,
		"----------------------------------------------------\n");
	surface_to_octree(&initial_tubes, &tube_octree, &tube_octree_data,
			  0.0, num_tube_sides*4);
	test_self_intersect(&initial_tubes, tube_octree, tube_octree_data,
			    query_result, &fuzz_factor);
#ifdef USE_TRIOCTREE
	trioctree_destroy(&tube_octree);
#else
	octree_destroy(&tube_octree);
#endif
	fprintf(stderr, "\n\n");

	fprintf(stderr, "Offsetting Border Tubes\n");
	fprintf(stderr, "-----------------------\n");
	offset_surface(&initial_tubes, &tubes, epsilon-(epsilon/1000.0),
		       num_tube_sides*4, fuzz_factor);
	free_surface(&initial_tubes);
	fprintf(stderr, "\n\n");

	fprintf(stderr, "Testing Border Tubes for Self-Intersections\n");
	fprintf(stderr, "-------------------------------------------\n");
	surface_to_octree(&tubes, &tube_octree, &tube_octree_data,
			  0.0, 0);
	test_self_intersect(&tubes, tube_octree, tube_octree_data,
			    query_result, &fuzz_factor);
	fprintf(stderr, "\n\n");
    }
    else
    {
	tubes.num_tris = 0;
	fprintf(stderr, "No borders on this surface\n");
	fprintf(stderr, "\n\n");
    }
#endif
#endif

    
    fprintf(stderr, "Deleting vertices\n");
    fprintf(stderr, "-----------------\n");

    build_removal_queue(*simp_model, &removal_queue);

    prev_queue_size = removal_queue.size;
    fprintf(stderr, "Removal queue size: \n%d ", prev_queue_size);
    fflush(stderr);

    verts_removed = 0;
    while(removal_queue.size > 0)
    {
	vert_id = removal_queue_extract(&removal_queue);
	vert = &((*simp_model)->verts[vert_id]);
	
	get_neighbor_list(vert, &neighbor_list);

	vert_removed = try_removing_vertex(vert, outer_octree,
					   inner_octree, tube_octree,
					   simp_octree, simp_octree_data, 
					   query_result, fuzz_factor,
					   FALSE);

	if (vert_removed)
	    verts_removed++;

	update_removal_queue(&removal_queue, vert_removed, &neighbor_list);

	if (((removal_queue.size%20) == 0) &&
	    (removal_queue.size != prev_queue_size))
	{
	    prev_queue_size = removal_queue.size;
	    fprintf(stderr, "%d ", prev_queue_size);
	    fflush(stderr);
	}
	destroy_neighbor_list(&neighbor_list);
    }
    
    fprintf(stderr, "\n\n");
    fprintf(stderr, "Removed %d vertices\n\n", verts_removed);
    fflush(stderr);

#ifdef USE_EXHAUSTIVE_FILL_HOLE    
    fprintf(stderr, "Deleting vertices - exhaustive hole-filling pass\n");
    fprintf(stderr, "------------------------------------------------\n");

    build_removal_queue(*simp_model, &removal_queue);

    prev_queue_size = removal_queue.size;
    fprintf(stderr, "Removal queue size: \n%d ", prev_queue_size);
    fflush(stderr);

    verts_removed = 0;
    while(removal_queue.size > 0)
    {
	vert_id = removal_queue_extract(&removal_queue);
	vert = &((*simp_model)->verts[vert_id]);
	
	get_neighbor_list(vert, &neighbor_list);

	vert_removed = try_removing_vertex(vert, outer_octree,
					   inner_octree, tube_octree,
					   simp_octree, simp_octree_data, 
					   query_result, fuzz_factor,
					   TRUE);

	if (vert_removed)
	    verts_removed++;
	
	update_removal_queue(&removal_queue, vert_removed, &neighbor_list);

	if (((removal_queue.size%20) == 0) &&
	    (removal_queue.size != prev_queue_size))
	{
	    prev_queue_size = removal_queue.size;
	    fprintf(stderr, "%d ", prev_queue_size);
	    fflush(stderr);
	}
	destroy_neighbor_list(&neighbor_list);
    }
    fprintf(stderr, "\n\n");
    fprintf(stderr, "Removed %d vertices\n\n", verts_removed);
    fflush(stderr);
#endif

    
    removal_queue_destroy(&removal_queue);
#ifdef USE_TRIOCTREE
    trioctree_destroy(&simp_octree);
    trioctree_destroy(&outer_octree);
    trioctree_destroy(&inner_octree);
#else
    octree_destroy(&simp_octree);
    octree_destroy(&outer_octree);
    octree_destroy(&inner_octree);
    FREE(simp_octree_data);
    FREE(outer_octree_data);
    FREE(inner_octree_data);
#endif
    FREE(query_result);
    free_surface(&outer_offset);
    free_surface(&inner_offset);
#ifdef REMOVE_BORDER_VERTS
#if (BORDER_TEST == TUBES_TEST)
    if (tubes.num_tris > 0)
    {
#ifdef USE_TRIOCTREE
	trioctree_destroy(&tube_octree);
#else
	octree_destroy(&tube_octree);
	FREE(tube_octree_data);
#endif
	free_surface(&tubes);
    }
#endif
#endif
    
    finalize_solution(*simp_model);

    fprintf(stderr, "\nNumber of triangles in output model: %d\n",
	    (*simp_model)->num_tris);
    fprintf(stderr, "\n\n");
    
    return;
} 


#ifdef VERTEX_EPSILONS

static double max_vertex_epsilon(Surface *model)
{
    int i;
    double max_epsilon;

    for (i=0, max_epsilon = 0.0; i<model->num_verts; i++)
    {
        max_epsilon = FMAX(max_epsilon, fabs(model->verts[i].epsilon));
    }
    if (max_epsilon == 0.0)
    {
        fprintf(stderr, "maximum vertex epsilon is 0\n");
        exit(-1);
    }
    return max_epsilon;
} 
#endif



static void init_solution(Surface *orig_model, Surface *solution)
{
    int       i, j;
    Vertex   *src_vert, *dest_vert;
    Edge     *src_edge, *dest_edge;
    Triangle *src_tri, *dest_tri;
    
    ALLOCN(solution->verts, Vertex, orig_model->num_verts);
    ALLOCN(solution->edges, Edge, orig_model->num_edges);
    ALLOCN(solution->tris, Triangle, orig_model->num_tris);
    solution->num_verts = orig_model->num_verts;
    solution->num_edges = orig_model->num_edges;   
    solution->num_tris = orig_model->num_tris;

    
    for (i=0; i<orig_model->num_verts; i++)
    {
	src_vert = &(orig_model->verts[i]);
	dest_vert = &(solution->verts[i]);

	dest_vert->id = src_vert->id;
	VEC3_ASN_OP(dest_vert->coord, =, src_vert->coord);
	VEC3_ASN_OP(dest_vert->normal, =, src_vert->normal);
	
	ALLOCN(dest_vert->tris, Triangle *, src_vert->num_tris);
	dest_vert->num_tris = src_vert->num_tris;
	for (j=0; j<src_vert->num_tris; j++)
	    dest_vert->tris[j] =
		&(solution->tris[src_vert->tris[j]->id]);

	ALLOCN(dest_vert->edges, Edge *, src_vert->num_edges);
	dest_vert->num_edges = src_vert->num_edges;
	for (j=0; j<src_vert->num_edges; j++)
	    dest_vert->edges[j] =
		&(solution->edges[src_vert->edges[j]->id]);

	dest_vert->other_props = src_vert->other_props;
    }

    
    for (i=0; i<orig_model->num_edges; i++)
    {
	src_edge = &(orig_model->edges[i]);
	dest_edge = &(solution->edges[i]);

	dest_edge->id = src_edge->id;
	dest_edge->verts[0] = &(solution->verts[src_edge->verts[0]->id]);
	dest_edge->verts[1] = &(solution->verts[src_edge->verts[1]->id]);
	dest_edge->tris[0] = &(solution->tris[src_edge->tris[0]->id]);
	dest_edge->tris[1] =
	    (src_edge->tris[1] == NULL) ? NULL :
		&(solution->tris[src_edge->tris[1]->id]);
    }

    
    for (i=0; i<orig_model->num_tris; i++)
    {
	src_tri = &(orig_model->tris[i]);
	dest_tri = &(solution->tris[i]);

	dest_tri->id = src_tri->id;
	for (j=0; j<3; j++)
	{
	    dest_tri->verts[j] = &(solution->verts[src_tri->verts[j]->id]);
	    dest_tri->edges[j] = &(solution->edges[src_tri->edges[j]->id]);
	}
	VEC_ASN_OP(dest_tri->plane_eq, =, src_tri->plane_eq, 4);
    }
} 



static void finalize_solution(Surface *model)
{
    int       i, j, k;
    Vertex   *vert, *src_vert, *dest_vert;
    Edge     *edge, *src_edge, *dest_edge;
    Triangle *tri,  *src_tri,  *dest_tri;
    int       new_num_verts, new_num_edges, new_num_tris;
    
    
    for (i=0, dest_vert = &(model->verts[0]), new_num_verts = 0;
	 i<model->num_verts; i++)
    {
	src_vert = &(model->verts[i]);
	if (src_vert->num_edges != 0)
	{
	 if (src_vert != dest_vert)
	 {
	    
	    dest_vert->id = new_num_verts;
	    VEC3_ASN_OP(dest_vert->coord, =, src_vert->coord);
	    VEC3_ASN_OP(dest_vert->normal, =, src_vert->normal);
	    dest_vert->curvature = src_vert->curvature;
	    dest_vert->tris = src_vert->tris;
	    dest_vert->num_tris = src_vert->num_tris;
	    dest_vert->edges = src_vert->edges;
	    dest_vert->num_edges = src_vert->num_edges;
	    dest_vert->other_props = src_vert->other_props;
	    dest_vert->handy_mark = src_vert->handy_mark;

	    
	    for (j=0; j<dest_vert->num_tris; j++)
	    {
		tri = dest_vert->tris[j];
		for (k=0; k<3; k++)
		    if (tri->verts[k] == src_vert)
			tri->verts[k] = dest_vert;
	    }
	    
	    
	    for (j=0; j<dest_vert->num_edges; j++)
	    {
		edge = dest_vert->edges[j];
		for (k=0; k<2; k++)
		    if (edge->verts[k] == src_vert)
			edge->verts[k] = dest_vert;
	    }
	  }   
	  dest_vert++;
	  new_num_verts++;
	}
    }
    model->num_verts = new_num_verts;
    
    
    for (i=0; i<model->num_edges; i++)
	model->edges[i].handy_mark = 0;
    for (i=0; i<model->num_verts; i++)
    {
	vert = &(model->verts[i]);
	for (j=0; j<vert->num_edges; j++)
	    vert->edges[j]->handy_mark = 1;
    }
    
    for (i=0, dest_edge = &(model->edges[0]), new_num_edges = 0;
	 i<model->num_edges; i++)
    {
	src_edge = &(model->edges[i]);
	if (src_edge->handy_mark != 0)
	{
	 if (src_edge != dest_edge)
	 {
	    
	    dest_edge->id = new_num_edges;
	    dest_edge->verts[0] = src_edge->verts[0];
	    dest_edge->verts[1] = src_edge->verts[1];
	    dest_edge->tris[0] = src_edge->tris[0];
	    dest_edge->tris[1] = src_edge->tris[1];
	    dest_edge->handy_mark = src_edge->handy_mark;
	    
	    
	    for (j=0; j<2; j++)
	    {
		vert = dest_edge->verts[j];
		for (k=0; k<vert->num_edges; k++)
		    if (vert->edges[k] == src_edge)
			vert->edges[k] = dest_edge;
	    }
	    
	    
	    for (j=0; j<2; j++)
	    {
		tri = dest_edge->tris[j];
		if (!(tri))
		    continue;
		for (k=0; k<3; k++)
		    if (tri->edges[k] == src_edge)
			tri->edges[k] = dest_edge;
	    }
	 }   
	 dest_edge++;
	 new_num_edges++;
	}
	
    }
    model->num_edges = new_num_edges;


    
    
    for (i=0; i<model->num_tris; i++)
	model->tris[i].handy_mark = 0;
    for (i=0; i<model->num_edges; i++)
    {
	edge = &(model->edges[i]);
	edge->tris[0]->handy_mark = 1;
	if (edge->tris[1])
	    edge->tris[1]->handy_mark = 1;
    }

    for (i=0, dest_tri = &(model->tris[0]), new_num_tris = 0;
	 i<model->num_tris; i++)
    {
	src_tri = &(model->tris[i]);
	if (src_tri->handy_mark != 0)
	{
	 if (src_tri != dest_tri)
	 {
	    
	    dest_tri->id = new_num_tris;
	    VEC3_ASN_OP(dest_tri->verts, =, src_tri->verts);
	    VEC3_ASN_OP(dest_tri->edges, =, src_tri->edges);
	    VEC_ASN_OP(dest_tri->plane_eq, =, src_tri->plane_eq, 4);
	    dest_tri->handy_mark = src_tri->handy_mark;
	    
	    
	    for (j=0; j<3; j++)
	    {
		vert = dest_tri->verts[j];
		for (k=0; k<vert->num_tris; k++)
		    if (vert->tris[k] == src_tri)
			vert->tris[k] = dest_tri;
	    }

	    
	    for (j=0; j<3; j++)
	    {
		edge = dest_tri->edges[j];
		for (k=0; k<3; k++)
		    if (edge->tris[k] == src_tri)
			edge->tris[k] = dest_tri;
	    }
	 }   
	 dest_tri++;
	 new_num_tris++;
	}
	
    }
    model->num_tris = new_num_tris;
    
} 





static void build_removal_queue(Surface *model, RemovalQueue *queue)
{
    int i;

    removal_queue_init(queue, model->num_verts);
    for (i=0; i<model->num_verts; i++)
	if (model->verts[i].num_tris > 0)
	    removal_queue_touch(queue, i);
    return;
} 



static void update_removal_queue(RemovalQueue *queue, int vert_removed,
				 NeighborList *neighbors)
{
    int i;

    if (vert_removed != TRUE)
	return;

    for (i=0; i<neighbors->num_neighbors; i++)
	removal_queue_touch(queue, neighbors->neighbors[i]);
    return;
} 



static void get_neighbor_list(Vertex *vert, NeighborList *list)
{
    int   i;
    Edge *edge;

    ALLOCN(list->neighbors, int, vert->num_edges);
    for (i=0, list->num_neighbors = 0; i<vert->num_edges; i++)
    {
	edge = vert->edges[i];
	list->neighbors[list->num_neighbors++] =
	    (edge->verts[0] == vert) ? edge->verts[1]->id :
		edge->verts[0]->id;
    }
    return;
} 



static void destroy_neighbor_list(NeighborList *list)
{
    FREE(list->neighbors);
    list->num_neighbors = 0;
    return;
} 








