


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



int main(int argc, char *argv[])
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

  return 0;
}




usage(char *progname)
{
  fprintf (stderr, "usage: %s [flags] <in.ply >out.ply\n", progname);
  fprintf (stderr, "       -t xtrans ytrans ztrans\n");
  fprintf (stderr, "       -s xscale yscale zscale\n");
  fprintf (stderr, "(point = trans_factor + scale_factor * point)\n");
}




read_file()
{
  int   i, j, temp_int;
  int   version_number, coordinate_system_count, curve_count;
  int   surface_count, object_count, color_index, num_lines;
  float fdummy;
  
  scanf("%d \n", &version_number);
  scanf("%d \n", &coordinate_system_count);
  if (coordinate_system_count)
  {
      for(i = 0; i < coordinate_system_count; i++)
      { for(j = 0; j < 12; j++)
          scanf("%f", &fdummy);
      }
  }

  scanf("%d \n", &curve_count);
  scanf("%d \n", &surface_count);
  scanf("%d \n", &object_count);
  scanf("%d \n", &color_index);
  scanf("%d \n", &nverts);

  if (curve_count)
  {
      fprintf(stderr, "ERROR: no curves allowed in igrip file\n");
      exit(-1);
  }

  if (surface_count)
  {
      fprintf(stderr, "ERROR: no surfaces allowed in igrip file\n");
      exit(-1);
  }

  if (object_count != 1)
  {
      fprintf(stderr, "ERROR: must have exactly one object in igrip file\n");
      exit(-1);
  }

  if (nverts < 3)
  {
      fprintf(stderr, "ERROR: must have at least 3 vertices in igrip file\n");
      exit(-1);
  }
  
        
  ALLOCN(vlist, Vertex *, nverts);
  for (i=0;i<nverts;i++)
  {
      ALLOCN(vlist[i], Vertex, 1);
      scanf("%f %f %f", &(vlist[i]->x), &(vlist[i]->y), &(vlist[i]->z));
  }
  
  scanf("%d", &num_lines);
  if (num_lines != 0)
  {
      fprintf(stderr,"ERROR: not equipped to handle lines in the igrip file\n");
      exit(-1);
  }
    
  scanf("%d ", &nfaces);
  ALLOCN(flist, Face *, nfaces);
  
  for (i=0;i< nfaces; i++)
  {
      ALLOCN(flist[i], Face, 1);
      scanf("%d", &(temp_int));
      if (temp_int > (1<<(8*sizeof(unsigned char))))
      {
	  fprintf(stderr, "Face %d has %d verts -- too many\n", i, temp_int);
	  exit(-1);
      }
      
      flist[i]->nverts = (unsigned char)temp_int;
      ALLOCN(flist[i]->verts, int, flist[i]->nverts);
      for (j=0; j<flist[i]->nverts; j++)
	  scanf("%d",&(flist[i]->verts[j]));
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

