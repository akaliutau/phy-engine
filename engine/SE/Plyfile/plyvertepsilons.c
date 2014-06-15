





#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <ply.h>
#include <values.h>



#define LO      0
#define HI       1

#define X         0
#define Y         1
#define Z         2

#define TRUE  1
#define FALSE 0


#define SQ_DIST3(a, b)          (((a)[0]-(b)[0])*((a)[0]-(b)[0]) +      \
                                 ((a)[1]-(b)[1])*((a)[1]-(b)[1]) +      \
                                 ((a)[2]-(b)[2])*((a)[2]-(b)[2]))

#define DOTPROD3(a, b)		 ((a)[0]*(b)[0] + (a)[1]*(b)[1] + (a)[2]*(b)[2])

#define MAX(x,y) ((x)>(y) ? (x) : (y))
#define MIN(x,y) ((x)<(y) ? (x) : (y))





typedef struct Vertex {
  double  coord[3];            
  float epsilon;
  void *other_props;       
} Vertex;





void read_file();
void write_file();
void compute_bbox_diagonal();
void compute_vertex_epsilons();



PlyProperty vert_props[] = { 
  {"x", PLY_FLOAT, PLY_DOUBLE, offsetof(Vertex,coord[X]), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_DOUBLE, offsetof(Vertex,coord[Y]), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_DOUBLE, offsetof(Vertex,coord[Z]), 0, 0, 0, 0},
  {"epsilon", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,epsilon), 0, 0, 0, 0},
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

static int has_x, has_y, has_z;
static int has_epsilons;

static double bbox_diagonal;

static double gradient_direction[3] = {1.0, 0.0, 0.0};

static double min_epsilon = 1.0/64.0;
static double max_epsilon = 1.0;










int main(int argc, char *argv[])
{
    read_file();
    compute_bbox_diagonal();
    compute_vertex_epsilons();    
    write_file();
    return 0;
}





void compute_bbox_diagonal()
{
    int       i, j, k;
    Vertex   *vert;
    double  bbox[2][3];
    
    bbox[HI][X] = bbox[HI][Y] = bbox[HI][Z] = -MAXDOUBLE;
    bbox[LO][X] = bbox[LO][Y] = bbox[LO][Z] = MAXDOUBLE;
    for (i=0; i<nverts; i++)
    {
        vert = vlist[i];
        for (j=0; j<3; j++)
	    {
		bbox[HI][j] = MAX(bbox[HI][j], vert->coord[j]);
		bbox[LO][j] = MIN(bbox[LO][j], vert->coord[j]);
	    }
    }

    bbox_diagonal = sqrt(SQ_DIST3(bbox[HI], bbox[LO]));
    
    return;
} 


void compute_vertex_epsilons()
{

    int i;
    double min_distance, max_distance, distance;
    double range, t, logscale, logscale_t;

    
    for (i=0, min_distance = MAXDOUBLE, max_distance = -MAXDOUBLE; 
            i<nverts; i++)
    {
        distance = DOTPROD3(vlist[i]->coord, gradient_direction);
        min_distance = MIN(distance, min_distance);
        max_distance = MAX(distance, max_distance);
    }


    range = max_distance - min_distance;
    logscale = log(max_epsilon/min_epsilon)/log(2.0);
    
    for (i=0; i<nverts; i++)
    {
        distance = DOTPROD3(vlist[i]->coord, gradient_direction);
        t = (distance - min_distance)/range;
        logscale_t = t*logscale;
        vlist[i]->epsilon = min_epsilon * pow(2.0, logscale_t) * bbox_diagonal * 0.01;
    }
}




void read_file()
{
  int i,j;
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

      
      
      has_x = has_y = has_z = has_epsilons = FALSE;
      
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
	  else if (equal_strings("epsilon", plist[j]->name))
	  {
	      ply_get_property (ply, elem_name, &vert_props[3]);
	      has_epsilons = TRUE;
	  }
      }
      vert_other = ply_get_other_properties (ply, elem_name,
					     offsetof(Vertex,other_props));

      
      if ((!has_x) || (!has_y) || (!has_z))
      {
	  fprintf(stderr, "Vertices must have x, y, and z coordinates\n");
	  exit(-1);
      }
      if (has_epsilons)
      {
              fprintf(stderr, "Vertices already have epsilons\n");
              exit(-1);
       }

      
      
      for (j = 0; j < num_elems; j++) {
        vlist[j] = (Vertex *) calloc (sizeof (Vertex), 1);
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




void write_file()
{
  int i,j;
  PlyFile *ply;
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


