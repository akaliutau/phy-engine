


#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <ply.h>

#define FALSE           0
#define TRUE            1

#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#define FP_EQ_EPS( a, b, c )  ((((a) - (b)) < (c)) && (((a) - (b)) > -(c)))



typedef struct Vertex {
  float curvature;
  unsigned char red, green, blue;
  void *other_props;       
} Vertex;

PlyProperty vert_props[] = { 
  {"curvature", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,curvature), 0, 0, 0, 0},
  {"red", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,red), 0, 0, 0, 0},
  {"green", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,green), 0, 0, 0, 0},
  {"blue", PLY_UCHAR, PLY_UCHAR, offsetof(Vertex,blue), 0, 0, 0, 0},
};




static int nverts;
static Vertex **vlist;
static PlyOtherElems *other_elements = NULL;
static PlyOtherProp *vert_other;
static int nelems;
static char **elist;
static int num_comments;
static char **comments;
static int num_obj_info;
static char **obj_info;
static int file_type;

int has_z, has_red, has_green, has_blue;




main(int argc, char *argv[])
{
    get_options(argc, argv);    
    read_file();
    curveshade();
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
  fprintf(stderr, "usage: %s [flags]  <in.ply   >out.ascarch\n", progname);
}


curveshade()
{
    int i;

    Vertex *vert;
    float   min, max, delta, intensity;
    float   min_intensity = 25.0;
    float   max_intensity = 255.0;
    float   scale_delta;
    int     int_intensity;
    
    
    for (i=0; i<nverts; i++)
    {
	vert = vlist[i];

	vert->red = 255*(1.0 - vert->curvature);
	vert->green = 0.0;
	vert->blue = 255*vert->curvature;
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

      
      
      has_z = has_red = has_green = has_blue = FALSE;
      
      for (j=0; j<nprops; j++)
      {
	  if (equal_strings("z", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[0]);  
	      has_z = TRUE;
	  }
	  else if (equal_strings("red", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[1]);  
	      has_red = TRUE;
	  }
	  else if (equal_strings("green", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[2]);  
	      has_green = TRUE;
	  }
	  else if (equal_strings("blue", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[3]);  
	      has_blue = TRUE;
	  }
      }
      vert_other = ply_get_other_properties (ply, elem_name,
					     offsetof(Vertex,other_props));

      
      if (!has_z)
      {
	  fprintf(stderr, "Vertices don't have z\n");
	  exit(-1);
      }
      if ((has_red) || (has_green) || (has_blue))
      {
	  fprintf(stderr, "Vertices already have colors\n");
	  exit(-1);
      }
      
      
      
      for (j = 0; j < num_elems; j++) {
        vlist[j] = (Vertex *) malloc (sizeof (Vertex));
        ply_get_element (ply, (void *) vlist[j]);
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
  static char *known_elements[] = {"vertex"};
  
  


  ply = ply_write (stdout, 1, known_elements, file_type);


  

  ply_element_count (ply, "vertex", nverts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_property (ply, "vertex", &vert_props[3]);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));

  ply_describe_other_elements (ply, other_elements);

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);

  ply_header_complete (ply);

  
  ply_put_element_setup (ply, "vertex");
  for (i = 0; i < nverts; i++)
    ply_put_element (ply, (void *) vlist[i]);

  ply_put_other_elements (ply);

  
  ply_close (ply);
}

