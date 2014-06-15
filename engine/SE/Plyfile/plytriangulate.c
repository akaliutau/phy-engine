


#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <ply.h>

#define FALSE           0
#define TRUE            1

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
  unsigned char nfaces;    
  int *faces;              
  void *other_props;       
} Vertex;

typedef struct Face {
  int id;
  unsigned char nverts;    
  int *verts;              
  void *other_props;       
} Face;

PlyProperty vert_props[] = { 
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,coord[X]), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,coord[Y]), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,coord[Z]), 0, 0, 0, 0},
};

PlyProperty face_props[] = { 
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
     1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
};



static int nverts,nfaces;
static Vertex **vlist;
static Face **flist;
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
int has_fverts;




main(int argc, char *argv[])
{
    get_options(argc, argv);    
    read_file();
    write_file();
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
    
    for (i = 0; i < nelems; i++)
    {
	
	
	elem_name = element_list[i];
	plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);
	
	if (equal_strings ("vertex", elem_name))
	{
	    
	    
	    vlist = (Vertex **) malloc (sizeof (Vertex *) * num_elems);
	    nverts = num_elems;
	    
	    
	    
	    has_x = has_y = has_z = FALSE;
	    
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
	    }
	    
	    vert_other = ply_get_other_properties (ply, elem_name,
						   offsetof(Vertex,other_props));
	    
	    
	    if ((!has_x) || (!has_y) || (!has_z))
	    {
		fprintf(stderr, "Vertices don't have x, y, and z\n");
		exit(-1);
	    }
	    
	    for (j = 0; j < num_elems; j++)
	    {
		vlist[j] = (Vertex *) malloc (sizeof (Vertex));
		ply_get_element (ply, (void *) vlist[j]);
		vlist[j]->id = j;
	    }
	}
	
	else if (equal_strings ("face", elem_name)) {
	    
	    
	    flist = (Face **) malloc (sizeof (Face *) * num_elems);
	    nfaces = num_elems;
	    
	    
	    
	    has_fverts = FALSE;
	    
	    for (j=0; j<nprops; j++)
	    {
		if (equal_strings("vertex_indices", plist[j]->name))
		{
		  ply_get_property(ply, elem_name, &face_props[0]);
		  has_fverts = TRUE;
		}
	    }
	    face_other = ply_get_other_properties (ply, elem_name,
						   offsetof(Face,other_props));
	    
	    
	    if (!has_fverts)
	    {
		fprintf(stderr, "Faces must have vertex indices\n");
		exit(-1);
	    }
	    
	    
	    for (j = 0; j < num_elems; j++) {
		flist[j] = (Face *) malloc (sizeof (Face));
		ply_get_element (ply, (void *) flist[j]);
		flist[j]->id = j;
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
  Face tri_face;
  int  vert_triple[3];
  int  ntris;
  Face *face;
  char *known_elements[] = {"vertex", "face"};
  
  


  ply = ply_write (stdout, 2, known_elements, file_type);


  

  ply_element_count (ply, "vertex", nverts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));

  for (i=0, ntris=0; i<nfaces; i++)
      ntris += flist[i]->nverts-2;
  
  
  ply_element_count (ply, "face", ntris);
  ply_describe_property (ply, "face", &face_props[0]);
  ply_describe_other_properties (ply, face_other, offsetof(Face,other_props));


  
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
  tri_face.nverts = 3;
  tri_face.verts = &(vert_triple[0]);
  
  for (i = 0; i < nfaces; i++)
  {
      face = flist[i];
      for (j=0; j<(face->nverts-2); j++)
      {
	  tri_face.verts[0] = face->verts[0];
	  tri_face.verts[1] = face->verts[j+1];
	  tri_face.verts[2] = face->verts[j+2];
	  tri_face.other_props = face->other_props;
	  
	  ply_put_element (ply, (void *)(&tri_face));
      }
  }
  
  ply_put_other_elements (ply);

  
  ply_close (ply);
}

