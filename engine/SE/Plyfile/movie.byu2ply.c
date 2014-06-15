


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



main(int argc, char *argv[])
{
  int i,j;
  char *s;
  char *progname;

  progname = argv[0];

  while (--argc > 0 && (*++argv)[0]=='-') {
    for (s = argv[0]+1; *s; s++)
      switch (*s) {
        default:
          usage (progname);
          exit (-1);
          break;
      }
  }

  read_file();
  write_file();
}




usage(char *progname)
{
  fprintf (stderr, "usage: %s [flags] <in.movie.byu >out.ply\n", progname);
}




read_file()
{
  int   i, j;
  int   num_parts, total_edges, first_face, last_face;
  int   indices[100];

  if (scanf("%d", &num_parts) != 1)
  {
      fprintf(stderr, "Couldn't read number of parts\n");
      exit(1);
  }
  
  if (num_parts != 1)
  {
      fprintf(stderr, "MOVIE.BYU file has multiple parts - not supported\n");
      exit(1);
  }

  if (scanf("%d", &nverts) != 1)
  {
      fprintf(stderr, "Couldn't read number of vertices\n");
      exit(1);
  }

  if (nverts <= 0)
  {
      fprintf(stderr, "Invalid number of vertices\n");
      exit(1);
  }
  
  if (scanf("%d", &nfaces) != 1)
  {
      fprintf(stderr, "Couldn't read number of faces\n");
      exit(1);
  }
  if (nfaces <= 0)
  {
      fprintf(stderr, "Invalid number of faces\n");
      exit(1);
  }

  if (scanf("%d", &total_edges) != 1)
  {
      fprintf(stderr, "Couldn't read number of edges\n");
      exit(1);
  }
  if (total_edges <= 0)
  {
      fprintf(stderr, "Invalid number of edges\n");
      exit(1);
  }

  if (scanf("%d", &first_face) != 1)
  {
      fprintf(stderr, "Couldn't read first face\n");
      exit(1);
  }
  if (first_face != 1)
  {
      fprintf(stderr, "part doesn't start with face 1\n");
      exit(1);
  }

  if (scanf("%d", &last_face) != 1)
  {
      fprintf(stderr, "Couldn't read last face\n");
      exit(1);
  }
  if (last_face != nfaces)
  {
      fprintf(stderr, "part doesn't finish with last face\n");
      exit(1);
  }
  
      
        
  ALLOCN(vlist, Vertex *, nverts);
  for (i=0;i<nverts;i++)
  {
      ALLOCN(vlist[i], Vertex, 1);
      scanf("%f %f %f", &(vlist[i]->x), &(vlist[i]->y), &(vlist[i]->z));
  }
  
  ALLOCN(flist, Face *, nfaces);
  
  for (i=0;i< nfaces; i++)
  {
      ALLOCN(flist[i], Face, 1);
      for (j=0; j<100; j++)
      {
	  if (scanf("%d", &(indices[j])) != 1)
	  {
	      fprintf(stderr, "couldn't read vertex index\n");
	      exit(1);
	  }
	  if (indices[j] < 0)
	  {
	      indices[j] = -indices[j];
	      j++;
	      break;
	  }
      }
      if (j >= 100)
      {
	  fprintf(stderr, "too many vertex indices on face\n");
	  exit(1);
      }
      flist[i]->nverts = (unsigned char)j;
      ALLOCN(flist[i]->verts, int, flist[i]->nverts);
      for (j=0; j<flist[i]->nverts; j++)
	  flist[i]->verts[j] = indices[j] - 1;
  }
}




write_file()
{
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;

  

  nelems = 2;
  file_type = PLY_BINARY_NATIVE;
  
  ply = ply_write (stdout, nelems, elem_names, file_type);


  

  ply_element_count (ply, "vertex", nverts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
#if 0
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));
#endif
  
  ply_element_count (ply, "face", nfaces);
  ply_describe_property (ply, "face", &face_props[0]);
#if 0
  ply_describe_other_properties (ply, face_other, offsetof(Face,other_props));

  ply_describe_other_elements (ply, other_elements);

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);
#endif
  
  ply_header_complete (ply);

  
  ply_put_element_setup (ply, "vertex");
  for (i = 0; i < nverts; i++)
    ply_put_element (ply, (void *) vlist[i]);

  
  ply_put_element_setup (ply, "face");
  for (i = 0; i < nfaces; i++)
    ply_put_element (ply, (void *) flist[i]);

  ply_put_other_elements (ply);

  
  ply_close (ply);
}

