

#include <stdio.h>
#include <math.h>
#include <strings.h>
#include <ply.h>




typedef struct Vertex {
  float x,y,z;             
  int count;               
  int a,b,c;               
  int index;
  struct Vertex *shared;
  struct Vertex *next;
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

static float tolerance = 0.0001;   




#define PR1  17
#define PR2 101
#define TABLE_SIZE1  5003
#define TABLE_SIZE2 17003
#define TABLE_SIZE3 53003

typedef struct Hash_Table {	
  int npoints;			
  Vertex **verts;		
  int num_entries;		
  float scale;			
} Hash_Table;


Hash_Table *init_table(int, float);
void add_to_hash (Vertex *, Hash_Table *, float);





main(int argc, char *argv[])
{
  int i,j;
  char *s;
  char *progname;

  progname = argv[0];

  while (--argc > 0 && (*++argv)[0]=='-') {
    for (s = argv[0]+1; *s; s++)
      switch (*s) {
        case 'd':
          tolerance = atof (*++argv);
          argc -= 1;
          break;
        default:
          usage (progname);
          exit (-1);
          break;
      }
  }

  read_file();

  crunch_vertices();

  write_file();
}




usage(char *progname)
{
  fprintf (stderr, "usage: %s [flags] <in.ply >out.ply\n", progname);
  fprintf (stderr, "       -d distance (default = %g)\n", tolerance);
}




crunch_vertices()
{
  int i,j,k;
  int jj;
  Hash_Table *table;
  float squared_dist;
  Vertex *vert;
  Face *face;
  float recip;

  table = init_table (nverts, tolerance);

  squared_dist = tolerance * tolerance;

  
  

  for (i = 0; i < nverts; i++) {
    vlist[i]->count = 1;
    add_to_hash (vlist[i], table, squared_dist);
  }

  
  

  for (i = 0; i < nverts; i++) {
    vert = vlist[i];
    if (vert->shared == vert) {
      recip = 1.0 / vert->count;
      vert->x *= recip;
      vert->y *= recip;
      vert->z *= recip;
    }
  }

  

  for (i = 0; i < nfaces; i++) {

    face = flist[i];
    
    
    for (j = 0; j < face->nverts; j++)
      face->verts[j] = (int) (vlist[face->verts[j]]->shared);

    
    for (j = face->nverts-1; j >= 0; j--) {
      jj = (j+1) % face->nverts;
      if (face->verts[j] == face->verts[jj]) {
        for (k = j+1; k < face->nverts - 1; k++)
          face->verts[k] = face->verts[k+1];
        face->nverts--;
      }
    }

    
    

    if (face->nverts < 3)
      face->nverts = 0;

  }
}




void add_to_hash(Vertex *vert, Hash_Table *table, float sq_dist)
{
  int index;
  int a,b,c;
  int aa,bb,cc;
  float scale;
  Vertex *ptr;
  float dx,dy,dz;
  float sq;
  float min_dist;
  Vertex *min_ptr;

  

  scale = table->scale;
  a = floor (vert->x * scale);
  b = floor (vert->y * scale);
  c = floor (vert->z * scale);
  index = (a * PR1 + b * PR2 + c) % table->num_entries;
  if (index < 0)
    index += table->num_entries;

  
  

  for (ptr = table->verts[index]; ptr != NULL; ptr = ptr->next)
    if (a == ptr->a && b == ptr->b && c == ptr->c) {
      
      ptr->x += vert->x;
      ptr->y += vert->y;
      ptr->z += vert->z;
      ptr->count++;
      vert->shared = ptr;
      return;
    }

  

  vert->next = table->verts[index];
  table->verts[index] = vert;
  vert->shared = vert;  
  vert->a = a;
  vert->b = b;
  vert->c = c;
}




Hash_Table *init_table(int nverts, float size)
{
  int i;
  int index;
  int a,b,c;
  Hash_Table *table;
  float scale;

  

  table = (Hash_Table *) malloc (sizeof (Hash_Table));

  if (nverts < TABLE_SIZE1)
    table->num_entries = TABLE_SIZE1;
  else if (nverts < TABLE_SIZE2)
    table->num_entries = TABLE_SIZE2;
  else
    table->num_entries = TABLE_SIZE3;

  table->verts = (Vertex **) malloc (sizeof (Vertex *) * table->num_entries);

  
  for (i = 0; i < table->num_entries; i++)
    table->verts[i] = NULL;

  

  scale = 1 / size;
  table->scale = scale;

  return (table);
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
  int i,j,k;
  PlyFile *ply;
  int num_elems;
  char *elem_name;
  int vert_count;
  int face_count;

  


  ply = ply_write (stdout, 2, elem_names, file_type);

  

  vert_count = 0;
  for (i = 0; i < nverts; i++)
    if (vlist[i]->shared == vlist[i]) {
      vlist[i]->index = vert_count;
      vert_count++;
    }

  

  face_count = 0;
  for (i = 0; i < nfaces; i++)
    if (flist[i]->nverts != 0)
      face_count++;

  

  ply_element_count (ply, "vertex", vert_count);
  ply_describe_property (ply, "vertex", &vert_props[0]);
  ply_describe_property (ply, "vertex", &vert_props[1]);
  ply_describe_property (ply, "vertex", &vert_props[2]);
  ply_describe_other_properties (ply, vert_other, offsetof(Vertex,other_props));

  ply_element_count (ply, "face", face_count);
  ply_describe_property (ply, "face", &face_props[0]);
  ply_describe_other_properties (ply, face_other, offsetof(Face,other_props));

  ply_describe_other_elements (ply, other_elements);

  for (i = 0; i < num_comments; i++)
    ply_put_comment (ply, comments[i]);

  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (ply, obj_info[i]);

  ply_header_complete (ply);

  

  ply_put_element_setup (ply, "vertex");

  for (i = 0; i < nverts; i++)
    if (vlist[i]->shared == vlist[i])
      ply_put_element (ply, (void *) vlist[i]);

  

  ply_put_element_setup (ply, "face");

  for (i = 0; i < nfaces; i++) {
    if (flist[i]->nverts == 0)
      continue;
    for (j = 0; j < flist[i]->nverts; j++)
      flist[i]->verts[j] = ((Vertex *) flist[i]->verts[j])->index;
    ply_put_element (ply, (void *) flist[i]);
  }

  ply_put_other_elements (ply);

  
  ply_close (ply);
}

