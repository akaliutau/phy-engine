


#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <values.h>

#include <ply.h>

#define FALSE           0
#define TRUE            1

#define SQ(a) ((a)*(a))

#define X               0
#define Y               1
#define Z               2



typedef float Point[3];
typedef float Vector[3];

typedef struct Vertex {
  Point coord;
  void *other_props;       
} Vertex;

typedef struct Face {
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




static int nverts,nfaces;
static Vertex **vlist;
static Face **flist;
static PlyOtherElems *other_elements = NULL;
static PlyOtherProp *vert_other,*face_other;
static int nelems;
static char **elist;
static int num_comments;
static char **comments;
static int num_obj_info;
static char **obj_info;
static int file_type;


int has_x, has_y, has_z;

int has_vertex_indices;




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
  fprintf(stderr, "usage: %s [flags]  <in.ply   >out.stl\n", progname);
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

  


  ply  = ply_read (stdin, &nelems, &elist);
  ply_get_info (ply, &version, &file_type);

  for (i = 0; i < nelems; i++) {

    
    elem_name = elist[i];
    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

    if (equal_strings ("vertex", elem_name)) {

      
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
      
      
      for (j = 0; j < num_elems; j++) {
        vlist[j] = (Vertex *) malloc (sizeof (Vertex));
        ply_get_element (ply, (void *) vlist[j]);
      }
    }
    else if (equal_strings ("face", elem_name)) {

      
      flist = (Face **) malloc (sizeof (Face *) * num_elems);
      nfaces = num_elems;

      
      
      has_vertex_indices = FALSE;

      for (j=0; j<nprops; j++)
      {
	  if (equal_strings("vertex_indices", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &face_props[0]);  
	      has_vertex_indices = TRUE;
	  }
      }
      face_other = ply_get_other_properties (ply, elem_name,
                     offsetof(Face,other_props));

      
      if (!has_vertex_indices)
      {
	  fprintf(stderr, "Faces don't have vertex indices\n");
	  exit(-1);
      }
      
      
      for (j = 0; j < num_elems; j++) {
        flist[j] = (Face *) malloc (sizeof (Face));
        ply_get_element (ply, (void *) flist[j]);
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
    int i, j;
    Face *face;
    Vertex *vert0, *vert1, *vert2;
    int v0, v1, v2;
    Vector vec1, vec2, normal;
    
    printf("solid  ply_object\n");
    
    for (i=0; i<nfaces; i++)
    {
	face = flist[i];
	for (j=0, v0=0, v1=1, v2=2;
	     j<face->nverts-2;
	     j++, v1++, v2++)
	{
	    vert0 = vlist[face->verts[v0]];
	    vert1 = vlist[face->verts[v1]];
	    vert2 = vlist[face->verts[v2]];

	    VEC3_V_OP_V(vec1, vert1->coord, -, vert0->coord);
	    VEC3_V_OP_V(vec2, vert2->coord, -, vert0->coord);
	    CROSSPROD3(normal, vec1, vec2);
	    NORMALIZE3(normal);

	    printf("  facet normal % .8E % .8E % .8E\n",
		   normal[X], normal[Y], normal[Z]);
	    printf("    outer loop\n");
	    printf("      vertex % .16E %.16E % .16E\n",
		   vert0->coord[X], vert0->coord[Y], vert0->coord[Z]);
	    printf("      vertex % .16E %.16E % .16E\n",
		   vert1->coord[X], vert1->coord[Y], vert1->coord[Z]);
	    printf("      vertex % .16E %.16E % .16E\n",
		   vert2->coord[X], vert2->coord[Y], vert2->coord[Z]);
	    printf("    endloop\n");
	    printf("  endfacet\n");
	}
    }
    printf("endsolid\n");
}


