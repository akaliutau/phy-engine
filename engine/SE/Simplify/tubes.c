





#include <stdio.h>
#include <math.h>
#include <simplify.h>
#include <geometry.h>
#include <intersect.h>
#include <offset.h>
#include <tubes.h>
#include <ply.h>












static void construct_border_polylines(Surface *model, PolyLine **polylines,
				       int *num_polylines);
static void construct_border_tubes(PolyLine *polylines, int num_polylines,
				   int num_sides, double width,
				   Surface *tubes);
static void construct_tube(PolyLine *polyline, int num_sides,
			    double width, Surface *tube);
static void append_tube(Surface *one_tube, Surface *tubes);







void init_tubes(Surface *model, Surface *tubes, double width, int num_sides)
{
    PolyLine *polylines;
    int       num_polylines;
    
    fprintf(stderr, "building polylines...");
    fflush(stderr);
    
    construct_border_polylines(model, &polylines, &num_polylines);

    fprintf(stderr, "done\n");
    fprintf(stderr, "building tubes...");
    fflush(stderr);

    construct_border_tubes(polylines, num_polylines, num_sides, width, tubes);

    fprintf(stderr, "done\n");
    fflush(stderr);
    
    return;
} 


static void construct_border_polylines(Surface *model, PolyLine **polylines,
				       int *num_polylines)
{
    int            i, j;
    unsigned char *bvert_used;
    Vertex        *vert;
    PolyLine      *polyline;
    Vertex        *start_vert, *adjacent_vert;
    Edge          *edge;
    Vertex        *prev_vert;
    
    ALLOCN(bvert_used, unsigned char, model->num_verts);
    for (i=0; i<model->num_verts; i++)
    {
	vert = &(model->verts[i]);
	if (vert->num_tris == vert->num_edges) 
	    bvert_used[i] = TRUE;
	else if (vert->num_tris == (vert->num_edges-1)) 
	    bvert_used[i] = FALSE;
	else 
	{
	    fprintf(stderr,
		    "Vertex #%d is non-manifold (on multiple borders)\n", i);
	    exit(1);
	}
    }

    for (i=0, *num_polylines = 0; i<model->num_verts; i++)
    {
	if (bvert_used[i] == TRUE)
	    continue;

	REALLOCN(*polylines, PolyLine , *num_polylines, *num_polylines + 1);
	polyline = &((*polylines)[*num_polylines]);

	start_vert = &(model->verts[i]);
	ALLOCN(polyline->verts, Vertex *, 1);
	polyline->verts[0] = start_vert;
	polyline->num_verts = 1;
	bvert_used[start_vert->id] = TRUE;
	for (j=0; j<start_vert->num_edges; j++)
	{
	    edge = start_vert->edges[j];
	    adjacent_vert =
		(edge->verts[0] == start_vert) ?
		    edge->verts[1] : edge->verts[0];
	    if (bvert_used[adjacent_vert->id] == FALSE) 
		break;
	}
	if (j >= start_vert->num_edges)
	{
	    fprintf(stderr, "Couldn't find second border vert\n");
	    exit(1);
	}
	vert = adjacent_vert;
	prev_vert = start_vert;
	while (vert != start_vert)
	{
	    REALLOCN(polyline->verts, Vertex *,
		     polyline->num_verts, polyline->num_verts+1);
	    polyline->verts[polyline->num_verts++] = vert;
	    bvert_used[vert->id] = TRUE;
	    
	    for (j=0; j<vert->num_edges; j++)
	    {
		edge = vert->edges[j];
		if (edge->tris[1] != NULL)
		    continue;
		adjacent_vert =
		    (edge->verts[0] == vert) ?
			edge->verts[1] : edge->verts[0];
		if (adjacent_vert == prev_vert)
		    continue;
		break;
	    }
	    if (j >= vert->num_edges)
	    {
		fprintf(stderr, "Couldn't follow border\n");
		exit(1);
	    }
	    
	    prev_vert = vert;
	    vert = adjacent_vert;
	}
	(*num_polylines)++;
    }
    return;
} 


void calculate_tube_verts(PolyLine *polyline, Vertex **verts,
			  int *num_verts, int num_sides, double width)
{
    Point *plane_points;
    int    point_count;
    int vert_count;
    Vertex *vert, *next_vert, *prev_vert;
    Vector forward_vec, backward_vec, binormal;
    Vector tangent;
    Vector plane_x, plane_y;
    Vector edge_vec, bisector, bisector_perp;
    Matrix mat;
    Vector normal_component;
    double normal_scale;
    Vector edge_bisector, prev_bisector, orig_bisector, forward_bisector;

    *num_verts = 0;
    ALLOCN(*verts, Vertex, num_sides*polyline->num_verts);

    
    ALLOCN(plane_points, Point, num_sides);
    for (point_count = 0; point_count < num_sides; point_count++)
    {
	plane_points[point_count][X] =
	    width * cos(2.0*M_PI* (double)point_count/(double)num_sides);
	plane_points[point_count][Y] =
	    width*sin(2.0*M_PI * (double)point_count/(double)num_sides);
	plane_points[point_count][Z] = 0.0;
    }

    for (vert_count = 0; vert_count < polyline->num_verts; vert_count++)
    {
	vert = polyline->verts[vert_count];
	next_vert = polyline->verts[(vert_count+1)%polyline->num_verts];
	prev_vert = polyline->verts[(vert_count+polyline->num_verts-1) %
				      polyline->num_verts];

	VEC3_V_OP_V(forward_vec, next_vert->coord, -, vert->coord);
	VEC3_V_OP_V(backward_vec, prev_vert->coord, -, vert->coord);
	NORMALIZE3(forward_vec);
	NORMALIZE3(backward_vec);

	
	if (DOTPROD3(forward_vec, backward_vec) < 0) 
	{
	    VEC3_V_OP_V(tangent, next_vert->coord, -, prev_vert->coord);
	    NORMALIZE3(tangent);
	}
	else
	{
	    VEC3_VOPV_OP_S(bisector, forward_vec, +, backward_vec, *, 0.5);
	    CROSSPROD3(binormal, forward_vec, backward_vec);
	    CROSSPROD3(tangent, bisector, binormal);
	    NORMALIZE3(tangent);
	}

	
	if (vert_count == 0)
	{
	    
	    VEC3_V_OP_V(edge_vec, next_vert->coord, -, vert->coord);
	    CROSSPROD3(plane_x, tangent, edge_vec);
	    CROSSPROD3(plane_y, plane_x, tangent);
	    NORMALIZE3(plane_x);
	    NORMALIZE3(plane_y);
	    VEC3_VOPV_OP_S(bisector, plane_x, +, plane_y, *, 0.5);
	    NORMALIZE3(bisector);
	    VEC3_ASN_OP(orig_bisector, =, bisector);
	}
	else if (vert_count != (polyline->num_verts - 1))
	{
	    
	    
	    VEC3_V_OP_V(edge_vec, vert->coord, -, prev_vert->coord);
	    NORMALIZE3(edge_vec);
	    
	    
	    normal_scale = DOTPROD3(prev_bisector, edge_vec);
	    VEC3_V_OP_S(normal_component, edge_vec, *, normal_scale);
	    VEC3_V_OP_V(edge_bisector, prev_bisector, -, normal_component);

	    
	    normal_scale = DOTPROD3(edge_bisector, tangent);
	    VEC3_V_OP_S(normal_component, tangent, *, normal_scale);
	    VEC3_V_OP_V(bisector, edge_bisector, -, normal_component);
	    NORMALIZE3(bisector);

	    

	    CROSSPROD3(bisector_perp, tangent, bisector);
	    NORMALIZE3(bisector_perp);

	    VEC3_VOPV_OP_S(plane_x, bisector, +, bisector_perp, *, 0.5);
	    NORMALIZE3(plane_x);
	    
	    CROSSPROD3(plane_y, plane_x, tangent);
	    NORMALIZE3(plane_y);
	}
	else
	{
	    


	       
	    

	    VEC3_V_OP_V(edge_vec, vert->coord, -, prev_vert->coord);
	    NORMALIZE3(edge_vec);
	    normal_scale = DOTPROD3(prev_bisector, edge_vec);
	    VEC3_V_OP_S(normal_component, edge_vec, *, normal_scale);
	    VEC3_V_OP_V(edge_bisector, prev_bisector, -, normal_component);

	    
	    normal_scale = DOTPROD3(edge_bisector, tangent);
	    VEC3_V_OP_S(normal_component, tangent, *, normal_scale);
	    VEC3_V_OP_V(bisector, edge_bisector, -, normal_component);	
	    NORMALIZE3(bisector);

	    

	    VEC3_V_OP_V(edge_vec, next_vert->coord, -, vert->coord);
	    NORMALIZE3(edge_vec);
	    normal_scale = DOTPROD3(orig_bisector, edge_vec);
	    VEC3_V_OP_S(normal_component, edge_vec, *, normal_scale);
	    VEC3_V_OP_V(edge_bisector, orig_bisector, -, normal_component);

	    
	    normal_scale = DOTPROD3(edge_bisector, tangent);
	    VEC3_V_OP_S(normal_component, tangent, *, normal_scale);
	    VEC3_V_OP_V(forward_bisector, edge_bisector, -, normal_component);	
	    NORMALIZE3(forward_bisector);

	    
	    VEC3_VOPV_OP_S(bisector, bisector, +, forward_bisector, *, 0.5);
	    
	    
	    

	    CROSSPROD3(bisector_perp, tangent, bisector);
	    NORMALIZE3(bisector_perp);

	    VEC3_VOPV_OP_S(plane_x, bisector, +, bisector_perp, *, 0.5);
	    NORMALIZE3(plane_x);
	    
	    CROSSPROD3(plane_y, plane_x, tangent);
	    NORMALIZE3(plane_y);
	}
	
	
	VEC3_ASN_OP(prev_bisector, =, bisector);


	
	
	
	mat[0][0] = plane_x[X];
	mat[1][0] = plane_x[Y];
	mat[2][0] = plane_x[Z];

	
	mat[0][1] = plane_y[X];
	mat[1][1] = plane_y[Y];
	mat[2][1] = plane_y[Z];

	
	mat[0][2] = tangent[X];
	mat[1][2] = tangent[Y];
	mat[2][2] = tangent[Z];

	
	mat[0][3] = polyline->verts[vert_count]->coord[X];
	mat[1][3] = polyline->verts[vert_count]->coord[Y];
	mat[2][3] = polyline->verts[vert_count]->coord[Z];
	
	for (point_count = 0; point_count < num_sides; point_count++)
	{
	    TRANSFORM_POINT((*verts)[*num_verts].coord,
			    mat, plane_points[point_count]);
	    TRANSFORM_VECTOR((*verts)[*num_verts].normal,
			     mat, plane_points[point_count]);
	    NORMALIZE3((*verts)[*num_verts].normal);
#ifdef VERTEX_EPSILONS
                (*verts)[*num_verts].epsilon = vert->epsilon;
#endif
	    (*num_verts)++;
	}
    }
    return;
} 





static void construct_border_tubes(PolyLine *polylines, int num_polylines,
				   int num_sides, double width,
				   Surface *tubes)
{
    int   i;
    Surface tube;
    PolyLine *polyline;
    

    
    tubes->num_verts = tubes->num_edges = tubes->num_tris = 0;
    for (i=0; i<num_polylines; i++)
    {
	polyline = &(polylines[i]);
	tubes->num_verts += num_sides*polyline->num_verts;
	tubes->num_edges += 3*num_sides*polyline->num_verts;
	tubes->num_tris += 2*num_sides*polyline->num_verts;
    }
    ALLOCN(tubes->verts, Vertex, tubes->num_verts);
    ALLOCN(tubes->edges, Edge, tubes->num_edges);
    ALLOCN(tubes->tris, Triangle, tubes->num_tris);
    tubes->num_verts = tubes->num_edges = tubes->num_tris = 0;

    
    for (i=0; i<num_polylines; i++)
    {
	construct_tube(&(polylines[i]), num_sides, width, &tube);
	append_tube(&tube, tubes);
    }

    return;
}



static void construct_tube(PolyLine *polyline, int num_sides,
			   double width, Surface *tube)
{
    int       i, j;
    int       edge_index, face_index, vert_index;
    Edge     *edge;
    Triangle *face;
    Vertex   *vert;
    
    
    calculate_tube_verts(polyline, &(tube->verts), &(tube->num_verts),
			 num_sides, width);
    
    ALLOCN(tube->tris, Triangle, polyline->num_verts * 2 * num_sides);
    tube->num_tris = polyline->num_verts * 2 * num_sides;

    ALLOCN(tube->edges, Edge, polyline->num_verts * 3 * num_sides);
    tube->num_edges = polyline->num_verts * 3 * num_sides; 

    

    

    
    
    
    for (i=0; i<polyline->num_verts; i++)
    {
	for (j=0; j<num_sides; j++)
	{
	    edge_index = i*num_sides*3 + j;
	    edge = &(tube->edges[edge_index]);
	    edge->id = edge_index;
	    edge->verts[0] = &(tube->verts[i*num_sides+j]);
	    edge->verts[1] = &(tube->verts[i*num_sides+(j+1)%num_sides]);
	    edge->tris[0] = &(tube->tris[2*i*num_sides+j]);
	    edge->tris[1] =
		&(tube->tris[((i+polyline->num_verts-1)%polyline->num_verts) *
			     2*num_sides + num_sides + j]);
	}
    }

    
    for (i=0; i<polyline->num_verts; i++)
    {
	for (j=0; j<num_sides; j++)
	{
	    edge_index = i*num_sides*3 + num_sides + j;
	    edge = &(tube->edges[edge_index]);
	    edge->id = edge_index;
	    edge->verts[0] = &(tube->verts[i*num_sides+j]);
	    edge->verts[1] = &(tube->verts[((i+1)%polyline->num_verts) *
					   num_sides + j]);
	    edge->tris[0] = &(tube->tris[2*i*num_sides+j]);
	    edge->tris[1] = &(tube->tris[2*i*num_sides + num_sides +
					 (j+num_sides-1)%num_sides]);
	}
    }
    
    
    for (i=0; i<polyline->num_verts; i++)
    {
	for (j=0; j<num_sides; j++)
	{
	    edge_index = i*num_sides*3 + 2*num_sides + j;
	    edge = &(tube->edges[edge_index]);
	    edge->id = edge_index;
	    edge->verts[0] = &(tube->verts[i*num_sides + (j+1)%num_sides]);
	    edge->verts[1] = &(tube->verts[((i+1)%polyline->num_verts) *
					   num_sides + j]);
	    edge->tris[0] = &(tube->tris[2*i*num_sides+j]);
	    edge->tris[1] = &(tube->tris[2*i*num_sides+num_sides+j]);
	}
    }
    
    
    for (i=0; i<polyline->num_verts; i++)
    {
	for (j=0; j<num_sides; j++)
	{
	    face_index = i*num_sides*2 + j;
	    face = &(tube->tris[face_index]);
	    face->id = face_index;
	    face->verts[0] = &(tube->verts[i*num_sides+j]);
	    face->verts[1] = &(tube->verts[((i+1)%polyline->num_verts) *
					   num_sides + j]);
	    face->verts[2] = &(tube->verts[i*num_sides+(j+1)%num_sides]);
	    face->edges[0] = &(tube->edges[i*num_sides*3 + num_sides + j]);
	    face->edges[1] = &(tube->edges[i*num_sides*3 + 2*num_sides + j]);
	    face->edges[2] = &(tube->edges[i*num_sides*3 + j]);
	    find_plane_eq(face->verts[0]->coord,
			  face->verts[1]->coord,
			  face->verts[2]->coord,
			  face->plane_eq);
	}
    }
    
    
    
    for (i=0; i<polyline->num_verts; i++)
    {
	for (j=0; j<num_sides; j++)
	{
	    face_index = i*num_sides*2 + num_sides + j;
	    face = &(tube->tris[face_index]);
	    face->id = face_index;
	    face->verts[0] = &(tube->verts[((i+1)%polyline->num_verts) *
					   num_sides + j]);
	    face->verts[1] = &(tube->verts[((i+1)%polyline->num_verts) *
					   num_sides + (j+1)%num_sides]);
	    face->verts[2] = &(tube->verts[i*num_sides+(j+1)%num_sides]);
	    face->edges[0] = &(tube->edges[((i+1)%polyline->num_verts) *
					   num_sides*3 + j]);
	    face->edges[1] = &(tube->edges[i*num_sides*3 + num_sides +
					   (j+1)%num_sides]);
	    face->edges[2] = &(tube->edges[i*num_sides*3 + 2*num_sides + j]);
	    find_plane_eq(face->verts[0]->coord,
			  face->verts[1]->coord,
			  face->verts[2]->coord,
			  face->plane_eq);
	}
    }
    
    
    for (i=0; i<polyline->num_verts; i++)
    {
	for (j=0; j<num_sides; j++)
	{
	    vert_index = i*num_sides + j;
	    vert = &(tube->verts[vert_index]);
	    vert->id = vert_index;
	    ALLOCN(vert->edges, Edge *, 6);
	    ALLOCN(vert->tris, Triangle *, 6);
	    vert->num_tris = vert->num_edges = 6;
	    vert->edges[0] = &(tube->edges[i*num_sides*3 +
					   (j+num_sides-1) % num_sides]);
	    vert->edges[1] = &(tube->edges[i*num_sides*3 + 2*num_sides +
					   (j+num_sides-1) % num_sides]);
	    vert->edges[2] = &(tube->edges[i*num_sides*3 + num_sides + j]);
	    vert->edges[3] = &(tube->edges[i*num_sides*3 + j]);
	    vert->edges[4] = &(tube->edges[((i+polyline->num_verts-1) %
					   polyline->num_verts)*num_sides*3 +
					   2*num_sides + j]);
	    vert->edges[5] = &(tube->edges[((i+polyline->num_verts-1) %
					   polyline->num_verts)*num_sides*3 +
					   num_sides + j]);
	    vert->tris[0] = &(tube->tris[i*num_sides*2 +
					 (j+num_sides-1)%num_sides]);
	    vert->tris[1] = &(tube->tris[i*num_sides*2 + num_sides +
					 (j+num_sides-1)%num_sides]);
	    vert->tris[2] = &(tube->tris[i*num_sides*2 + j]);
	    vert->tris[3] = &(tube->tris[((i+polyline->num_verts-1) %
					  polyline->num_verts)*num_sides*2 +
					 num_sides + j]);
	    vert->tris[4] = &(tube->tris[((i+polyline->num_verts-1) %
					  polyline->num_verts)*num_sides*2 +
					 j]);
	    vert->tris[5] = &(tube->tris[((i+polyline->num_verts-1) %
					  polyline->num_verts)*num_sides*2 +
					 num_sides +
					 (j+num_sides-1)%num_sides]);
	}
    }
    return;
} 


static void append_tube(Surface *one_tube, Surface *tubes)
{
    int       i, j;
    int       start_vert, start_edge, start_tri;
    Vertex   *src_vert, *dest_vert;
    Edge     *src_edge, *dest_edge;
    Triangle *src_tri, *dest_tri;
    Vertex   *vert;
    

    start_vert = tubes->num_verts;
    start_edge = tubes->num_edges;
    start_tri  = tubes->num_tris;
    
    
    for (i=0; i<one_tube->num_verts; i++)
    {
	src_vert = &(one_tube->verts[i]);
	dest_vert = &(tubes->verts[tubes->num_verts]);
	dest_vert->id = tubes->num_verts;
	VEC3_ASN_OP(dest_vert->coord, =, src_vert->coord);
	VEC3_ASN_OP(dest_vert->normal, =, src_vert->normal);
	
	
	ALLOCN(dest_vert->edges, Edge *, src_vert->num_edges);
	for (j=0; j<src_vert->num_edges; j++)
	    dest_vert->edges[j] =
		&(tubes->edges[src_vert->edges[j]->id + start_edge]);
	dest_vert->num_edges = src_vert->num_edges;

	
	ALLOCN(dest_vert->tris, Triangle *, src_vert->num_tris);
	for (j=0; j<src_vert->num_tris; j++)
	    dest_vert->tris[j] =
		&(tubes->tris[src_vert->tris[j]->id + start_tri]);
	dest_vert->num_tris = src_vert->num_tris;
	
	tubes->num_verts++;
    }
    
    
    for (i=0; i<one_tube->num_edges; i++)
    {
	src_edge = &(one_tube->edges[i]);
	dest_edge = &(tubes->edges[tubes->num_edges]);
	dest_edge->id = tubes->num_edges;

	
	for (j=0; j<2; j++)
	    dest_edge->verts[j] =
		&(tubes->verts[src_edge->verts[j]->id + start_vert]);
	
	
	for (j=0; j<2; j++)
	    dest_edge->tris[j] =
		&(tubes->tris[src_edge->tris[j]->id + start_tri]);

	tubes->num_edges++;
    }
    
    
    for (i=0; i<one_tube->num_tris; i++)
    {
	src_tri = &(one_tube->tris[i]);
	dest_tri = &(tubes->tris[tubes->num_tris]);
	dest_tri->id = tubes->num_tris;

	
	for (j=0; j<3; j++)
	    dest_tri->verts[j] =
		&(tubes->verts[src_tri->verts[j]->id + start_vert]);

	
	for (j=0; j<3; j++)
	    dest_tri->edges[j] =
		&(tubes->edges[src_tri->edges[j]->id + start_edge]);

	VEC_ASN_OP(dest_tri->plane_eq, =, src_tri->plane_eq, 4);
	
	tubes->num_tris++;
    }
    
    
    for (i=0; i<one_tube->num_verts; i++)
    {
	vert = &(one_tube->verts[i]);
	FREE(vert->edges);
	FREE(vert->tris);
    }
    FREE(one_tube->verts);
    FREE(one_tube->edges);
    FREE(one_tube->tris);
    
    return;
} 



