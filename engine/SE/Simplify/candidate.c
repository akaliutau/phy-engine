





#include <simplify.h>
#include <candidate.h>
#include <vertex_removal.h>
#include <intersect.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>







#define BOUNDARY_EDGE(hole, vert1, vert2)			\
       ((((vert2)-(vert1)) == 1) ||				\
	(((hole)->num_edges==(hole)->num_verts) &&		\
	 (((vert2)-(vert1)) == ((hole)->num_verts-1))))






typedef int TupleType[2];
typedef struct double_sort_type
{
    double  key;
    void   *data;
} DoubleSortType;



static int edge_tuple_exists(int *vert_start, TupleType *edge_tuples,
			     int vert1, int vert2, int *tuple_id);
static void compute_candidate_edges(Hole *hole, int **vert_start,
				    TupleType **edge_tuples,
				    Octree *outer_offset,
				    Octree *inner_offset,
				    OctreeData **query_result,
				    double fuzz_factor);
static void compute_candidate_tris(Hole *hole, int **vert_start,
				   TupleType **edge_tuples);
static double find_tri_min_angle(Triangle *tri);
static void rank_sort_candidate_tris(Hole *hole);
static void quick_sort_doubles(DoubleSortType **sorted, int m, int n);
static int mesh_edge(Vertex *vert1, Vertex *vert2);








void compute_candidates(Hole *hole, Octree *outer_offset,
			Octree *inner_offset, OctreeData **query_result,
			double fuzz_factor)
{
    int *vert_start;
    TupleType *edge_tuples;
    
    compute_candidate_edges(hole, &vert_start, &edge_tuples, outer_offset,
			    inner_offset, query_result, fuzz_factor);
    compute_candidate_tris(hole, &vert_start, &edge_tuples);
    rank_sort_candidate_tris(hole);
    return;
} 


static void compute_candidate_edges(Hole *hole, int **vert_start,
				    TupleType **edge_tuples,
				    Octree *outer_offset,
				    Octree *inner_offset,
				    OctreeData **query_result,
				    double fuzz_factor)
{
    int	  i, j;
    int   n;
    int   num_etuples;
    Edge *edge;
    Edge  an_edge;
    
    n = hole->num_verts;
    ALLOCN((*vert_start), int, n);
    ALLOCN((*edge_tuples), TupleType, (n*(n-1))/2);
    
    num_etuples = 0;
    for(i = 0; i < hole->num_verts; i++) 
    {
	
	(*vert_start)[i] = num_etuples;
	for(j = i+1; j < hole->num_verts; j++) 
	{
	    an_edge.verts[0] = hole->verts[i];
	    an_edge.verts[1] = hole->verts[j];
	    
	    if ((BOUNDARY_EDGE(hole, i, j)) ||
		((!(mesh_edge(hole->verts[i], hole->verts[j]))) &&
		 (!(edge_offsets_intersect(outer_offset, inner_offset,
					   query_result,
					   &an_edge, fuzz_factor)))))
	    {
		
		(*edge_tuples)[num_etuples][0] = i; 
		(*edge_tuples)[num_etuples][1] = j;
		num_etuples++;
	    }
	}
    }

    
    ALLOCN(hole->candidate_edges, Edge, num_etuples);
    for(i = 0; i < num_etuples; i++)
    {
	edge = &(hole->candidate_edges[i]);
	edge->id = i;
	edge->verts[0] = hole->verts[(*edge_tuples)[i][0]];
	edge->verts[1] = hole->verts[(*edge_tuples)[i][1]];
	edge->tris[0] = edge->tris[1] = NULL;
	if (BOUNDARY_EDGE(hole, (*edge_tuples)[i][0], (*edge_tuples)[i][1]))
	    edge->handy_mark = 1;  
	else
	    edge->handy_mark = 0;  
    }
    hole->num_candidate_edges = num_etuples;

    return;
} 


static void compute_candidate_tris(Hole *hole, int **vert_start,
				   TupleType **edge_tuples)
{
    int		i, j, k, l;
    int		count;
    Triangle   *tri;
    
    
    for(i = count = 0; i < hole->num_verts-1; i++)
    {
	for(j = (*vert_start)[i]; j < (*vert_start)[i+1]; j++)
	{
	    for(k = j+1; k < (*vert_start)[i+1]; k++)
		if (edge_tuple_exists(*vert_start, *edge_tuples,
				      (*edge_tuples)[j][1],
				      (*edge_tuples)[k][1], &l))
		    count++;
	}
    }

    ALLOCN(hole->candidate_tris, Triangle, count);
    
    
    for(i = count = 0; i < hole->num_verts-1; i++)
	for(j = (*vert_start)[i]; j < (*vert_start)[i+1]; j++)
	    for(k = j+1; k < (*vert_start)[i+1]; k++)
		if (edge_tuple_exists(*vert_start, *edge_tuples,
				      (*edge_tuples)[j][1],
				      (*edge_tuples)[k][1], &l)) 
		{
		    tri = &(hole->candidate_tris[count]);
		    tri->id = count;
		    tri->verts[0] = hole->verts[i];
		    tri->verts[1] = hole->verts[(*edge_tuples)[j][1]];
		    tri->verts[2] = hole->verts[(*edge_tuples)[k][1]];
		    tri->edges[0] = &(hole->candidate_edges[j]);
		    tri->edges[1] = &(hole->candidate_edges[l]);
		    tri->edges[2] = &(hole->candidate_edges[k]);
		    find_plane_eq(tri->verts[0]->coord, tri->verts[1]->coord,
				  tri->verts[2]->coord, tri->plane_eq);
		    tri->handy_mark = -1;
		    count++;
		}
    hole->num_candidate_tris = count;
    FREE(*edge_tuples);
    FREE(*vert_start);
    return;
} 



static int mesh_edge(Vertex *vert1, Vertex *vert2)
{
    int i;
    Vertex *tempvert;
    Edge   *edge;
    
    if (vert1->num_edges > vert2->num_edges)
    {
	tempvert = vert2;
	vert2 = vert1;
	vert1 = tempvert;
    }

    for (i=0; i<vert1->num_edges; i++)
    {
	edge = vert1->edges[i];
	if ((edge->verts[0] == vert2) || (edge->verts[1] == vert2))
	    return TRUE;
    }
    return FALSE;
} 






static int edge_tuple_exists(int *vert_start, TupleType *edge_tuples,
			     int vert1, int vert2, int *tuple_id)
{ 
    int		i;

    i = vert_start[vert1];
    while ((i < vert_start[vert1+1]) && (edge_tuples[i][1] <= vert2))
    {
	if (edge_tuples[i][1] == vert2)
	{
	    *tuple_id = i; 
	    return TRUE;
	}
	i++;
    }
    return FALSE;
} 




static void rank_sort_candidate_tris(Hole *hole)
{
    int              i;
    DoubleSortType  *tri_records;
    DoubleSortType **tri_record_list;

    ALLOCN(hole->ranked_candidate_tris, Triangle *, hole->num_candidate_tris);

    ALLOCN(tri_records, DoubleSortType, hole->num_candidate_tris+1);
    ALLOCN(tri_record_list, DoubleSortType *, hole->num_candidate_tris+1);

    for (i=0; i<hole->num_candidate_tris; i++)
    {
	tri_records[i].key = find_tri_min_angle(&(hole->candidate_tris[i]));
	tri_records[i].data = &(hole->candidate_tris[i]);
	tri_record_list[i] = &(tri_records[i]);
    }
    tri_records[hole->num_candidate_tris].key = -HUGE;
    tri_records[hole->num_candidate_tris].data = NULL;
    tri_record_list[hole->num_candidate_tris] =
	&(tri_records[hole->num_candidate_tris]);
    
    quick_sort_doubles(tri_record_list, 0, hole->num_candidate_tris-1);

    for (i=0; i<hole->num_candidate_tris; i++)
	hole->ranked_candidate_tris[i] =
	    (Triangle *)(tri_record_list[i]->data);

    FREE(tri_record_list);
    FREE(tri_records);

    return;
} 



static double find_tri_min_angle(Triangle *tri)
{ 
    int	   i, j, k;
    Point  diff0, diff1;
    double theta, min_theta;
    
    min_theta = HUGE;
    for(i=0; i < 3; i++)
    {
	j = (i+1)%3;
	k = (j+1)%3;
    
	VEC3_V_OP_V(diff0, tri->verts[j]->coord, -, tri->verts[i]->coord);
	VEC3_V_OP_V(diff1, tri->verts[k]->coord, -, tri->verts[i]->coord);
	NORMALIZE3(diff0);
	NORMALIZE3(diff1);
	theta = DOTPROD3(diff0, diff1);
	if (theta > 1.0) theta = 1.0;
	if (theta < -1.0) theta = -1.0;
	theta = acos(theta);
	if (theta < min_theta) min_theta = theta;
    }
    
    
    min_theta *= 180.0/M_PI;
    
    return min_theta;
} 




static void quick_sort_doubles(DoubleSortType **sorted, int m, int n)
{ 
    int        	    i, j;
    double     	    key; 
    DoubleSortType *temp_ptr;

    if (m < n)
    {
	i = m;
	j = n + 1;
	
	key = sorted[m]->key;
	
	do
	{    
	    while (sorted[++i]->key > key);
	     
	    while (sorted[--j]->key < key);
	    if (i < j)
	    { 
		temp_ptr = sorted[i];
		sorted[i] = sorted[j];
		sorted[j] = temp_ptr;
	    }
	} while (i < j);
	
	
	temp_ptr = sorted[m];
	sorted[m] = sorted[j];
	sorted[j] = temp_ptr;
	
	quick_sort_doubles(sorted, m, j-1);
	quick_sort_doubles(sorted, j+1, n);
    }
    return;
} 



