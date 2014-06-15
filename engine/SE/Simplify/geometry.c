





#include <geometry.h>
#include <simplify.h>
#include <math.h>



















void find_plane_eq(Point pt0, Point pt1, Point pt2, double plane_eq[4])
{
    
  Point                 diff1, diff2;

  VEC3_V_OP_V(diff1, pt2, -, pt0);
  VEC3_V_OP_V(diff2, pt2, -, pt1);

  CROSSPROD3(plane_eq, diff1, diff2);
  NORMALIZE3(plane_eq);

  plane_eq[3] = DOTPROD3(plane_eq, pt0);
} 


void tri_to_rawtri(Triangle *tri, RawTriangle *rawtri)
{
    int i;

    for (i=0; i<3; i++)
	VEC3_ASN_OP(rawtri->verts[i], =, tri->verts[i]->coord);
    VEC_ASN_OP(rawtri->plane_eq, =, tri->plane_eq, 4);
    return;
} 



int get_common_index_verts(Triangle *tri1, Triangle *tri2, int common_verts[3][2])
{
    int i, j;
    int num_common_verts;

    num_common_verts = 0;
    for (i=0; i<3; i++)
	for (j=0; j<3; j++)
	    if (tri1->verts[i]->id == tri2->verts[j]->id)
	    {
		common_verts[num_common_verts][0] = i;
		common_verts[num_common_verts][1] = j;
		num_common_verts++;
	    }
    return num_common_verts;
} 


int get_common_coord_verts(Triangle *tri1, Triangle *tri2, int common_verts[3][2])
{
    int i, j;
    int num_common_verts;

    num_common_verts = 0;
    for (i=0; i<3; i++)
	for (j=0; j<3; j++)
	    if (VEC3_EQ(tri1->verts[i]->coord, tri2->verts[j]->coord))
	    {
		common_verts[num_common_verts][0] = i;
		common_verts[num_common_verts][1] = j;
		num_common_verts++;
	    }
    return num_common_verts;
} 



int get_common_coord_edge_tri_verts(Edge *edge, Triangle *tri,
				    int common_verts[2][2])
{
    int i, j;
    int num_common_verts;

    num_common_verts = 0;
    for (i=0; i<2; i++)
	for (j=0; j<3; j++)
	    if (VEC3_EQ(edge->verts[i]->coord, tri->verts[j]->coord))
	    {
		common_verts[num_common_verts][0] = i;
		common_verts[num_common_verts][1] = j;
		num_common_verts++;
	    }
    return num_common_verts;    
} 


double point_line_distance(Vertex *point,
			   Vertex *line_vert1, Vertex *line_vert2)
{
    double plane_eq[4];
    Point  line_closest_point;
    double t;
    double distance;
    
    VEC3_V_OP_V(plane_eq, line_vert2->coord, -, line_vert1->coord);
    NORMALIZE3(plane_eq);  
    plane_eq[3] = DOTPROD3(plane_eq, point->coord);

    t = (plane_eq[3] - DOTPROD3(line_vert1->coord, plane_eq));
    VEC3_V_OP_V_OP_S(line_closest_point, line_vert1->coord, +,
		     plane_eq, *, t);
    distance = sqrt(SQ_DIST3(point->coord, line_closest_point));
    return distance;
} 



void free_surface(Surface *surface)
{
    int i;

    for (i=0; i<surface->num_verts; i++)
    {
	FREE(surface->verts[i].edges);
	FREE(surface->verts[i].tris);
    }
    FREE(surface->verts);
    FREE(surface->edges);
    FREE(surface->tris);
    surface->num_verts = surface->num_edges = surface->num_tris = 0;
    return;
} 







