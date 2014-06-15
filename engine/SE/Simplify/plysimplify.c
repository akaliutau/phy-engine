





#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <ply.h>
#include <simplify.h>
#include <geometry.h>
#include <offset.h>
#include <values.h>



#define BBOX_PERCENT       0
#define ABSOLUTE_DISTANCE  1








typedef struct PlyVertex {
  Point  coord;            
  Vector normal;           
#ifdef VERTEX_EPSILONS
  float epsilon;           
#endif
  unsigned char nfaces;    
  int *faces;              
  unsigned char nedges;    
  int *edges;              
  void *other_props;       
} PlyVertex;

typedef struct PlyFace {
  unsigned char nverts;    
  int *verts;              
  unsigned char nedges;    
  int *edges;              
} PlyFace;

typedef struct PlyEdge {
  int vert1, vert2;        
  int face1, face2;        
} PlyEdge;





void compute_bbox_epsilon(Surface *model, double bbox_epsilon, double *epsilon);
void get_options(int argc, char *argv[]);
void read_file(Surface *model);
void write_file(Surface *model);
void init_defaults();
void usage(char *progname);



PlyProperty vert_props[] = { 
  {"x", PLY_FLOAT, PLY_DOUBLE, offsetof(PlyVertex,coord[X]), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_DOUBLE, offsetof(PlyVertex,coord[Y]), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_DOUBLE, offsetof(PlyVertex,coord[Z]), 0, 0, 0, 0},
  {"nx", PLY_FLOAT, PLY_DOUBLE, offsetof(PlyVertex,normal[X]), 0, 0, 0, 0},
  {"ny", PLY_FLOAT, PLY_DOUBLE, offsetof(PlyVertex,normal[Y]), 0, 0, 0, 0},
  {"nz", PLY_FLOAT, PLY_DOUBLE, offsetof(PlyVertex,normal[Z]), 0, 0, 0, 0},
  {"edge_indices", PLY_INT, PLY_INT, offsetof(PlyVertex,edges),
     1, PLY_UCHAR, PLY_UCHAR, offsetof(PlyVertex,nedges)},
  {"face_indices", PLY_INT, PLY_INT, offsetof(PlyVertex,faces),
     1, PLY_UCHAR, PLY_UCHAR, offsetof(PlyVertex,nfaces)},
 #ifdef VERTEX_EPSILONS
  {"epsilon", PLY_FLOAT, PLY_FLOAT, offsetof(PlyVertex,epsilon), 0, 0, 0, 0},
 #endif
};

PlyProperty face_props[] = { 
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(PlyFace,verts),
     1, PLY_UCHAR, PLY_UCHAR, offsetof(PlyFace,nverts)},
  {"edge_indices", PLY_INT, PLY_INT, offsetof(PlyFace,edges),
     1, PLY_UCHAR, PLY_UCHAR, offsetof(PlyFace,nedges)},
};

PlyProperty edge_props[] = { 
  {"vert1", PLY_INT, PLY_INT, offsetof(PlyEdge,vert1), 0, 0, 0, 0},
  {"vert2", PLY_INT, PLY_INT, offsetof(PlyEdge,vert2), 0, 0, 0, 0},
  {"face1", PLY_INT, PLY_INT, offsetof(PlyEdge,face1), 0, 0, 0, 0},
  {"face2", PLY_INT, PLY_INT, offsetof(PlyEdge,face2), 0, 0, 0, 0},
};



static PlyOtherElems *other_elements = NULL;
static PlyOtherProp *vert_other,*face_other,*edge_other;
static int nelems;
static char **element_list;
static int num_comments;
static char **comments;
static int num_obj_info;
static char **obj_info;
static int file_type;

static int has_x, has_y, has_z;
static int has_nx, has_ny, has_nz;
static int has_vedges, has_vfaces;
static int has_fverts, has_fedges;
static int has_vert1, has_vert2;
static int has_face1, has_face2;

#ifdef VERTEX_EPSILONS
static int has_epsilons;
#endif

static double epsilon, bbox_epsilon;
static int    epsilon_mode;







int main(int argc, char *argv[])
{
    Surface original_model, *simplified_model;
    
    init_defaults();
    get_options(argc, argv);    
    read_file(&original_model);

    if (epsilon_mode == BBOX_PERCENT)
	compute_bbox_epsilon(&original_model, bbox_epsilon, &epsilon);

    simplify(&original_model, &simplified_model, epsilon);
    
    write_file(simplified_model);
    return 0;
} 



void init_defaults()
{
    epsilon_mode = BBOX_PERCENT;
    bbox_epsilon = 1.0;
} 



void get_options(int argc, char *argv[])
{
    char *s;
    char *progname;
    
    progname = argv[0];

    while (--argc > 0 && (*++argv)[0]=='-')
    {
	for (s = argv[0]+1; *s; s++)
	    switch (*s)
	    {
	    case 'e':
		++argv;
		if (sscanf(*argv, "%lf", &epsilon) == 1)
		{
		    epsilon_mode = ABSOLUTE_DISTANCE;
		    argc -= 1;
		}
		else
		{
		    usage(progname);
		    exit(-1);
		}
		break;
	    case 'E':
		++argv;
		if (sscanf(*argv, "%lf", &bbox_epsilon) == 1)
		{
		    epsilon_mode = BBOX_PERCENT;
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


void usage(char *progname)
{
  fprintf(stderr, "usage: %s [flags]  <in.ply   >out.ply\n", progname);
  fprintf(stderr, "  -- optional flags -- \n");
  fprintf(stderr, "    -e <epsilon>  : absolute epsilon distance to offset\n");
  fprintf(stderr, "    -E <epsilon>  : percent of bbox diagonal distance to offset\n");
  fprintf(stderr, "\n");
} 



void compute_bbox_epsilon(Surface *model, double bbox_epsilon, double *epsilon)
{
    int       i, j, k;
    Triangle *tri;
    Vertex   *vert;
    Extents   bbox;
    double    diag;

    
    bbox[HI][X] = bbox[HI][Y] = bbox[HI][Z] = -MAXDOUBLE;
    bbox[LO][X] = bbox[LO][Y] = bbox[LO][Z] = MAXDOUBLE;
    for (i=0; i<model->num_tris; i++)
    {
	tri = &(model->tris[i]);
	for (j=0; j<3; j++)
	{
	    vert = tri->verts[j];
	    for (k=0; k<3; k++)
	    {
		bbox[HI][k] = FMAX(bbox[HI][k], vert->coord[k]);
		bbox[LO][k] = FMIN(bbox[LO][k], vert->coord[k]);
	    }
	}
    }
    
    diag = sqrt(SQ_DIST3(bbox[HI], bbox[LO]));

    *epsilon = diag * bbox_epsilon / 100.0;
    
    return;
} 



void read_file(Surface *model)
{
  int i,j;
  PlyFile *ply;
  int nprops;
  int num_elems;
  PlyProperty **plist;
  char *elem_name;
  float version;
  PlyVertex plyvertex;
  PlyEdge   plyedge;
  PlyFace   plyface;
  Vertex   *vert;
  Edge     *edge;
  Triangle *face;
  
  


  ply  = ply_read (stdin, &nelems, &element_list);
  ply_get_info (ply, &version, &file_type);

  for (i = 0; i < nelems; i++) {

    
    elem_name = element_list[i];
    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

    if (equal_strings ("vertex", elem_name)) {

      
	ALLOCN(model->verts, Vertex, num_elems);
	model->num_verts = num_elems;

      
      
      has_x = has_y = has_z = has_nx = has_ny = has_nz =
	  has_vedges = has_vfaces = FALSE;
      
#ifdef VERTEX_EPSILONS
      has_epsilons = FALSE;
#endif

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
#ifdef VERTEX_EPSILONS
	  else if (equal_strings("epsilon", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[8]);
	      has_epsilons = TRUE;
	  }
#endif
      }
      vert_other = ply_get_other_properties (ply, elem_name,
					     offsetof(PlyVertex,other_props));

      
      if ((!has_x) || (!has_y) || (!has_z))
      {
	  fprintf(stderr, "Vertices must have x, y, and z coordinates\n");
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
      if ((!has_nx) || (!has_ny) || (!has_nz))
      {
	  fprintf(stderr, "Vertices must have x, y, and z normal components\n");
	  exit(-1);
      }

#ifdef VERTEX_EPSILONS
      if (!has_epsilons)
      {
              fprintf(stderr, "Vertices must have epsilons\n");
              exit(-1);
       }
#endif
      
      
      for (j = 0; j < num_elems; j++)
      {
        ply_get_element (ply, (void *) (&plyvertex));
	vert = &(model->verts[j]);
	vert->id = j;
	VEC3_ASN_OP(vert->coord, =, plyvertex.coord);
	VEC3_ASN_OP(vert->normal, =, plyvertex.normal);
	vert->num_tris = plyvertex.nfaces;
	ALLOCN(vert->tris, Triangle *, vert->num_tris);
	VEC_ASN_OP(vert->tris, =, ((Triangle **)plyvertex.faces),
		   plyvertex.nfaces);
	vert->num_edges = plyvertex.nedges;
	ALLOCN(vert->edges, Edge *, vert->num_edges);
	VEC_ASN_OP(vert->edges, =, ((Edge **)plyvertex.edges),
		   plyvertex.nedges);
#ifdef VERTEX_EPSILONS
            vert->epsilon = plyvertex.epsilon;
#endif
        vert->other_props = plyvertex.other_props;
      }
    }
    else if (equal_strings ("face", elem_name)) {

      
      ALLOCN(model->tris, Triangle, num_elems);
      model->num_tris = num_elems;

      
      
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
      
      
      for (j = 0; j < num_elems; j++)
      {
        ply_get_element (ply, (void *) (&plyface));
	if ((plyface.nverts != 3) || (plyface.nedges != 3))
	{
	    fprintf(stderr, "Faces must be triangulated\n");
	    fprintf(stderr, "  face #%d\n", j);
	    fprintf(stderr, "  nverts = %d, nedges = %d\n",
		    plyface.nverts, plyface.nedges);
	    
	    exit(-1);
	}
	face = &(model->tris[j]);
	face->id = j;
	VEC3_ASN_OP(face->verts, =, (Vertex *)plyface.verts);
	VEC3_ASN_OP(face->edges, =, (Edge *)plyface.edges);
      }
    }
    else if (equal_strings ("edge", elem_name)) {

      
      ALLOCN(model->edges, Edge, num_elems);
      model->num_edges = num_elems;

      
      
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
        ply_get_element (ply, (void *) (&plyedge));
	edge = &(model->edges[j]);
	edge->id = j;
	edge->verts[0] = (Vertex *)plyedge.vert1;
	edge->verts[1] = (Vertex *)plyedge.vert2;
	edge->tris[0] = (Triangle *)plyedge.face1;
	edge->tris[1] = (Triangle *)plyedge.face2;
      }
    }
    else
      other_elements = ply_get_other_element (ply, elem_name, num_elems);
  }

  comments = ply_get_comments (ply, &num_comments);
  obj_info = ply_get_obj_info (ply, &num_obj_info);

  ply_close (ply);

  
  for (i=0; i<model->num_verts; i++)
  {
      vert = &(model->verts[i]);
      for (j=0; j<vert->num_tris; j++)
	  vert->tris[j] = &(model->tris[(int)vert->tris[j]]);
      for (j=0; j<vert->num_edges; j++)
	  vert->edges[j] = &(model->edges[(int)vert->edges[j]]);
  }
  for (i=0; i<model->num_tris; i++)
  {
      face = &(model->tris[i]);
      for (j=0; j<3; j++)
      {
	  face->verts[j] = &(model->verts[(int)face->verts[j]]);
	  face->edges[j] = &(model->edges[(int)face->edges[j]]);
      }
  }
  for (i=0; i<model->num_edges; i++)
  {
      edge = &(model->edges[i]);
      for (j=0; j<2; j++)
      {
	  edge->verts[j] = &(model->verts[(int)edge->verts[j]]);
	  if ((int)edge->tris[j] != -1)
	      edge->tris[j] = &(model->tris[(int)edge->tris[j]]);
	  else
	      edge->tris[j] = NULL;
      }
  }

  
  for (i=0; i<model->num_tris; i++)
  {
      face = &(model->tris[i]);
      find_plane_eq(face->verts[0]->coord, face->verts[1]->coord,
		    face->verts[2]->coord, face->plane_eq);
  }
} 



void write_file(Surface *model)
{
  int i,j;
  PlyFile *ply;
  PlyVertex plyvert;
  PlyEdge   plyedge;
  PlyFace   plyface;
  Vertex   *vert;
  Edge     *edge;
  Triangle *face;

  


  ply = ply_write (stdout, nelems, element_list, file_type);


  

  ply_element_count (ply, "vertex", model->num_verts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_property (ply, "vertex", &vert_props[3]);
  ply_describe_property (ply, "vertex", &vert_props[4]);
  ply_describe_property (ply, "vertex", &vert_props[5]);
  ply_describe_property (ply, "vertex", &vert_props[6]);
  ply_describe_property (ply, "vertex", &vert_props[7]);
#ifdef VERTEX_EPSILONS
  ply_describe_property(ply, "vertex", &vert_props[8]);
#endif
  ply_describe_other_properties (ply, vert_other, offsetof(PlyVertex,other_props));

  ply_element_count (ply, "face", model->num_tris);
  ply_describe_property (ply, "face", &face_props[0]);
  ply_describe_property (ply, "face", &face_props[1]);

  ply_element_count (ply, "edge", model->num_edges);
  ply_describe_property (ply, "edge", &edge_props[0]);
  ply_describe_property (ply, "edge", &edge_props[1]);
  ply_describe_property (ply, "edge", &edge_props[2]);
  ply_describe_property (ply, "edge", &edge_props[3]);

  
  ply_describe_other_elements (ply, other_elements);

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);

  ply_header_complete (ply);

  
  ply_put_element_setup (ply, "vertex");
  for (i = 0; i < model->num_verts; i++)
  {
      vert = &(model->verts[i]);
      VEC3_ASN_OP(plyvert.coord, =, vert->coord);
      VEC3_ASN_OP(plyvert.normal, =, vert->normal);
      ALLOCN(plyvert.faces, int, vert->num_tris);
      for (j=0; j<vert->num_tris; j++)
	  plyvert.faces[j] = vert->tris[j]->id;
      plyvert.nfaces = vert->num_tris;
      ALLOCN(plyvert.edges, int, vert->num_edges);
      for (j=0; j<vert->num_edges; j++)
	  plyvert.edges[j] = vert->edges[j]->id;
      plyvert.nedges = vert->num_edges;
#ifdef VERTEX_EPSILONS
      plyvert.epsilon = vert->epsilon;
#endif
      plyvert.other_props = vert->other_props;
      ply_put_element (ply, (void *)&plyvert);
      FREE(plyvert.faces);
      FREE(plyvert.edges);
  }
  
  
  ply_put_element_setup (ply, "face");
  ALLOCN(plyface.verts, int, 3);
  ALLOCN(plyface.edges, int, 3);
  for (i = 0; i < model->num_tris; i++)
  {
      face = &(model->tris[i]);
      for (j=0; j<3; j++)
      {
	  plyface.verts[j] = face->verts[j]->id;
	  plyface.edges[j] = face->edges[j]->id;
      }
      plyface.nverts = plyface.nedges = 3;
      ply_put_element (ply, (void *)&plyface);
  }
  FREE(plyface.verts);
  FREE(plyface.edges);
  
  
  ply_put_element_setup (ply, "edge");
  for (i = 0; i < model->num_edges; i++)
  {
      edge = &(model->edges[i]);
      plyedge.vert1 = edge->verts[0]->id;
      plyedge.vert2 = edge->verts[1]->id;
      plyedge.face1 = edge->tris[0]->id;
      if (edge->tris[1] != NULL)
	  plyedge.face2 = edge->tris[1]->id;
      else
	  plyedge.face2 = -1;
      ply_put_element (ply, (void *)&plyedge);
  }
  
  ply_put_other_elements (ply);
  
  
  ply_close (ply);
} 




