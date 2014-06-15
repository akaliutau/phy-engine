

#include <stdio.h>
#include <math.h>
#include <ply.h>




typedef struct Vertex {
  float x,y,z;
  void *other_props;       
} Vertex;

PlyProperty vert_props[] = { 
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
};


typedef double Matrix[4][4];



int nverts;
Vertex **vlist;
PlyOtherElems *other_elements = NULL;
PlyOtherProp *vert_other;
int nelems;
char **elist;
int num_comments;
char **comments;
int num_obj_info;
char **obj_info;
int file_type;

static float xtrans = 0;
static float ytrans = 0;
static float ztrans = 0;

static float xscale = 1;
static float yscale = 1;
static float zscale = 1;

static char   *rotmode;
static double *rotations;

static char *progname;

void build_identity_matrix(Matrix mat);
void copy_matrix(Matrix destination, Matrix source);
void matrix_multiply(Matrix product, Matrix left, Matrix right);
void apply_matrix(Matrix mat, Vertex *vert);
void build_rotation_matrix(Matrix mat, char axis, double degrees);
void build_total_rotation_matris(Matrix rot);
void build_scale_matrix(Matrix scale);
void build_translate_matrix(Matrix trans);
void build_transformation(Matrix mat);

void transform();
void read_file();
void write_file();
void usage();



int main(int argc, char *argv[])
{
  int   i;
  char *s;

  progname = argv[0];

  while (--argc > 0 && (*++argv)[0]=='-') {
      switch (*(argv[0]+1)) {
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
	case 'r':
	  rotmode = *++argv;
	  for (i=0; i<strlen(rotmode); i++)
	     if ((rotmode[i] != 'x') &&
		 (rotmode[i] != 'X') &&
		 (rotmode[i] != 'y') &&
		 (rotmode[i] != 'Y') &&
		 (rotmode[i] != 'z') &&
		 (rotmode[i] != 'Z'))
	     {
		 fprintf(stderr, "Invalid rotation mode: %s\n",
			 rotmode);
		 usage();
	     }

	  if ((argc-2) < strlen(rotmode))
	  {
	      fprintf(stderr, "Degree list too short for rotation mode\n");
	      usage();
	  }
	  
	  ALLOCN(rotations, double, strlen(rotmode));
	  for (i=0; i<strlen(rotmode); i++)
	      if (sscanf(*++argv, "%lf", &(rotations[i])) != 1)
	      {
		  fprintf(stderr, "Invalid rotation degrees\n");
		  usage();
	      }
	  argc -= strlen(rotmode) + 1;
	  break;
	default:
	  usage();
	  break;
      }
  }
  if (argc != 0)
  {
      fprintf(stderr, "%d extra arguments on command line\n", argc);
      exit(1);
  }
  
  read_file();
  transform();
  write_file();
  return 0;
}

void usage()
{
    fprintf(stderr, "usage: %s [flags] <in.ply >out.ply\n", progname);
    fprintf(stderr, "       -t xtrans ytrans ztrans\n");
    fprintf(stderr, "       -s xscale yscale zscale\n");
    fprintf(stderr, "       -r <rotmode> <rotation degree list>\n\n");
    fprintf(stderr, "<rotmode> is any string of x's, y's and z's\n");
    fprintf(stderr, "  (e.g. xyz, zyx, xy, y, etc)\n\n");
    fprintf(stderr, "Rotations will be applied to the points in\n");
    fprintf(stderr, " the order they appear in the string.\n");
    fprintf(stderr, " (i.e. - if you think of the points as\n");
    fprintf(stderr, "  column vectors, the matrices will be\n");
    fprintf(stderr, "  multiplied in the reverse order of the\n");
    fprintf(stderr, "  string.\n\n");
    fprintf(stderr, "For each rotation in the string, there\n");
    fprintf(stderr, " be a number of degrees in the degree list.\n\n");
    fprintf(stderr, "Each vertex will first be scaled, then any\n");
    fprintf(stderr, " rotations will be applied, and finally it\n");
    fprintf(stderr, " be translated.\n");
    exit (1);
}



void build_identity_matrix(Matrix mat)
{
    int i,j;

    for (i=0; i<4; i++)
       for (j=0; j<4; j++)
       {
	   if (i == j)
	      mat[i][j] = 1.0;
	   else
	      mat[i][j] = 0.0;
       }
    return;
}

void copy_matrix(Matrix destination, Matrix source)
{
    int i,j;

    for (i=0; i<4; i++)
       for (j=0; j<4; j++)
	  destination[i][j] = source[i][j];
    return;
}

void matrix_multiply(Matrix product, Matrix left, Matrix right)
{
    int i,j;
    
    for (i=0; i<4; i++)
       for (j=0; j<4; j++)
	  product[i][j] =
	      left[i][0] * right[0][j] +
	      left[i][1] * right[1][j] +
	      left[i][2] * right[2][j] +
	      left[i][3] * right[3][j];
    return;
}

void apply_matrix(Matrix mat, Vertex *vert)
{
    int    row;
    double result[3];

    for (row=0; row<3; row++)
       result[row] =
	   mat[row][0]*vert->x +
	   mat[row][1]*vert->y +
	   mat[row][2]*vert->z +
	   mat[row][3];
    vert->x = result[0];
    vert->y = result[1];
    vert->z = result[2];
    return;
}

void build_rotation_matrix(Matrix mat, char axis, double degrees)
{
    double theta, sintheta, costheta;

    theta    = degrees * M_PI / 180.0;
    sintheta = sin(theta);
    costheta = cos(theta);

    build_identity_matrix(mat);

    
    switch(axis)
    {
     case 'x':
     case 'X':
         mat[1][1] =  costheta;
	 mat[1][2] = -sintheta;
	 mat[2][1] =  sintheta;
	 mat[2][2] =  costheta;
	 break;
    case 'y':
    case 'Y':
         mat[0][0] =  costheta;
	 mat[0][2] =  sintheta;
	 mat[2][0] = -sintheta;
	 mat[2][2] =  costheta;
	 break;
    case 'z':
    case 'Z':
         mat[0][0] =  costheta;
	 mat[0][1] = -sintheta;
	 mat[1][0] =  sintheta;
	 mat[1][1] =  costheta;
	 break;
    default:
         fprintf(stderr, "Invalid rotation axis\n");
	 exit(1);
    }
    return;
}


void build_total_rotation_matrix(Matrix rot)
{
    int    i;
    Matrix axisrot, accum;

    build_identity_matrix(rot);
    for (i=0; i<strlen(rotmode); i++)
    {
	build_rotation_matrix(axisrot, rotmode[i], rotations[i]);
	matrix_multiply(accum, axisrot, rot);
	copy_matrix(rot, accum);
    }
    
}

void build_scale_matrix(Matrix scale)
{
    build_identity_matrix(scale);
    scale[0][0] = xscale;
    scale[1][1] = yscale;
    scale[2][2] = zscale;
    return;
}


void build_translate_matrix(Matrix trans)
{
    build_identity_matrix(trans);
    trans[0][3] = xtrans;
    trans[1][3] = ytrans;
    trans[2][3] = ztrans;
}

void build_transformation(Matrix mat)
{
    Matrix scale;
    Matrix rot;
    Matrix trans;
    Matrix temp;
    
    build_scale_matrix(scale);
    build_total_rotation_matrix(rot);
    build_translate_matrix(trans);

    matrix_multiply(temp, rot, scale);
    matrix_multiply(mat, trans, temp);
    return;
}

void transform()
{
    int i;
    Vertex *vert;
    Matrix  transformation;
    
    build_transformation(transformation);
    
    for (i = 0; i < nverts; i++)
    {
	vert = vlist[i];
	apply_matrix(transformation, vert);
    }
    return;
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
    else
      other_elements = ply_get_other_element (ply, elem_name, num_elems);
  }

  comments = ply_get_comments (ply, &num_comments);
  obj_info = ply_get_obj_info (ply, &num_obj_info);

  ply_close (ply);
}




void write_file()
{
  int i;
  PlyFile *ply;
  char    *known_elements[] = {"vertex"};
  int      num_known_elements = 1;
  
  


  ply = ply_write (stdout, num_known_elements, known_elements, file_type);


  

  ply_element_count (ply, "vertex", nverts);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
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

