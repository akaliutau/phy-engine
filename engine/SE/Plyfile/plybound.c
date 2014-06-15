

#include <stdio.h>
#include <math.h>
#include <ply.h>




typedef struct Vertex {
  float x,y,z;
  void *other_props;       
} Vertex;

char *elem_names[] = { 
  "vertex", "face"
};

PlyProperty vert_props[] = { 
  {"x", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,x), 0, 0, 0, 0},
  {"y", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,y), 0, 0, 0, 0},
  {"z", PLY_FLOAT, PLY_FLOAT, offsetof(Vertex,z), 0, 0, 0, 0},
};




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

static float xcenter = 0;
static float ycenter = 0;
static float zcenter = 0;

static float box_size = 2;

static int print_only = 0;  
static int use_mass = 0;    




main(int argc, char *argv[])
{
  int i,j;
  char *s;
  char *progname;

  progname = argv[0];

  while (--argc > 0 && (*++argv)[0]=='-') {
    for (s = argv[0]+1; *s; s++)
      switch (*s) {
        case 'm':
          use_mass = 1 - use_mass;
          break;
        case 'p':
          print_only = 1 - print_only;
          break;
        case 'b':
          box_size = atof (*++argv);
          argc -= 1;
          break;
        case 'c':
          xcenter = atof (*++argv);
          ycenter = atof (*++argv);
          zcenter = atof (*++argv);
          argc -= 3;
          break;
        default:
          usage (progname);
          exit (-1);
          break;
      }
  }

  read_file();

  transform();

  if (!print_only)
    write_file();
}




usage(char *progname)
{
  fprintf (stderr, "usage: %s [flags] <in.ply >out.ply\n", progname);
  fprintf (stderr, "       -b box_size (default = 2)\n");
  fprintf (stderr, "       -c xcenter ycenter zcenter (default is origin)\n");
  fprintf (stderr, "       -m (use mass center instead of geometric center)\n");
  fprintf (stderr, "       -p (only print information)\n");
}




transform()
{
  int i;
  Vertex *vert;
  float xmin,ymin,zmin;
  float xmax,ymax,zmax;
  float xdiff,ydiff,zdiff;
  float diff;
  float scale;
  float xt,yt,zt;
  float xc,yc,zc;
  float xsum,ysum,zsum;

  if (nverts == 0)
    return;

  vert = vlist[0];

  xmin = vert->x;
  ymin = vert->y;
  zmin = vert->z;

  xmax = vert->x;
  ymax = vert->y;
  zmax = vert->z;

  xsum = ysum = zsum = 0;

#define MIN(a,b) ((a < b) ? (a) : (b))
#define MAX(a,b) ((a > b) ? (a) : (b))

  

  for (i = 0; i < nverts; i++) {

    vert = vlist[i];

    xmin = MIN (vert->x, xmin);
    ymin = MIN (vert->y, ymin);
    zmin = MIN (vert->z, zmin);

    xmax = MAX (vert->x, xmax);
    ymax = MAX (vert->y, ymax);
    zmax = MAX (vert->z, zmax);

    xsum += vert->x;
    ysum += vert->y;
    zsum += vert->z;
  }

  xdiff = xmax - xmin;
  ydiff = ymax - ymin;
  zdiff = zmax - zmin;

  diff = MAX (xdiff, MAX (ydiff, zdiff));
  scale = box_size / diff;

  if (print_only) {
    fprintf (stderr, "\n");
    fprintf (stderr, "xmin xmax: %g %g\n", xmin, xmax);
    fprintf (stderr, "ymin ymax: %g %g\n", ymin, ymax);
    fprintf (stderr, "zmin zmax: %g %g\n", zmin, zmax);
    fprintf (stderr, "geometric center = %g %g %g\n",
             0.5 * (xmin + xmax), 0.5 * (ymin + ymax), 0.5 * (zmin + zmax));
    fprintf (stderr, "center of mass   = %g %g %g\n",
             xsum / nverts, ysum / nverts, zsum / nverts);
    fprintf (stderr, "maximum side length = %g\n", diff);
    fprintf (stderr, "\n");
    return;
  }

  

  if (use_mass) {
    xc = xsum / nverts;
    yc = ysum / nverts;
    zc = zsum / nverts;
  }
  else {
    xc = 0.5 * (xmin + xmax);
    yc = 0.5 * (ymin + ymax);
    zc = 0.5 * (zmin + zmax);
  }

  xt = xcenter - scale * xc;
  yt = ycenter - scale * yc;
  zt = zcenter - scale * zc;

  
  

  for (i = 0; i < nverts; i++) {
    vert = vlist[i];
    vert->x = xt + vert->x * scale;
    vert->y = yt + vert->y * scale;
    vert->z = zt + vert->z * scale;
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

  


  ply = ply_write (stdout, 1, elem_names, file_type);


  

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

