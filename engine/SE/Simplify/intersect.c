





#include <intersect.h>
#include <geometry.h>
#include <vertex_removal.h>
#include <values.h>
#include <math.h>
#include <stdio.h>



#define MAX_FUZZ (1e-2)
#define MIN_FUZZ (1e-8)









int tri_tri_intersect(Triangle *tri1, Triangle *tri2, double fuzz_factor,
		      int common_verts[3][2], int num_common_verts,
		      int test_type);
int edge_tri_intersect(Edge *edge, Triangle *tri, double fuzz_factor,
		       int common_verts[2][2], int num_common_verts);
int rawtri_rawtri_intersect(RawTriangle *tri1, RawTriangle *tri2,
			     double fuzz_factor);
int lineseg_rawtri_intersect(LineSegment *lineseg, RawTriangle *tri,
			     double *t, double fuzz_factor);







void test_self_intersect(Surface *model, Octree *tree, OctreeData *octdata,
			 OctreeData **query_result, double *fuzz_factor)
{
    int        	 i, j;
    int          query_count;
    Triangle    *tri1, *tri2;
    int          nearby, intersecting;
    int          intersect, done;

    nearby = intersecting = 0;
    *fuzz_factor = -MAX_FUZZ;
    for (i=0; i<model->num_tris; i++)
    {
	tri1 = &(model->tris[i]);
	
#ifdef USE_TRIOCTREE
	trioctree_tri_range_query(tree, tri1, query_result, &query_count);
#else
	octree_range_query(tree, octdata[i].bbox, query_result, &query_count);
#endif
	for (j=0; j<query_count; j++)
	{
#ifdef USE_TRIOCTREE
	    tri2 = query_result[j];
#else
	    tri2 = (Triangle *)(query_result[j]->data);
#endif

	    if (tri2->id <= tri1->id)
		continue;
	    nearby++;

	    done = FALSE;
	    while (done == FALSE)
	    {
		intersect =
		    index_filtered_tri_tri_intersect(tri1, tri2, *fuzz_factor,
						     SURFACE_VS_ITSELF);
		if (intersect == FALSE)
		    done = TRUE;
		else
		{
		    *fuzz_factor /= 2.0;
		    if (*fuzz_factor > -MIN_FUZZ)
		    {
			fprintf(stderr, "intersecting pair: (tri #%d, tri #%d)\n",
				tri1->id, tri2->id);
			intersecting++;
			*fuzz_factor = -MIN_FUZZ;
			done = TRUE;
		    }
		}
	    }
	}
    }
    

    fprintf(stderr, "Found %d pairs of nearby triangles\n", nearby);
    fprintf(stderr, "Found %d pairs of intersecting triangles\n",
	    intersecting);
    fprintf(stderr, "Final fuzz factor: %g\n", *fuzz_factor);
    fprintf(stderr, "\n");
    
    return;
    
} 



int edge_offsets_intersect(Octree *outer_offset, Octree *inner_offset,
			   OctreeData **query_result,
			   Edge *edge, double fuzz_factor)
{
    int     	i;
    int         query_count;
    Triangle   *tri;
#ifndef USE_TRIOCTREE
    Extents 	edge_bbox;
    double      fuzz_width;
#endif

    
#ifdef USE_TRIOCTREE
    trioctree_edge_range_query(outer_offset, edge, query_result,
			       &query_count);
#else
    for (i=0; i<3; i++)
    {
	edge_bbox[LO][i] =
	    FMIN(edge->verts[0]->coord[i], edge->verts[1]->coord[i]);
	edge_bbox[HI][i] =
	    FMAX(edge->verts[0]->coord[i], edge->verts[1]->coord[i]);
	if (fuzz_factor < 0.0)
	{
	    fuzz_width = (edge_bbox[HI][i] - edge_bbox[LO][i]) * -fuzz_factor;
	    edge_bbox[LO][i] -= fuzz_width;
	    edge_bbox[HI][i] += fuzz_width;
	}
    }

    octree_range_query(outer_offset, edge_bbox, query_result,
		       &query_count);
#endif
    
    for (i=0; i<query_count; i++)
    {
#ifdef USE_TRIOCTREE
	tri = query_result[i];
#else
	tri = (Triangle *)(query_result[i]->data);
#endif
	if (coord_filtered_edge_tri_intersect(edge, tri, fuzz_factor))
	    return TRUE;
    }


    
#ifdef USE_TRIOCTREE
    trioctree_edge_range_query(inner_offset, edge, query_result,
			       &query_count);
#else
    octree_range_query(inner_offset, edge_bbox, query_result,
		       &query_count);
#endif

    for (i=0; i<query_count; i++)
    {
#ifdef USE_TRIOCTREE
	tri = query_result[i];
#else
	tri = (Triangle *)(query_result[i]->data);
#endif
	if (coord_filtered_edge_tri_intersect(edge, tri, fuzz_factor))
	    return TRUE;
    }
    return FALSE;
} 





int tri_offsets_intersect(Triangle *tri,
			  Octree *outer_offset, Octree *inner_offset,
			  OctreeData **query_result, double fuzz_factor)
{
    int       	 i;
#ifndef USE_TRIOCTREE
    Extents   	 tri_bbox;
    double       fuzz_width;
#endif
    Triangle  	*tri2;
    int          query_count;
    
    
#ifdef USE_TRIOCTREE
    trioctree_tri_range_query(outer_offset, tri, query_result,
			     &query_count);
#else
    for (i=0; i<3; i++)
    {
	tri_bbox[LO][i] = FMIN(FMIN(tri->verts[0]->coord[i],
				    tri->verts[1]->coord[i]),
			       tri->verts[2]->coord[i]);
	tri_bbox[HI][i] = FMAX(FMAX(tri->verts[0]->coord[i],
				    tri->verts[1]->coord[i]),
			       tri->verts[2]->coord[i]);
	if (fuzz_factor < 0.0)
	{
	    fuzz_width = (tri_bbox[HI][i] - tri_bbox[LO][i]) * -fuzz_factor;
	    tri_bbox[LO][i] -= fuzz_width;
	    tri_bbox[HI][i] += fuzz_width;
	}
    }

    octree_range_query(outer_offset, tri_bbox, query_result,
		       &query_count);
#endif
    
    for (i=0; i<query_count; i++)
    {
#ifdef USE_TRIOCTREE
	tri2 = query_result[i];
#else
	tri2 = (Triangle *)(query_result[i]->data);
#endif
	if (coord_filtered_tri_tri_intersect(tri, tri2, fuzz_factor,
					     SURFACE_VS_OUTER))
	    return TRUE;
    }


#ifdef USE_TRIOCTREE
    trioctree_tri_range_query(inner_offset, tri, query_result,
			     &query_count);
#else
    
    octree_range_query(inner_offset, tri_bbox, query_result,
		       &query_count);
#endif
    
    for (i=0; i<query_count; i++)
    {
#ifdef USE_TRIOCTREE
	tri2 = query_result[i];
#else
	tri2 = (Triangle *)(query_result[i]->data);
#endif
	if (coord_filtered_tri_tri_intersect(tri, tri2, fuzz_factor,
					     SURFACE_VS_INNER))
	    return TRUE;
    }

    return FALSE;
} 



int tri_model_interference(Triangle *tri, Vertex *vert,
			   Octree *model_octree, OctreeData **query_result,
			   double fuzz_factor)
{
    int       	 i;
#ifndef USE_TRIOCTREE
    Extents   	 tri_bbox;
    double       fuzz_width;
#endif
    Triangle  	*tri2;
    int          query_count;
    
#ifdef USE_TRIOCTREE
    trioctree_tri_range_query(model_octree, tri, query_result,
			      &query_count);
#else
    for (i=0; i<3; i++)
    {
	tri_bbox[LO][i] = FMIN(FMIN(tri->verts[0]->coord[i],
				    tri->verts[1]->coord[i]),
			       tri->verts[2]->coord[i]);
	tri_bbox[HI][i] = FMAX(FMAX(tri->verts[0]->coord[i],
				    tri->verts[1]->coord[i]),
			       tri->verts[2]->coord[i]);
	if (fuzz_factor < 0.0)
	{
	    fuzz_width = (tri_bbox[HI][i] - tri_bbox[LO][i]) * -fuzz_factor;
	    tri_bbox[LO][i] -= fuzz_width;
	    tri_bbox[HI][i] += fuzz_width;
	}
    }

    octree_range_query(model_octree, tri_bbox, query_result,
		       &query_count);
#endif
    
    for (i=0; i<query_count; i++)
    {
#ifdef USE_TRIOCTREE
	tri2 = query_result[i];
#else
	tri2 = (Triangle *)(query_result[i]->data);
#endif
	
	if ((tri2->verts[0] == vert) || (tri2->verts[1] == vert) ||
	    (tri2->verts[2] == vert))
	    continue;
	
	if (index_filtered_tri_tri_intersect(tri, tri2, fuzz_factor,
					     SURFACE_VS_ITSELF))
	    return TRUE;
    }

    return FALSE;
} 


int tri_solution_interference(Hole *hole, Triangle *tri, double fuzz_factor)
{
    int i;

    for (i=0; i < *hole->num_solution_tris_ptr; i++)
	if (index_filtered_tri_tri_intersect(tri, hole->solution_tris[i],
					     fuzz_factor,
					     SURFACE_VS_ITSELF))
	    return TRUE;
    return FALSE;
} 


int edge_tubes_intersect(Edge *edge,
			 Octree *tubes, OctreeData **query_result,
			 double fuzz_factor)
{
    int     	i;
#ifndef USE_TRIOCTREE
    Extents     edge_bbox;
    double      fuzz_width;
#endif
    int         query_count;
    Triangle   *tri;
    
#ifdef USE_TRIOCTREE
    trioctree_edge_range_query(tubes, edge, query_result, &query_count);
#else
    for (i=0; i<3; i++)
    {
	edge_bbox[LO][i] =
	    FMIN(edge->verts[0]->coord[i], edge->verts[1]->coord[i]);
	edge_bbox[HI][i] =
	    FMAX(edge->verts[0]->coord[i], edge->verts[1]->coord[i]);
	if (fuzz_factor < 0.0)
	{
	    fuzz_width = (edge_bbox[HI][i] - edge_bbox[LO][i]) * -fuzz_factor;
	    edge_bbox[LO][i] -= fuzz_width;
	    edge_bbox[HI][i] += fuzz_width;
	}
    }

    octree_range_query(tubes, edge_bbox, query_result, &query_count);
#endif
    
    for (i=0; i<query_count; i++)
    {
#ifdef USE_TRIOCTREE
	tri = query_result[i];
#else
	tri = (Triangle *)(query_result[i]->data);
#endif
	if (unfiltered_edge_tri_intersect(edge, tri, fuzz_factor))
	    return TRUE;
    }

    return FALSE;
} 




int index_filtered_tri_tri_intersect(Triangle *tri1, Triangle *tri2,
				     double fuzz_factor, int test_type)
{
    int intersect, num_common_verts;
    int common_verts[3][2];
    
    num_common_verts = get_common_index_verts(tri1, tri2, common_verts);

    intersect = tri_tri_intersect(tri1, tri2, fuzz_factor, common_verts,
				  num_common_verts, test_type);
    return intersect;
} 


int coord_filtered_tri_tri_intersect(Triangle *tri1, Triangle *tri2,
				     double fuzz_factor, int test_type)
{
    int intersect, num_common_verts;
    int common_verts[3][2];
    
    num_common_verts = get_common_coord_verts(tri1, tri2, common_verts);

    intersect = tri_tri_intersect(tri1, tri2, fuzz_factor, common_verts,
				  num_common_verts, test_type);
    return intersect;
} 



int coord_filtered_edge_tri_intersect(Edge *edge, Triangle *tri,
				      double fuzz_factor)
{
    int intersect, num_common_verts;
    int common_verts[2][2];
    
    num_common_verts =
	get_common_coord_edge_tri_verts(edge, tri, common_verts);

    intersect = edge_tri_intersect(edge, tri, fuzz_factor,
				   common_verts, num_common_verts);
    return intersect;
} 



int unfiltered_edge_tri_intersect(Edge *edge, Triangle *tri,
				  double fuzz_factor)
{
    int intersect, num_common_verts;
    int common_verts[2][2];
    
    num_common_verts = 0;

    intersect = edge_tri_intersect(edge, tri, fuzz_factor,
				   common_verts, num_common_verts);
    return intersect;
} 







int edge_tri_intersect(Edge *edge, Triangle *tri, double fuzz_factor,
		       int common_verts[2][2], int num_common_verts)
{
    LineSegment lineseg;
    RawTriangle rawtri;
    int         intersect;
    double      t;
    
    switch(num_common_verts)
    {
    case 0:
	VERTS_TO_LINESEG(edge->verts[0], edge->verts[1], lineseg);
	tri_to_rawtri(tri, &rawtri);
	intersect =
	    lineseg_rawtri_intersect(&lineseg, &rawtri, &t, fuzz_factor);
	break;
    case 1:
	
	intersect = FALSE;
	break;
    case 2:
	
	intersect = FALSE;
	break;
    default:
	fprintf(stderr, "edge_tri_intersect: invalid number of common verts (%d)\n",
		num_common_verts);
	exit(-1);
    }

    return intersect;
} 








int tri_tri_intersect(Triangle *tri1, Triangle *tri2, double fuzz_factor,
		      int common_verts[3][2], int num_common_verts,
		      int test_type)
{
    RawTriangle  rawtri1, rawtri2;
    LineSegment  lineseg;
    double       t;
    int          intersect;
    double       cos_theta;
    Edge        *offset_edge;
    Triangle    *other_offset_tri;
    Vertex      *tri_vert, *offset_vert1, *offset_vert2;
    double       tri_vert_dot1, tri_vert_dot2;
    double       offset_vert1_dot, offset_vert2_dot;
    int          and_test;
    int          i;
    
    intersect = FALSE;
    switch(num_common_verts)
    {
    case 0:
	
	tri_to_rawtri(tri1, &rawtri1);
	tri_to_rawtri(tri2, &rawtri2);
	intersect = rawtri_rawtri_intersect(&rawtri1, &rawtri2, fuzz_factor);
	break;
    case 1:
	
	switch(common_verts[0][0])
	{
	case 0:
	    VERTS_TO_LINESEG(tri1->verts[1], tri1->verts[2], lineseg);
	    break;
	case 1:
	    VERTS_TO_LINESEG(tri1->verts[0], tri1->verts[2], lineseg);
	    break;
	case 2:
	    VERTS_TO_LINESEG(tri1->verts[0], tri1->verts[1], lineseg);
	    break;
	default:
	    fprintf(stderr, "tri_tri_intersect: invalid common vert\n");
	    exit(1);
	}
	tri_to_rawtri(tri2, &rawtri2);
	intersect = lineseg_rawtri_intersect(&lineseg, &rawtri2, &t, fuzz_factor);
	if (intersect == TRUE)
	    break;
	switch(common_verts[0][1])
	{
	case 0:
	    VERTS_TO_LINESEG(tri2->verts[1], tri2->verts[2], lineseg);
	    break;
	case 1:
	    VERTS_TO_LINESEG(tri2->verts[0], tri2->verts[2], lineseg);
	    break;
	case 2:
	    VERTS_TO_LINESEG(tri2->verts[0], tri2->verts[1], lineseg);
	    break;
	default:
	    fprintf(stderr, "tri_tri_intersect: invalid common vert\n");
	    exit(1);
	}
	tri_to_rawtri(tri1, &rawtri1);
	intersect = lineseg_rawtri_intersect(&lineseg, &rawtri1, &t, fuzz_factor);
	break;
    case 2:
	switch (test_type)
	{
	case SURFACE_VS_ITSELF:
	    
	    
	    
	    cos_theta = DOTPROD3(tri1->plane_eq, tri2->plane_eq);
	    
	    
	    if (!(((common_verts[1][0] == (common_verts[0][0] + 1) % 3) &&
		   (common_verts[0][1] == (common_verts[1][1] + 1) % 3)) ||
		  ((common_verts[0][0] == (common_verts[1][0] + 1) % 3) &&
		   (common_verts[1][1] == (common_verts[0][1] + 1) % 3))))
	    {
		fprintf(stderr, "tri_tri_intersect: shared edge orientation error\n");
		exit(-1);
	        
	    }
	    
	    
	    if (fuzz_factor >= 0)
		intersect = ((cos_theta <= -1) ? TRUE : FALSE);
	    else
		intersect = ((cos_theta <= (-1-fuzz_factor)) ? TRUE : FALSE);
	    
	    break;
	case SURFACE_VS_OUTER:
	case SURFACE_VS_INNER:
	    
	    
	    for (i=0, offset_edge = NULL; i<3; i++)
		if (((common_verts[0][1] == i) &&
		     (common_verts[1][1] == ((i+1)%3))) ||
		    ((common_verts[0][1] == ((i+1)%3)) &&
		     (common_verts[1][1] == i)))
		    offset_edge = tri2->edges[i];
	    if (offset_edge == NULL)
	    {
		fprintf(stderr, "tri_tri_intersect: couldn't find offset edge\n");
		exit(-1);
	    }
	    
	    if (offset_edge->tris[0] == tri2)
		other_offset_tri = offset_edge->tris[1];
	    else if (offset_edge->tris[1] == tri2)
		other_offset_tri = offset_edge->tris[0];
	    else
	    {
		fprintf(stderr, "tri_tri_intersect: couldn't find other offset tri\n");
		exit(-1);
	    }
	    
	    for (i=0, tri_vert = NULL; i<3; i++)
		if ((common_verts[0][0] != i) && (common_verts[1][0] != i))
		    tri_vert = tri1->verts[i];
	    if (tri_vert == NULL)
	    {
		fprintf(stderr, "tri_tri_intersect: couldn't find tri vert\n");
		exit(-1);
	    }
	    
	    
	    tri_vert_dot1 =
		DOTPROD3(tri_vert->coord, tri2->plane_eq);
	    if (test_type == SURFACE_VS_OUTER)
		tri_vert_dot1 *= -1.0;
	    
	    if (other_offset_tri != NULL) 
	    {
		for (i=0, offset_vert1 = NULL; i<3; i++)
		    if ((tri2->verts[i] != offset_edge->verts[0]) &&
			(tri2->verts[i] != offset_edge->verts[1]))
			offset_vert1 = tri2->verts[i];
		
		for (i=0, offset_vert2 = NULL; i<3; i++)
		    if ((other_offset_tri->verts[i] != offset_edge->verts[0]) &&
			(other_offset_tri->verts[i] != offset_edge->verts[1]))
			offset_vert2 = other_offset_tri->verts[i];
		
		offset_vert1_dot =
		    DOTPROD3(offset_vert1->coord, other_offset_tri->plane_eq);
		offset_vert2_dot =
		    DOTPROD3(offset_vert2->coord, tri2->plane_eq);
		
		if (test_type == SURFACE_VS_OUTER)
		{
		    offset_vert1_dot *= -1.0;
		    offset_vert2_dot *= -1.0;
		}
		
		and_test = FALSE;
		if ((offset_vert1_dot >= 0.0) || (offset_vert2_dot >= 0.0))
		    and_test = TRUE;
		
		tri_vert_dot2 =
		    DOTPROD3(tri_vert->coord, other_offset_tri->plane_eq);
		if (test_type == SURFACE_VS_OUTER)
		    tri_vert_dot2 *= -1.0;
		
		if (((and_test == TRUE) &&
		     ((tri_vert_dot1 >= 0.0) && (tri_vert_dot2 >= 0.0))) ||
		    
		    ((and_test == FALSE) &&
		     ((tri_vert_dot1 >= 0.0) || (tri_vert_dot2 >= 0.0))))
		    intersect = FALSE;
		else
		    intersect = TRUE;
	    }
	    else
	    {
		if (tri_vert_dot1 >= 0.0)
		    intersect = FALSE;
		else
		    intersect = TRUE;
	    }
	    break;
	default:
	    fprintf(stderr, "tri_tri_intersect: invalid test_type\n");
	    exit(-1);
	}
	break;
    case 3:
	switch (test_type)
	{
	case SURFACE_VS_ITSELF:
	    
	    intersect = TRUE;
	    break;
	case SURFACE_VS_OUTER:
	case SURFACE_VS_INNER:
	    
	    intersect = FALSE;
	    break;
	}
	break;
    default:
	fprintf(stderr, "tri_tri_intersect: invalid number of common verts\n");
	exit(1);
	break;
    }
    return intersect;
} 






int rawtri_rawtri_intersect(RawTriangle *tri1, RawTriangle *tri2,
			    double fuzz_factor)
{
    int          i, j;
    LineSegment  edges[2][3];
    double       t;
    
    for (i=0; i<3; i++)
    {
	j= (i+1) % 3;
	
	VEC3_ASN_OP(edges[0][i].endpoints[0], =, tri1->verts[i]);
	VEC3_ASN_OP(edges[0][i].endpoints[1], =, tri1->verts[j]);

	VEC3_ASN_OP(edges[1][i].endpoints[0], =, tri2->verts[i]);
	VEC3_ASN_OP(edges[1][i].endpoints[1], =, tri2->verts[j]);
    }

    for (i=0; i<3; i++)
    {
	if (lineseg_rawtri_intersect(&(edges[0][i]), tri2, &t, fuzz_factor))
	    return TRUE;
	if (lineseg_rawtri_intersect(&(edges[1][i]), tri1, &t, fuzz_factor))
	    return TRUE;
    }
    return FALSE;
} 


int lineseg_rawtri_intersect(LineSegment *lineseg, RawTriangle *tri,
			     double *t, double fuzz_factor)
{ 
    Point	ptmp;
    double	dtmp, N_dot_v;
    int		i, j, k;
    Point	u, v;
    Point	abs_norm;  
    double	alpha, beta;

    
    
    
    
    VEC3_V_OP_V(v, lineseg->endpoints[1], -, lineseg->endpoints[0]);
    N_dot_v = DOTPROD3(tri->plane_eq, v);
    
    if (N_dot_v == 0)	
    {
	*t = MAXDOUBLE;
	return(FALSE);
    }
    *t = ((tri->plane_eq[3] - DOTPROD3(lineseg->endpoints[0], tri->plane_eq)) /
	  N_dot_v);
    
    if ((*t < fuzz_factor) || (*t > (1-fuzz_factor)))
    {
	*t = MAXDOUBLE;
	return (FALSE);
    }
  
    
    VEC3_V_OP_V_OP_S(ptmp, lineseg->endpoints[0], +, v, *, (*t));
    
    
    for(i = 0; i < 3; i++) abs_norm[i] = fabs(tri->plane_eq[i]);
    dtmp=FMAX(abs_norm[X], FMAX(abs_norm[Y], abs_norm[Z]));
    if (dtmp == abs_norm[X])      { i = 0; j = 1; k = 2; }
    else if (dtmp == abs_norm[Y]) { i = 1; j = 2; k = 0; }
    else { i = 2; j = 0; k = 1; }
    
    u[0] = ptmp[j] - tri->verts[0][j];
    v[0] = ptmp[k] - tri->verts[0][k];
    
    u[1] = tri->verts[1][j] - tri->verts[0][j];
    v[1] = tri->verts[1][k] - tri->verts[0][k];
    
    u[2] = tri->verts[2][j] - tri->verts[0][j];
    v[2] = tri->verts[2][k] - tri->verts[0][k];
    
    dtmp = 1.0 / (u[1]*v[2] - v[1]*u[2]);
    alpha = (u[0]*v[2] - v[0]*u[2]) * dtmp;
    beta  = (u[1]*v[0] - v[1]*u[0]) * dtmp;
    
    if ((alpha >= fuzz_factor) && (beta >= fuzz_factor) && 
	(alpha + beta <= 1-fuzz_factor))
    {
	if ((*t >= fuzz_factor) && (*t <= 1-fuzz_factor))
	    return(TRUE); 
    }
    else
	*t = MAXDOUBLE; 
    
    return(FALSE);
} 





