


#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <ply.h>

#define FALSE           0
#define TRUE            1

#define CCW             0
#define CW              1

#define OUTWARD         0
#define INWARD          1

#define MIN             0
#define MAX             1

#define X               0
#define Y               1
#define Z               2


double LAmag,LAsum; int LAi,LAj,LAk;
#define VEC3_ZERO(a)	       { a[0]=a[1]=a[2]=0; }
#define VEC3_NEG(a,b)           { a[0]= -b[0]; a[1]= -b[1];a[2]= -b[2];}
#define VEC3_V_OP_V(a,b,op,c)  { a[0] = b[0] op c[0]; \
				 a[1] = b[1] op c[1]; \
				 a[2] = b[2] op c[2]; \
				}
#define VEC3_ASN_OP(a,op,b)      {a[0] op b[0]; a[1] op b[1]; a[2] op b[2];}
	    
#define DOTPROD3(a, b)		 (a[0]*b[0] + a[1]*b[1] + a[2]*b[2])

#define CROSSPROD3(a,b,c)       {a[0]=b[1]*c[2]-b[2]*c[1]; \
                                 a[1]=b[2]*c[0]-b[0]*c[2]; \
                                 a[2]=b[0]*c[1]-b[1]*c[0];}

#define NORMALIZE3(a)		{LAmag=1./sqrt(a[0]*a[0]+a[1]*a[1]+a[2]*a[2]);\
				 a[0] *= LAmag; a[1] *= LAmag; a[2] *= LAmag;}

#define ZERO3_TOL(a, tol)      { a[0] = ((a[0]<tol)&&(a[0]>-tol))?0.0:a[0];\
				 a[1] = ((a[1]<tol)&&(a[1]>-tol))?0.0:a[1];\
			         a[2] = ((a[2]<tol)&&(a[2]>-tol))?0.0:a[2];\
			       }



typedef float Point[3];
typedef float Vector[3];

typedef struct Vertex {
  int    id;
  Point  coord;            
  Vector normal;           
  unsigned char nfaces;    
  int *faces;              
  unsigned char nedges;    
  int *edges;              
  void *other_props;       
  unsigned char oriented;  
} Vertex;

typedef struct Face {
  int id;
  unsigned char nverts;    
  int *verts;              
  unsigned char nedges;    
  int *edges;              
  void *other_props;       
  unsigned char oriented;  
} Face;

typedef struct Edge {
  int id;
  int vert1, vert2;        
  int face1, face2;        
  void *other_props;       
} Edge;


PlyProperty vert_props[] = { 
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,coord[X]), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,coord[Y]), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,coord[Z]), 0, 0, 0, 0},
  {"nx", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,normal[X]), 0, 0, 0, 0},
  {"ny", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,normal[Y]), 0, 0, 0, 0},
  {"nz", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,normal[Z]), 0, 0, 0, 0},
  {"edge_indices", PLY_INT, PLY_INT, offsetof(Vertex,edges),
     1, PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,nedges)},
  {"face_indices", PLY_INT, PLY_INT, offsetof(Vertex,faces),
     1, PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,nfaces)},
};

PlyProperty face_props[] = { 
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
     1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
  {"edge_indices", PLY_INT, PLY_INT, offsetof(Face,edges),
     1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nedges)},
};

PlyProperty edge_props[] = { 
  {"vert1", PLY_INT, PLY_INT, offsetof(Edge,vert1), 0, 0, 0, 0},
  {"vert2", PLY_INT, PLY_INT, offsetof(Edge,vert2), 0, 0, 0, 0},
  {"face1", PLY_INT, PLY_INT, offsetof(Edge,face1), 0, 0, 0, 0},
  {"face2", PLY_INT, PLY_INT, offsetof(Edge,face2), 0, 0, 0, 0},
};



static int nverts,nfaces,nedges;
static Vertex **vlist;
static Face **flist;
static Edge **edge_list;
static PlyOtherElems *other_elements = NULL;
static PlyOtherProp *vert_other,*face_other,*edge_other;
static int nelems;
static char **element_list;
static int num_comments;
static char **comments;
static int num_obj_info;
static char **obj_info;
static int file_type;

int has_x, has_y, has_z;
int has_nx, has_ny, has_nz;
int has_vedges, has_vfaces;
int has_fverts, has_fedges;
int has_vert1, has_vert2;
int has_face1, has_face2;
int has_normals;

int vertex_orientation, normal_direction;

int num_oriented_faces, reorient_count, reorient_norm_count;



main(int argc, char *argv[])
{
    init_defaults();
    get_options(argc, argv);    
    read_file();
    orient();
    write_file();
    return 0;
}

init_defaults()
{
    vertex_orientation = CCW;
    normal_direction  = OUTWARD;
}

get_options(int argc, char *argv[])
{
    char *s;
    char *progname;
    
    progname = argv[0];

    while (--argc > 0 && (*++argv)[0]=='-')
    {
	for (s = argv[0]+1; *s; s++)
	    switch (*s)
	    {
	    case 'v':
		++argv;
		if (equal_strings(*argv, "ccw"))
		{
		    vertex_orientation = CCW;
		    argc -= 1;
		}
		else if (equal_strings(*argv, "cw"))
		{
		    vertex_orientation = CW;
		    argc -= 1;
		}
		else
		{
		    usage(progname);
		    exit(-1);
		}
		break;
	    case 'n':
		++argv;
		if (equal_strings(*argv, "outward"))
		{
		    normal_direction = OUTWARD;
		    argc -= 1;
		}
		else if (equal_strings(*argv, "inward"))
		{
		    normal_direction = INWARD;
		    argc -= 1;
		}
		else
		{
		    usage(progname);
		    exit(-1);
		}
		break;
	    default:
		usage (progname);
		exit (-1);
		break;
	    }
    }
}



usage(char *progname)
{
  fprintf(stderr, "usage: %s [flags]  <in.ply   >out.ascarch\n", progname);
  fprintf(stderr, "  -- optional flags -- \n");
  fprintf(stderr, "    -v ccw        : counter-clockwise vertices (default)\n");
  fprintf(stderr, "    -v cw         : clockwise vertices\n");
  fprintf(stderr, "    -n outward    : outward-pointing normals (default)\n");
  fprintf(stderr, "    -n inward     : inward-pointing normals\n");
}


reorient(Face *face)
{
    int i, temp;

    if (face->nverts != face->nedges)
    {
	fprintf(stderr, "Face's vertex and edge counts disagree\n");
	exit(-1);
    }

    
    for (i=0; i < (face->nverts/2); i++)
    {
	temp = face->verts[i];
	face->verts[i] = face->verts[face->nverts-i-1];
	face->verts[face->nverts-i-1] = temp;

	temp = face->edges[i];
	face->edges[i] = face->edges[face->nedges-i-1];
	face->edges[face->nedges-i-1] = temp;
    }

    
    temp = face->edges[0];
    for (i=0; i<(face->nedges-1); i++)
	face->edges[i] = face->edges[i+1];
    face->edges[face->nedges-1] = temp;
    
    reorient_count++;
}

Face *get_oriented_face()
{
    int     i, j, k, m;
    Vertex *extrema[2][3];
    Face   *face, *temp_face;
    int     vert_index;
    Vertex *vert, *vert1, *vert2;
    Edge   *edge;
    Vector  vec1, vec2,  temp_normal, normal;
    char    reorient_face, axis, min_max;
    
    
    for (i=0; i<2; i++)
	for (j=0; j<3; j++)
	    extrema[i][j] = NULL;	
    for (i=0; i<nverts; i++)
    {
	vert = vlist[i];
	if (vert->nfaces)
	{
	    for (j=0; j<vert->nfaces; j++)
	    {
		if (flist[vert->faces[j]]->oriented == FALSE)
		{
		    for (k=0; k<2; k++)
			for (m=0; m<3; m++)
			    extrema[k][m] = vert;
		    break;
		}
	    }
	}
	if (extrema[0][0])
	    break;
    }
    if (extrema[0][0] == NULL)
    {
	fprintf(stderr, "Couldn't find unoriented vertex\n");
	exit(-1);
    }

    
    for (i=0; i<nverts; i++)
    {
	vert = vlist[i];
	for (j=0; j<3; j++)
	{
	    if ((vert->coord[j] < extrema[MIN][j]->coord[j]) &&
		(vert->nfaces))
	    {
		for (k=0; k<vert->nfaces; k++)
		    if (flist[vert->faces[k]]->oriented == FALSE)
		    {
			extrema[MIN][j] = vert;
			break;
		    }
	    }
	    if ((vert->coord[j] > extrema[MAX][j]->coord[j]) &&
		(vert->nfaces))
	    {
		for (k=0; k<vert->nfaces; k++)
		    if (flist[vert->faces[k]]->oriented == FALSE)
		    {
			extrema[MAX][j] = vert;
			break;
		    }
	    }
	}	
    }

    
    for (min_max=0; min_max<2; min_max++)
    {
	for (axis=0; axis<3; axis++)
	{
	    vert = extrema[min_max][axis];
	    face = NULL;
	    VEC3_ZERO(normal);
	    
	    
	    for (j=0; j<vert->nfaces; j++)
	    {
		temp_face = flist[vert->faces[j]];

		
		for (k=0, vert_index = -1; k<temp_face->nverts; k++)
		    if (temp_face->verts[k] == vert->id)
		    {
			vert_index = k;
			break;
		    }
		if (vert_index == -1)
		{
		    fprintf(stderr, "Couldn't find extremal vertex on face\n");
		    fprintf(stderr, "Extremal vert id == %d\n", vert->id);
		    exit(-1);
		}
		
		
		vert1 =
		    vlist[temp_face->verts[(vert_index+1)%
					   (temp_face->nverts)]];
		vert2 =
		    vlist[temp_face->verts[(vert_index+temp_face->nverts-1)
					   %(temp_face->nverts)]];
		VEC3_V_OP_V(vec1, vert1->coord, -, vert->coord);
		VEC3_V_OP_V(vec2, vert2->coord, -, vert->coord);
		CROSSPROD3(temp_normal, vec1, vec2);
		NORMALIZE3(temp_normal);

		if (fabs(temp_normal[axis]) > fabs(normal[axis]))
		{
		    VEC3_ASN_OP(normal, =, temp_normal);
		    if (temp_face->oriented == FALSE)
			face = temp_face;
		    else
			face = NULL;
		}
	    }
	    if (face)
		break;
	}  
	if (face)        
	    break;
    }  
    
    

    
    if (!face)
    {
	fprintf(stderr, "Couldn't find a face to orient\n");
	exit(-1);
    }
    
    
    if (((min_max == 0) && (normal[axis] < 0)) ||
	((min_max == 1) && (normal[axis] > 0)))
	reorient_face = FALSE;
    else
	reorient_face = TRUE;
    
    if (vertex_orientation == CW)
	reorient_face = ((reorient_face == TRUE) ? FALSE : TRUE);
    
    if (reorient_face == TRUE)
	reorient(face);
    
    face->oriented = TRUE;
    num_oriented_faces++;
    return face;
}

orient_neighbors(Face *face)
{
    int     i, j;
    Vertex *vert1, *vert2;
    Edge   *edge;
    Face   *adjacent_face;
    int     edge_index;
    Vertex *adjacent_vert1, *adjacent_vert2;
    char    reorient_face;
    
    if (face->nverts != face->nedges)
    {
	fprintf(stderr, "Face's edge and vertex counts disagree\n");
	exit(-1);
    }
    
    for (i=0; i<face->nverts; i++)
    {
	vert1 = vlist[face->verts[i]];
	vert2 = vlist[face->verts[(i+1)%(face->nverts)]];
	edge = edge_list[face->edges[i]];
	if (((edge->vert1 != vert1->id) && (edge->vert2 != vert1->id)) ||
	    ((edge->vert1 != vert2->id) && (edge->vert2 != vert2->id)))
	{
	    fprintf(stderr, "Face's edges and vertices out of sync\n");
	    exit(-1);
	}
	
	if ((edge->face1 == -1) || (edge->face2 == -1))
	    continue;
   
	adjacent_face = ((edge->face1 == face->id) ?
			 flist[edge->face2] : flist[edge->face1]);

	for (j=0, edge_index = -1; j<adjacent_face->nedges; j++)
	{
	    if (adjacent_face->edges[j] == edge->id)
	    {
		edge_index = j;
		break;
	    }
	}
	if (edge_index == -1)
	{
	    fprintf(stderr, "Couldn't find edge on adjacent face\n");
	    exit(-1);
	}

	adjacent_vert1 =
	    vlist[adjacent_face->verts[edge_index]];
	adjacent_vert2 =
	    vlist[adjacent_face->verts[(edge_index+1)%(adjacent_face->nverts)]];
	if ((vert1 == adjacent_vert1) && (vert2 == adjacent_vert2))
	    reorient_face = TRUE;
	else if ((vert1 == adjacent_vert2) && (vert2 == adjacent_vert1))
	    reorient_face = FALSE;
	else
	{
	    fprintf(stderr, "Adjacent face's edges and vertices out of sync\n");
	    exit(-1);
	}

	if (adjacent_face->oriented == TRUE)
	{
	    if (reorient_face == TRUE)
	    {
		fprintf(stderr, "Non-orientable surface\n");
		exit(-1);
	    }
	    else
		continue;
	}
	
	if (reorient_face == TRUE)
	    reorient(adjacent_face);
	adjacent_face->oriented = TRUE;
	num_oriented_faces++;
	orient_neighbors(adjacent_face);
    }
}


orient_normals()
{
    int i, j;
    Vector vec1, vec2, face_norm;
    Face *face;
    Vertex *vert, *vert1, *vert2;
    double dot, temp_dot;
    Vector normal;
    
    if (has_normals == FALSE)
	return;

    reorient_norm_count = 0;


#if 0
    for (i=0; i<nverts; i++)
    {
	vert = vlist[i];

	dot = 0.0;
	for (j=0; j<vert->nfaces; j++)
	{
	    face = flist[vert->faces[j]];
	    vert1 = vlist[face->verts[(j+1)%(face->nverts)]];
	    vert2 = vlist[face->verts[(j+face->nverts-1)
				      %(face->nverts)]];
	    VEC3_V_OP_V(vec1, vert1->coord, -, vert->coord);
	    VEC3_V_OP_V(vec2, vert2->coord, -, vert->coord);
	    if (vertex_orientation == CCW)
	    {
		CROSSPROD3(normal, vec1, vec2);
	    }
	    else
	    {
		CROSSPROD3(normal, vec2, vec1);
	    }
	    NORMALIZE3(normal);
	    temp_dot = DOTPROD3(normal, vert->normal);
	    if (fabs(temp_dot) > fabs(dot))
		dot = temp_dot;
	}
	if (((dot < 0) && (normal_direction == OUTWARD)) ||
	    ((dot > 0) && (normal_direction == INWARD)))
	{
	    VEC3_NEG(vert->normal, vert->normal);
	    reorient_norm_count++;
	}
    }
    

#else
    
    for (i=0; i<nverts; i++)
	vlist[i]->oriented = FALSE;

    for (i=0; i<nfaces; i++)
    {
	face = flist[i];
	if (face->nverts < 3)
	{
	    fprintf(stderr, "Face %d has less than 3 vertices\n",
		    i, face->nverts);
	    exit(-1);
	}
	for (j=0; j<face->nverts; j++)
	{
	     
	    vert = vlist[face->verts[j]];
	    vert1 = vlist[face->verts[(j+1)%(face->nverts)]];
	    vert2 =
		vlist[face->verts[(j+face->nverts-1)%(face->nverts)]];
	    VEC3_V_OP_V(vec1, vert1->coord, -, vert->coord);
	    VEC3_V_OP_V(vec2, vert2->coord, -, vert->coord);
	    if (vertex_orientation == CCW)
	    {
		CROSSPROD3(face_norm, vec1, vec2);
	    }
	    else if (vertex_orientation == CW)
	    {
		CROSSPROD3(face_norm, vec2, vec1);
	    }
	    else
	    {
		fprintf(stderr, "Invalid vertex orientation\n");
		exit(-1);
	    }

	    dot = DOTPROD3(face_norm, vert->normal);

	    if (((normal_direction == OUTWARD) && (dot < 0.0)) ||
		((normal_direction == INWARD) && (dot > 0.0)))
	    {
		if (vert->oriented == TRUE)
		{
		    fprintf(stderr, "Warning: ");
		    fprintf(stderr,
			    "Couldn't orient the normal of vertex %d\n",
			    vert->id);
		    continue;
		}
		VEC3_NEG(vert->normal, vert->normal);
		reorient_norm_count++;
		vert->oriented = TRUE;
	    }
	}
    }
#endif


    fprintf(stderr, "Number of reoriented normals: %d\n",
	    reorient_norm_count);
}



orient_faces()
{
    int i;
    Face *face;
    
    for (i=0; i<nfaces; i++)
	flist[i]->oriented = FALSE;
    num_oriented_faces = 0;
    reorient_count = 0;;
    while (num_oriented_faces < nfaces)
    {
	face = get_oriented_face();
	orient_neighbors(face);
    }
    fprintf(stderr, "Number of reoriented faces: %d\n", reorient_count);
}




int get_vertex_orientation(Vertex *vert)
{
    Edge   *edge;
    Face   *face;
    Vertex *other_vert;
    int     i;
    Vertex *v1, *v2;
    
    edge = edge_list[vert->edges[0]];
    face = flist[vert->faces[0]];
    other_vert = (vert == vlist[edge->vert1]) ?
	vlist[edge->vert2] : vlist[edge->vert1];

    for (i=0; i<face->nverts; i++)
    {
	v1 = vlist[face->verts[i]];
	v2 = vlist[face->verts[(i+1)%face->nverts]];

	if ((v1 == vert) && (v2 == other_vert))
	    return vertex_orientation;
	if ((v1 == other_vert) && (v2 == vert))
	    return ((vertex_orientation == CCW) ? CW : CCW);
    }
    
    fprintf(stderr, "Couldn't get vertex orientation\n");
    exit(1);
		
} 



void reverse_vertex_orientation(Vertex *vert)
{
    int i, temp;

    
    for (i=0; i< (vert->nedges/2); i++)
    {
	temp = vert->edges[i];
	vert->edges[i] = vert->edges[vert->nedges-i-1];
	vert->edges[vert->nedges-i-1] = temp;
    }
    
    
    for (i=0; i< (vert->nfaces/2); i++)
    {
	temp = vert->faces[i];
	vert->faces[i] = vert->faces[vert->nfaces-i-1];
	vert->faces[vert->nfaces-i-1] = temp;
    }
    return;
} 


orient_vertices()
{
    int i;
    int orientation;
    
    for (i=0; i<nverts; i++)
    {
	orientation = get_vertex_orientation(vlist[i]);
	if (orientation != vertex_orientation)
	    reverse_vertex_orientation(vlist[i]);
    }
    return;
} 

    
orient()
{
    orient_faces();
    orient_vertices();
    orient_normals();
}










read_file()
{
  int i,j,k;
  PlyFile *ply;
  int nprops;
  int num_elems;
  PlyProperty **plist;
  char *elem_name;
  float version;

  


  ply  = ply_read (stdin, &nelems, &element_list);
  ply_get_info (ply, &version, &file_type);

  for (i = 0; i < nelems; i++) {

    
    elem_name = element_list[i];
    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

    if (equal_strings ("vertex", elem_name)) {

      
      vlist = (Vertex **) malloc (sizeof (Vertex *) * num_elems);
      nverts = num_elems;

      
      
      has_x = has_y = has_z = has_nx = has_ny = has_nz =
	  has_vedges = has_vfaces = FALSE;
      
      for (j=0; j<nprops; j++)
      {
	  if (equal_strings("x", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[0]);  
	      has_x = TRUE;
	  }
	  else if (equal_strings("y", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[1]);  
	      has_y = TRUE;
	  }
	  else if (equal_strings("z", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[2]);  
	      has_z = TRUE;
	  }
	  else if (equal_strings("nx", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[3]);  
	      has_nx = TRUE;
	  }
	  else if (equal_strings("ny", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[4]);  
	      has_ny = TRUE;
	  }
	  else if (equal_strings("nz", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[5]);  
	      has_nz = TRUE;
	  }
	  else if (equal_strings("edge_indices", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[6]);
	      has_vedges = TRUE;
	  }
	  else if (equal_strings("face_indices", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[7]);
	      has_vfaces = TRUE;
	  }
      }
      vert_other = ply_get_other_properties (ply, elem_name,
					     offsetof(Vertex,other_props));

      
      if ((!has_x) || (!has_y) || (!has_z))
      {
	  fprintf(stderr, "Vertices don't have x, y, and z\n");
	  exit(-1);
      }
      if (!has_vedges)
      {
	  fprintf(stderr, "Vertices must have edge indices\n");
	  exit(-1);
      }
      if (!has_vfaces)
      {
	  fprintf(stderr, "Vertices must have face indices\n");
	  exit(-1);
      }
      if (has_nx && has_ny && has_nz)
	  has_normals = TRUE;
      else if ((!has_nx) && (!has_ny) && (!has_nz))
	  has_normals = FALSE;
      else
      {
	  fprintf(stderr, "Vertices have partial normals\n");
	  exit(-1);
      }
      
      
      for (j = 0; j < num_elems; j++) {
        vlist[j] = (Vertex *) malloc (sizeof (Vertex));
        ply_get_element (ply, (void *) vlist[j]);
	vlist[j]->id = j;
      }
    }
    else if (equal_strings ("face", elem_name)) {

      
      flist = (Face **) malloc (sizeof (Face *) * num_elems);
      nfaces = num_elems;

      
      
      has_fverts = has_fedges = FALSE;

      for (j=0; j<nprops; j++)
      {
	  if (equal_strings("vertex_indices", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &face_props[0]);  
	      has_fverts = TRUE;
	  }
	  if (equal_strings("edge_indices", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &face_props[1]);
	      has_fedges = TRUE;
	  }
      }
      face_other = ply_get_other_properties (ply, elem_name,
                     offsetof(Face,other_props));

      
      if (!has_fverts)
      {
	  fprintf(stderr, "Faces must have vertex indices\n");
	  exit(-1);
      }
      if (!has_fedges)
      {
	  fprintf(stderr, "Faces must have edge indices\n");
	  exit(-1);
      }
      
      
      for (j = 0; j < num_elems; j++) {
        flist[j] = (Face *) malloc (sizeof (Face));
        ply_get_element (ply, (void *) flist[j]);
	flist[j]->id = j;
      }
    }
    else if (equal_strings ("edge", elem_name)) {

      
      edge_list = (Edge **) malloc (sizeof (Edge *) * num_elems);
      nedges = num_elems;

      
      
      has_vert1 = has_vert2 = has_face1 = has_face2 = FALSE;

      for (j=0; j<nprops; j++)
      {
	  if (equal_strings("vert1", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &edge_props[0]);
	      has_vert1 = TRUE;
	  }
	  if (equal_strings("vert2", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &edge_props[1]);
	      has_vert2 = TRUE;
	  }
	  if (equal_strings("face1", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &edge_props[2]);
	      has_face1 = TRUE;
	  }
	  if (equal_strings("face2", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &edge_props[3]);
	      has_face2 = TRUE;
	  }
      }
      edge_other = ply_get_other_properties (ply, elem_name,
					     offsetof(Edge,other_props));

      
      if ((!has_vert1) || (!has_vert2))
      {
	  fprintf(stderr, "Edges must have vertex indices\n");
	  exit(-1);
      }
      if ((!has_face1) || (!has_face2))
      {
	  fprintf(stderr, "Edges must have face indices\n");
	  exit(-1);
      }
      
      
      for (j = 0; j < num_elems; j++) {
        edge_list[j] = (Edge *) malloc (sizeof (Edge));
        ply_get_element (ply, (void *) edge_list[j]);
	edge_list[j]->id = j;
      }
    }
    else
      other_elements = ply_get_other_element (ply, elem_name, num_elems);
  }

  comments = ply_get_comments (ply, &num_comments);
  obj_info = ply_get_obj_info (ply, &num_obj_info);

  ply_close (ply);
}




write_file()
{
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;
  char *known_elements[] = {"vertex", "face", "edge"};
  int   num_known_elements = 3;
  
  


  ply = ply_write (stdout, num_known_elements, known_elements, file_type);


  

  ply_element_count (ply, "vertex", nverts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  if (has_nx)
      ply_describe_property (ply, "vertex", &vert_props[3]);
  if (has_ny)
      ply_describe_property (ply, "vertex", &vert_props[4]);
  if (has_nz)
      ply_describe_property (ply, "vertex", &vert_props[5]);
  ply_describe_property (ply, "vertex", &vert_props[6]);
  ply_describe_property (ply, "vertex", &vert_props[7]);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));

  ply_element_count (ply, "face", nfaces);
  ply_describe_property (ply, "face", &face_props[0]);
  ply_describe_property (ply, "face", &face_props[1]);
  ply_describe_other_properties (ply, face_other, offsetof(Face,other_props));

  ply_element_count (ply, "edge", nedges);
  ply_describe_property (ply, "edge", &edge_props[0]);
  ply_describe_property (ply, "edge", &edge_props[1]);
  ply_describe_property (ply, "edge", &edge_props[2]);
  ply_describe_property (ply, "edge", &edge_props[3]);
  ply_describe_other_properties (ply, edge_other, offsetof(Edge,other_props));

  
  ply_describe_other_elements (ply, other_elements);

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);

  ply_header_complete (ply);

  
  ply_put_element_setup (ply, "vertex");
  for (i = 0; i < nverts; i++)
    ply_put_element (ply, (void *) vlist[i]);

  
  ply_put_element_setup (ply, "face");
  for (i = 0; i < nfaces; i++)
    ply_put_element (ply, (void *) flist[i]);

  
  ply_put_element_setup (ply, "edge");
  for (i = 0; i < nedges; i++)
    ply_put_element (ply, (void *) edge_list[i]);

  ply_put_other_elements (ply);

  
  ply_close (ply);
}

