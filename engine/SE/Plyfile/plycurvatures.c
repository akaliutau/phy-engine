


#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <ply.h>
#include <values.h>

#define FALSE           0
#define TRUE            1

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
#define SQ_DIST3(a, b)          ((a[0]-b[0])*(a[0]-b[0]) +      \
                                 (a[1]-b[1])*(a[1]-b[1]) +      \
                                 (a[2]-b[2])*(a[2]-b[2]))
#define FMAX(x,y) ((x)>(y) ? (x) : (y))
#define FMIN(x,y) ((x)<(y) ? (x) : (y))
#define FP_EQ_EPS( a, b, c )  ((((a) - (b)) <= (c)) && (((a) - (b)) >= -(c)))



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
  float curvature;
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
  {"curvature", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,curvature), 0, 0, 0, 0},
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

int do_averaging;




main(int argc, char *argv[])
{
    init_defaults();
    get_options(argc, argv);    
    read_file();
    find_curvatures();
    write_file();
}

init_defaults()
{
    do_averaging = FALSE;
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
	    case 'a':
		do_averaging = TRUE;
		++argv;
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
  fprintf(stderr, "usage: %s [flags]  <in.ply   >out.ply\n", progname);
  fprintf(stderr, "  -- optional flags -- \n");
  fprintf(stderr, "    -a      : average neighboring vertex curvatures\n");
}










find_curvatures()
{
    find_raw_curvatures();
    scale_curvatures();
    if (do_averaging == TRUE)
	average_curvatures();
}


find_raw_curvatures()
{
    int    i, j;
    float  dot, theta, length, radius;
    Vertex  *vert, *vert2;
    Edge  *edge;
    Vector vec;
    
    for (i=0; i<nverts; i++)
    {
	vert = vlist[i];
	NORMALIZE3(vert->normal);  
	vert->curvature = MAXFLOAT;
	for (j=0; j<vert->nedges; j++)
	{
	    edge = edge_list[vert->edges[j]];
	    vert2 = (edge->vert1 == vert->id) ?
		vlist[edge->vert2] : vlist[edge->vert1];
	    VEC3_V_OP_V(vec, vert2->coord, -, vert->coord);
	    NORMALIZE3(vec);
	    dot = fabs(DOTPROD3(vert->normal, vec));
	    if (FP_EQ_EPS(dot, 0, 1e-2))
		continue; 
	    theta = acos(dot);
	    length = sqrt(SQ_DIST3(vert->coord, vert2->coord));
	    radius = tan(theta) * length / 2.0;
	    vert->curvature = FMIN(vert->curvature, radius);
	}
    }
}

scale_curvatures()
{
    int i;
    Vertex *vert;
#if 0
    float min, max, delta;
#else
    int valid_count;
    float sum, mean;
    float deviation, sum_of_square_deviations, variance, std_deviation;
    float lower, upper, min, max;
#endif
    
#if 0
    min = MAXFLOAT;
    max = 0;
    
    for (i=0; i<nverts; i++)
    {
	vert = vlist[i];
	min = FMIN(min, vert->curvature);
	if (vert->curvature != MAXFLOAT)
	    max = FMAX(max, vert->curvature);
    }

    delta = max - min;
    
    for (i=0; i<nverts; i++)
    {
	vert = vlist[i];
	if (vert->curvature == MAXFLOAT)
	    vert->curvature = 1.0;
	else
	{
	    vert->curvature =
		(vert->curvature - min) / delta;
	}
    }
#else
    for (i=0, valid_count = 0.0, sum = 0.0; i<nverts; i++)
    {
	vert = vlist[i];
	if (vert->curvature == MAXFLOAT)
	    continue;
	sum += vert->curvature;
	valid_count++;
    }
    mean = sum / valid_count;

    for (i=0, sum_of_square_deviations = 0.0; i<nverts; i++)
    {
	vert = vlist[i];
	if (vert->curvature == MAXFLOAT)
	    continue;
	deviation = mean - vert->curvature;
	sum_of_square_deviations += deviation*deviation;
    }
    variance = sum_of_square_deviations / valid_count;
    std_deviation = sqrt(variance);
#if 1
    lower = mean - 3 * std_deviation;
    upper = mean + 3 * std_deviation;

    min = MAXFLOAT;
    max = 0;
    for (i=0; i<nverts; i++)
    {
	vert = vlist[i];
	if ((vert->curvature < min) && (vert->curvature >= lower))
	    min = vert->curvature;
	if ((vert->curvature > max) && (vert->curvature <= upper))
	    max = vert->curvature;
    }
    lower = min;
    upper = max;
#else
    lower = 0.0;
    upper = 2*mean;
#endif

    fprintf(stderr, "Mean: %f\n", mean);
    fprintf(stderr, "Std Deviation %f\n", std_deviation);
    fprintf(stderr, "Lower: %f\n", lower);
    fprintf(stderr, "Upper: %f\n", upper);
    
    for (i=0; i<nverts; i++)
    {
	vert = vlist[i];
	if (vert->curvature >= upper)
	    vert->curvature = 1.0;
	else if (vert->curvature <= lower)
	    vert->curvature = 0.0;
	else
	    vert->curvature = (vert->curvature - lower) / (upper - lower);
    }
#endif
}

average_curvatures()
{
    int     i, j;
    Vertex *vert, *vert2;
    Edge   *edge;
    float   sum;
    
    for (i=0; i<nverts; i++)
    {
	vert = vlist[i];
	sum = vert->curvature;
	for (j=0; j<vert->nedges; j++)
	{
	    edge = edge_list[vert->edges[j]];
	    vert2 = (edge->vert1 == vert->id) ?
		vlist[edge->vert2] : vlist[edge->vert1];
	    sum += vert2->curvature;
	}
	vert->curvature = sum / (vert->nedges + 1);
    }
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
      if ((!has_nx) || (!has_ny) || (!has_nz))
      {
	  fprintf(stderr, "Vertices must have normals\n");
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
  static char *known_elements[] = {"vertex", "face", "edge"};
  
  


  ply = ply_write (stdout, 3, known_elements, file_type);


  

  ply_element_count (ply, "vertex", nverts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_property (ply, "vertex", &vert_props[3]);
  ply_describe_property (ply, "vertex", &vert_props[4]);
  ply_describe_property (ply, "vertex", &vert_props[5]);
  ply_describe_property (ply, "vertex", &vert_props[6]);
  ply_describe_property (ply, "vertex", &vert_props[7]);
  ply_describe_property (ply, "vertex", &vert_props[8]);
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

