

#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <ply.h>




typedef struct Vertex {
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
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;
  int vert_count;
  int index;
  Face *face;

  


  ply = ply_write (stdout, 2, elem_names, file_type);

  

  vert_count = 0;
  for (i = 0; i < nfaces; i++)
    vert_count += flist[i]->nverts;

  

  ply_element_count (ply, "vertex", vert_count);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));

  ply_element_count (ply, "face", nfaces);
  ply_describe_property (ply, "face", &face_props[0]);
  ply_describe_other_properties (ply, face_other, offsetof(Face,other_props));

  ply_describe_other_elements (ply, other_elements);

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);

  ply_header_complete (ply);

  

  ply_put_element_setup (ply, "vertex");

  for (i = 0; i < nfaces; i++) {
    face = flist[i];
    for (j = 0; j < face->nverts; j++)
      ply_put_element (ply, (void *) vlist[face->verts[j]]);
  }

  

  ply_put_element_setup (ply, "face");

  index = 0;
  for (i = 0; i < nfaces; i++) {
    face = flist[i];
    for (j = 0; j < face->nverts; j++)
      face->verts[j] = index++;
    ply_put_element (ply, (void *) face);
  }

  ply_put_other_elements (ply);

  
  ply_close (ply);
}

