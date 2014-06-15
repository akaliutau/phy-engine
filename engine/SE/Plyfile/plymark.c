


#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <ply.h>

#define UNKNOWN        -1
#define FALSE           0
#define TRUE            1




typedef struct Vertex {
  unsigned char marked;
  void *other_props;       
} Vertex;

typedef struct Face {
  unsigned char marked;
  void *other_props;       
} Face;

char *elem_names[] = { 
  "vertex", "face"
};

PlyProperty vert_props[] = { 
  {"marked", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,marked), 0, 0, 0, 0},
};

PlyProperty face_props[] = { 
  {"marked", PLY_UCHAR, PLY_UCHAR, offsetof(Face,marked), 0, 0, 0, 0},
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

int has_vmarked, has_fmarked;
int write_vmarked, write_fmarked;
char **marked_verts, **marked_faces;
int num_marked_verts, num_marked_faces;



main(int argc, char *argv[])
{
    init_defaults();
    get_options(argc, argv);    
    read_file();
    map_defaults();
    mark();
    write_file();
}

init_defaults()
{
    write_vmarked = UNKNOWN;
    write_fmarked = UNKNOWN;
    num_marked_verts = num_marked_faces = 0;
}

map_defaults()
{
    if ((write_vmarked == UNKNOWN) && (has_vmarked))
	write_vmarked = TRUE;
    if ((write_fmarked == UNKNOWN) && (has_fmarked))
	write_fmarked = TRUE;
}

get_options(int argc, char *argv[])
{
    char *s;
    char *progname;
    int   dummy;
    
    progname = argv[0];

    while (--argc > 0 && (*++argv)[0]=='-')
    {
	for (s = argv[0]+1; *s; s++)
	    switch (*s)
	    {
	    case 'v':
		++argv;
		marked_verts = argv;
		for (num_marked_verts = 0;
		     ((*argv) && (sscanf((*argv), "%d\n", &dummy) == 1));
		     num_marked_verts++)
		    ++argv;
		write_vmarked = TRUE;
		--argv;
		argc -= num_marked_verts;
		break;
	    case 'f':
		++argv;
		marked_faces = argv;
		for (num_marked_faces = 0;
		     ((*argv) && (sscanf((*argv), "%d\n", &dummy) == 1));
		     num_marked_faces++)
		    ++argv;
		write_fmarked = TRUE;
		--argv;
		argc -= num_marked_faces;
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
  fprintf(stderr, "    -v <id> <id> ...    : vertices to mark\n");
  fprintf(stderr, "    -f <id> <id> ...    : faces to mark\n");
}

mark()
{
    int i, id;

    if ((write_vmarked) && (!has_vmarked))
	for (i=0; i<nverts; i++)
	    vlist[i]->marked = FALSE;
    if ((write_fmarked) && (!has_fmarked))
	for (i=0; i<nfaces; i++)
	    flist[i]->marked = FALSE;
    
    for (i=0; i<num_marked_verts; i++)
    {
	if ((sscanf(marked_verts[i], "%d", &id) != 1) ||
	    (id < 0) || (id >= nverts))
	{
	    fprintf(stderr, "Invalid vertex id\n");
	    exit(-1);
	}
	vlist[id]->marked = TRUE;
    }

    for (i=0; i<num_marked_faces; i++)
    {
	if ((sscanf(marked_faces[i], "%d", &id) != 1) ||
	    (id < 0) || (id >= nfaces))
	{
	    fprintf(stderr, "Invalid face id\n");
	    exit(-1);
	}
	flist[id]->marked = TRUE;
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

      
      vlist = (Vertex **) calloc (sizeof (Vertex *), num_elems);
      nverts = num_elems;

      
      
      has_vmarked = FALSE;
      
      for (j=0; j<nprops; j++)
      {
	  if (equal_strings("marked", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[0]);  
	      has_vmarked = TRUE;
	  }
      }
      vert_other = ply_get_other_properties (ply, elem_name,
					     offsetof(Vertex,other_props));

      
      for (j = 0; j < num_elems; j++) {
        vlist[j] = (Vertex *) calloc (sizeof (Vertex), 1);
        ply_get_element (ply, (void *) vlist[j]);
      }
    }
    else if (equal_strings ("face", elem_name)) {

      
      flist = (Face **) calloc (sizeof (Face *), num_elems);
      nfaces = num_elems;

      
      
      has_fmarked = FALSE;

      for (j=0; j<nprops; j++)
      {
	  if (equal_strings("marked", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &face_props[0]);  
	      has_fmarked = TRUE;
	  }
      }
      face_other = ply_get_other_properties (ply, elem_name,
                     offsetof(Face,other_props));

      
      for (j = 0; j < num_elems; j++) {
        flist[j] = (Face *) calloc (sizeof (Face), 1);
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

  


  ply = ply_write (stdout, 2, elem_names, file_type);


  

  ply_element_count (ply, "vertex", nverts);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));
  if (write_vmarked == TRUE)
      ply_describe_property (ply, "vertex", &vert_props[0]);

  ply_describe_other_properties (ply, face_other, offsetof(Face,other_props));
  ply_element_count (ply, "face", nfaces);
  if (write_fmarked == TRUE)
      ply_describe_property (ply, "face", &face_props[0]);

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
  
  ply_put_other_elements (ply);
  
  
  ply_close (ply);
}

