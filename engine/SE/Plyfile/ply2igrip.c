


#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <ply.h>




typedef struct Vertex {
  float x,y,z;
  void *other_props;       
} Vertex;

typedef struct Face {
  unsigned char nverts;    
  int *verts;              
  void *other_props;       
} Face;

char *elem_names[] = { 
  "vertex", "face"
};

PlyProperty vert_props[] = { 
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
};

PlyProperty face_props[] = { 
  {"vertex_indices", PLY_INT, PLY_INT, offsetof(Face,verts),
   1, PLY_UCHAR, PLY_UCHAR, offsetof(Face,nverts)},
};




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

static float xtrans = 0;
static float ytrans = 0;
static float ztrans = 0;

static float xscale = 1;
static float yscale = 1;
static float zscale = 1;




main(int argc, char *argv[])
{
  int i,j;
  char *s;
  char *progname;

  progname = argv[0];

  while (--argc > 0 && (*++argv)[0]=='-') {
    for (s = argv[0]+1; *s; s++)
      switch (*s) {
        case 's':
          xscale = atof (*++argv);
          yscale = atof (*++argv);
          zscale = atof (*++argv);
          argc -= 3;
          break;
        case 't':
          xtrans = atof (*++argv);
          ytrans = atof (*++argv);
          ztrans = atof (*++argv);
          argc -= 3;
          break;
        default:
          usage (progname);
          exit (-1);
          break;
      }
  }

  read_file();
  write_file();
  return 0;
}




usage(char *progname)
{
  fprintf (stderr, "usage: %s [flags] <in.ply >out.ply\n", progname);
  fprintf (stderr, "       -t xtrans ytrans ztrans\n");
  fprintf (stderr, "       -s xscale yscale zscale\n");
  fprintf (stderr, "(point = trans_factor + scale_factor * point)\n");
}




transform()
{
  int i;
  Vertex *vert;

  for (i = 0; i < nverts; i++) {
    vert = vlist[i];
    vert->x = xtrans + xscale * vert->x;
    vert->y = ytrans + yscale * vert->y;
    vert->z = ztrans + zscale * vert->z;
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


  


  ply  = ply_read (stdin, &nelems, &elist);
  ply_get_info (ply, &version, &file_type);

  for (i = 0; i < nelems; i++) {

    
    elem_name = elist[i];
    plist = ply_get_element_description (ply, elem_name, &num_elems, &nprops);

    if (equal_strings ("vertex", elem_name)) {

      
      vlist = (Vertex **) malloc (sizeof (Vertex *) * num_elems);
      nverts = num_elems;

      

      ply_get_property (ply, elem_name, &vert_props[0]);
      ply_get_property (ply, elem_name, &vert_props[1]);
      ply_get_property (ply, elem_name, &vert_props[2]);
      vert_other = ply_get_other_properties (ply, elem_name,
                     offsetof(Vertex,other_props));

      
      for (j = 0; j < num_elems; j++) {
        vlist[j] = (Vertex *) malloc (sizeof (Vertex));
        ply_get_element (ply, (void *) vlist[j]);
      }
    }
    else if (equal_strings ("face", elem_name)) {

      
      flist = (Face **) malloc (sizeof (Face *) * num_elems);
      nfaces = num_elems;

      

      ply_get_property (ply, elem_name, &face_props[0]);
      face_other = ply_get_other_properties (ply, elem_name,
                     offsetof(Face,other_props));

      
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
    int i,j;

    if ((nverts <= 0) || (nfaces <= 0))
    {
	fprintf(stderr, "ply2igrip: empty file\n");
	exit(-1);
    }
    
    printf("11\n0\n0\n0\n1\n-2\n");
    printf("%d\n", nverts);
    for (i=0; i<nverts; i++)
	printf("%.10g %.10g %.10g\n", vlist[i]->x, vlist[i]->y, vlist[i]->z);
    printf("0\n");
    printf("%d\n", nfaces);
    for (i=0; i<nfaces; i++)
    {
	printf("%d ", flist[i]->nverts);
	for (j=0; j<flist[i]->nverts; j++)
	    printf("%d ", flist[i]->verts[j]);
	printf("\n");
    }
    printf("0\n0\n");
}
