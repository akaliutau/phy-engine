


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
  fprintf (stderr, "usage: %s [flags] <in.ply >out.ply\n", progname);
  fprintf (stderr, "       -t xtrans ytrans ztrans\n");
  fprintf (stderr, "       -s xscale yscale zscale\n");
  fprintf (stderr, "(point = trans_factor + scale_factor * point)\n");
}




read_file()
{
    int i;
    int max_verts, max_faces;
    char solid[16], facet[16], vertex[16], input_line[256];
    
    if (fgets(input_line, 256, stdin) == NULL)
    {
	fprintf(stderr, "Couldn't read solid input line\n");
	exit(-1);
    }
    
    sscanf(input_line, "%s", solid);
    if (strcmp(solid, "solid") != 0)
    {
	fprintf(stderr, "Couldn't read solid keyword\n");
	exit(-1);
    }
    
    ALLOCN(vlist, Vertex *, 1);
    max_verts = 1;
    nverts = 0;

    ALLOCN(flist, Face *, 1);
    max_faces = 1;
    nfaces = 0;
    
    while (fgets(input_line, 256, stdin) != NULL)
    {
	sscanf(input_line, "%s", facet);
	if (strcmp(facet, "endsolid") == 0)
	    break;
	
	if (strcmp(facet, "facet") != 0)
	{
	    fprintf(stderr, "Couldn't read facet keyword\n");
	    exit(-1);
	}
	fgets(input_line, 256, stdin);  
	
	while ((nverts+3) >= max_verts)
	{
	    REALLOCN(vlist, Vertex *, max_verts, max_verts<<1);
	    max_verts <<= 1;
	}
	
	while ((nfaces+1) >= max_faces)
	{
	    REALLOCN(flist, Face *, max_faces, max_faces<<1);
	    max_faces <<= 1;
	}
	
	ALLOCN(flist[nfaces], Face, 1);
	ALLOCN(flist[nfaces]->verts, int, 3);
	flist[nfaces]->nverts = 3;

	for (i=0; i<3; i++)
	{
	    ALLOCN(vlist[nverts], Vertex, 1);
	    fgets(input_line, 256, stdin);
	    sscanf(input_line, "%s %f %f %f", vertex,
		   &(vlist[nverts]->x), &(vlist[nverts]->y),
		   &(vlist[nverts]->z));
	    flist[nfaces]->verts[i] = nverts;
	    nverts++;
	}
	nfaces++;
	
	fgets(input_line, 256, stdin);  
	fgets(input_line, 256, stdin);  
    }
}




write_file()
{
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;

  

  nelems = 2;
  file_type = PLY_ASCII;
  
  ply = ply_write (stdout, nelems, elem_names, file_type);


  

  ply_element_count (ply, "vertex", nverts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  
  ply_element_count (ply, "face", nfaces);
  ply_describe_property (ply, "face", &face_props[0]);
  
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

