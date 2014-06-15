

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ply.h>

char *type_names[] = {
"invalid",
"char", "short", "int",
"uchar", "ushort", "uint",
"float", "double",
};

int ply_type_size[] = {
  0, 1, 2, 4, 1, 2, 4, 4, 8
};

typedef union
{
      int  int_value;
      char byte_values[sizeof(int)];
} endian_test_type;


static int native_binary_type = -1;
static int types_checked = 0;

#define NO_OTHER_PROPS  -1

#define DONT_STORE_PROP  0
#define STORE_PROP       1

#define OTHER_PROP       0
#define NAMED_PROP       1



int equal_strings(char *, char *);


PlyElement *find_element(PlyFile *, char *);


PlyProperty *find_property(PlyElement *, char *, int *);


void write_scalar_type (FILE *, int);


char **get_words(FILE *, int *, char **);
char **old_get_words(FILE *, int *);


void write_binary_item(FILE *, int, int, unsigned int, double, int);
void write_ascii_item(FILE *, int, unsigned int, double, int);
double old_write_ascii_item(FILE *, char *, int);


void add_element(PlyFile *, char **);
void add_property(PlyFile *, char **);
void add_comment(PlyFile *, char *);
void add_obj_info(PlyFile *, char *);


void copy_property(PlyProperty *, PlyProperty *);


void store_item(char *, int, int, unsigned int, double);


void get_stored_item( void *, int, int *, unsigned int *, double *);


double get_item_value(char *, int);


void get_ascii_item(char *, int, int *, unsigned int *, double *);
void get_binary_item(FILE *, int, int, int *, unsigned int *, double *);


void ascii_get_element(PlyFile *, char *);
void binary_get_element(PlyFile *, char *);


char *my_alloc(int, int, char *);


void get_native_binary_type();
void swap_bytes(char *, int);

void check_types();








PlyFile *ply_write(
  FILE *fp,
  int nelems,
  char **elem_names,
  int file_type
)
{
  int i;
  PlyFile *plyfile;
  PlyElement *elem;

  
  if (fp == NULL)
    return (NULL);

  if (native_binary_type == -1)
     get_native_binary_type();
  if (!types_checked)
     check_types();
  
  

  plyfile = (PlyFile *) myalloc (sizeof (PlyFile));
  if (file_type == PLY_BINARY_NATIVE)
     plyfile->file_type = native_binary_type;
  else
     plyfile->file_type = file_type;
  plyfile->num_comments = 0;
  plyfile->num_obj_info = 0;
  plyfile->nelems = nelems;
  plyfile->version = 1.0;
  plyfile->fp = fp;
  plyfile->other_elems = NULL;

  

  plyfile->elems = (PlyElement **) myalloc (sizeof (PlyElement *) * nelems);
  for (i = 0; i < nelems; i++) {
    elem = (PlyElement *) myalloc (sizeof (PlyElement));
    plyfile->elems[i] = elem;
    elem->name = strdup (elem_names[i]);
    elem->num = 0;
    elem->nprops = 0;
  }

  
  return (plyfile);
}




PlyFile *ply_open_for_writing(
  char *filename,
  int nelems,
  char **elem_names,
  int file_type,
  float *version
)
{
  PlyFile *plyfile;
  char *name;
  FILE *fp;

  

  name = (char *) myalloc (sizeof (char) * (strlen (filename) + 5));
  strcpy (name, filename);
  if (strlen (name) < 4 ||
      strcmp (name + strlen (name) - 4, ".ply") != 0)
      strcat (name, ".ply");

  

  fp = fopen (name, "w");
  if (fp == NULL) {
    return (NULL);
  }

  

  plyfile = ply_write (fp, nelems, elem_names, file_type);
  if (plyfile == NULL)
    return (NULL);

  
  *version = plyfile->version;

  
  return (plyfile);
}




void ply_describe_element(
  PlyFile *plyfile,
  char *elem_name,
  int nelems,
  int nprops,
  PlyProperty *prop_list
)
{
  int i;
  PlyElement *elem;
  PlyProperty *prop;

  
  elem = find_element (plyfile, elem_name);
  if (elem == NULL) {
    fprintf(stderr,"ply_describe_element: can't find element '%s'\n",elem_name);
    exit (-1);
  }

  elem->num = nelems;

  

  elem->nprops = nprops;
  elem->props = (PlyProperty **) myalloc (sizeof (PlyProperty *) * nprops);
  elem->store_prop = (char *) myalloc (sizeof (char) * nprops);

  for (i = 0; i < nprops; i++) {
    prop = (PlyProperty *) myalloc (sizeof (PlyProperty));
    elem->props[i] = prop;
    elem->store_prop[i] = NAMED_PROP;
    copy_property (prop, &prop_list[i]);
  }
}




void ply_describe_property(
  PlyFile *plyfile,
  char *elem_name,
  PlyProperty *prop
)
{
  PlyElement *elem;
  PlyProperty *elem_prop;

  
  elem = find_element (plyfile, elem_name);
  if (elem == NULL) {
    fprintf(stderr, "ply_describe_property: can't find element '%s'\n",
            elem_name);
    return;
  }

  

  if (elem->nprops == 0) {
    elem->props = (PlyProperty **) myalloc (sizeof (PlyProperty *));
    elem->store_prop = (char *) myalloc (sizeof (char));
    elem->nprops = 1;
  }
  else {
    elem->nprops++;
    elem->props = (PlyProperty **)
                  realloc (elem->props, sizeof (PlyProperty *) * elem->nprops);
    elem->store_prop = (char *)
                  realloc (elem->store_prop, sizeof (char) * elem->nprops);
  }

  

  elem_prop = (PlyProperty *) myalloc (sizeof (PlyProperty));
  elem->props[elem->nprops - 1] = elem_prop;
  elem->store_prop[elem->nprops - 1] = NAMED_PROP;
  copy_property (elem_prop, prop);
}




void ply_describe_other_properties(
  PlyFile *plyfile,
  PlyOtherProp *other,
  int offset
)
{
  int i;
  PlyElement *elem;
  PlyProperty *prop;

  
  elem = find_element (plyfile, other->name);
  if (elem == NULL) {
    fprintf(stderr, "ply_describe_other_properties: can't find element '%s'\n",
            other->name);
    return;
  }

  

  if (elem->nprops == 0) {
    elem->props = (PlyProperty **)
                  myalloc (sizeof (PlyProperty *) * other->nprops);
    elem->store_prop = (char *) myalloc (sizeof (char) * other->nprops);
    elem->nprops = 0;
  }
  else {
    int newsize;
    newsize = elem->nprops + other->nprops;
    elem->props = (PlyProperty **)
                  realloc (elem->props, sizeof (PlyProperty *) * newsize);
    elem->store_prop = (char *)
                  realloc (elem->store_prop, sizeof (char) * newsize);
  }

  

  for (i = 0; i < other->nprops; i++) {
    prop = (PlyProperty *) myalloc (sizeof (PlyProperty));
    copy_property (prop, other->props[i]);
    elem->props[elem->nprops] = prop;
    elem->store_prop[elem->nprops] = OTHER_PROP;
    elem->nprops++;
  }

  
  elem->other_size = other->size;
  elem->other_offset = offset;
}




void ply_element_count(
  PlyFile *plyfile,
  char *elem_name,
  int nelems
)
{
  PlyElement *elem;

  
  elem = find_element (plyfile, elem_name);
  if (elem == NULL) {
    fprintf(stderr,"ply_element_count: can't find element '%s'\n",elem_name);
    exit (-1);
  }

  elem->num = nelems;
}




void ply_header_complete(PlyFile *plyfile)
{
  int i,j;
  FILE *fp = plyfile->fp;
  PlyElement *elem;
  PlyProperty *prop;

  fprintf (fp, "ply\n");

  switch (plyfile->file_type) {
    case PLY_ASCII:
      fprintf (fp, "format ascii 1.0\n");
      break;
    case PLY_BINARY_BE:
      fprintf (fp, "format binary_big_endian 1.0\n");
      break;
    case PLY_BINARY_LE:
      fprintf (fp, "format binary_little_endian 1.0\n");
      break;
    default:
      fprintf (stderr, "ply_header_complete: bad file type = %d\n",
               plyfile->file_type);
      exit (-1);
  }

  

  for (i = 0; i < plyfile->num_comments; i++)
    fprintf (fp, "comment %s\n", plyfile->comments[i]);

  

  for (i = 0; i < plyfile->num_obj_info; i++)
    fprintf (fp, "obj_info %s\n", plyfile->obj_info[i]);

  

  for (i = 0; i < plyfile->nelems; i++) {

    elem = plyfile->elems[i];
    fprintf (fp, "element %s %d\n", elem->name, elem->num);

    
    for (j = 0; j < elem->nprops; j++) {
      prop = elem->props[j];
      if (prop->is_list) {
        fprintf (fp, "property list ");
        write_scalar_type (fp, prop->count_external);
        fprintf (fp, " ");
        write_scalar_type (fp, prop->external_type);
        fprintf (fp, " %s\n", prop->name);
      }
      else {
        fprintf (fp, "property ");
        write_scalar_type (fp, prop->external_type);
        fprintf (fp, " %s\n", prop->name);
      }
    }
  }

  fprintf (fp, "end_header\n");
}




void ply_put_element_setup(PlyFile *plyfile, char *elem_name)
{
  PlyElement *elem;

  elem = find_element (plyfile, elem_name);
  if (elem == NULL) {
    fprintf(stderr, "ply_elements_setup: can't find element '%s'\n", elem_name);
    exit (-1);
  }

  plyfile->which_elem = elem;
}




void ply_put_element(PlyFile *plyfile, void *elem_ptr)
{
  int j,k;
  FILE *fp = plyfile->fp;
  PlyElement *elem;
  PlyProperty *prop;
  char *elem_data,*item;
  char **item_ptr;
  int list_count;
  int item_size;
  int int_val;
  unsigned int uint_val;
  double double_val;
  char **other_ptr;

  elem = plyfile->which_elem;
  elem_data = (char *)elem_ptr;
  other_ptr = (char **) (((char *) elem_ptr) + elem->other_offset);

  

  if (plyfile->file_type == PLY_ASCII) {

    

    
    for (j = 0; j < elem->nprops; j++) {
      prop = elem->props[j];
      if (elem->store_prop[j] == OTHER_PROP)
        elem_data = *other_ptr;
      else
        elem_data = (char *)elem_ptr;
      if (prop->is_list) {
        item = elem_data + prop->count_offset;
        get_stored_item ((void *) item, prop->count_internal,
                         &int_val, &uint_val, &double_val);
        write_ascii_item (fp, int_val, uint_val, double_val,
                          prop->count_external);
        list_count = uint_val;
        item_ptr = (char **) (elem_data + prop->offset);
        item = item_ptr[0];
        item_size = ply_type_size[prop->internal_type];
        for (k = 0; k < list_count; k++) {
          get_stored_item ((void *) item, prop->internal_type,
                           &int_val, &uint_val, &double_val);
          write_ascii_item (fp, int_val, uint_val, double_val,
                            prop->external_type);
          item += item_size;
        }
      }
      else {
        item = elem_data + prop->offset;
        get_stored_item ((void *) item, prop->internal_type,
                         &int_val, &uint_val, &double_val);
        write_ascii_item (fp, int_val, uint_val, double_val,
                          prop->external_type);
      }
    }

    fprintf (fp, "\n");
  }
  else {

    

    
    for (j = 0; j < elem->nprops; j++) {
      prop = elem->props[j];
      if (elem->store_prop[j] == OTHER_PROP)
        elem_data = *other_ptr;
      else
        elem_data = (char *)elem_ptr;
      if (prop->is_list) {
        item = elem_data + prop->count_offset;
        item_size = ply_type_size[prop->count_internal];
        get_stored_item ((void *) item, prop->count_internal,
                         &int_val, &uint_val, &double_val);
        write_binary_item (fp, plyfile->file_type, int_val, uint_val,
			   double_val, prop->count_external);
        list_count = uint_val;
        item_ptr = (char **) (elem_data + prop->offset);
        item = item_ptr[0];
        item_size = ply_type_size[prop->internal_type];
        for (k = 0; k < list_count; k++) {
          get_stored_item ((void *) item, prop->internal_type,
                           &int_val, &uint_val, &double_val);
          write_binary_item (fp, plyfile->file_type, int_val, uint_val,
			     double_val, prop->external_type);
          item += item_size;
        }
      }
      else {
        item = elem_data + prop->offset;
        item_size = ply_type_size[prop->internal_type];
        get_stored_item ((void *) item, prop->internal_type,
                         &int_val, &uint_val, &double_val);
        write_binary_item (fp, plyfile->file_type, int_val, uint_val,
			   double_val, prop->external_type);
      }
    }

  }
}




void ply_put_comment(PlyFile *plyfile, char *comment)
{
  
  if (plyfile->num_comments == 0)
    plyfile->comments = (char **) myalloc (sizeof (char *));
  else
    plyfile->comments = (char **) realloc (plyfile->comments,
                         sizeof (char *) * (plyfile->num_comments + 1));

  
  plyfile->comments[plyfile->num_comments] = strdup (comment);
  plyfile->num_comments++;
}




void ply_put_obj_info(PlyFile *plyfile, char *obj_info)
{
  
  if (plyfile->num_obj_info == 0)
    plyfile->obj_info = (char **) myalloc (sizeof (char *));
  else
    plyfile->obj_info = (char **) realloc (plyfile->obj_info,
                         sizeof (char *) * (plyfile->num_obj_info + 1));

  
  plyfile->obj_info[plyfile->num_obj_info] = strdup (obj_info);
  plyfile->num_obj_info++;
}















PlyFile *ply_read(FILE *fp, int *nelems, char ***elem_names)
{
  int i,j;
  PlyFile *plyfile;
  int nwords;
  char **words;
  char **elist;
  PlyElement *elem;
  char *orig_line;

  
  if (fp == NULL)
    return (NULL);

  if (native_binary_type == -1)
     get_native_binary_type();
  if (!types_checked)
     check_types();
  
  

  plyfile = (PlyFile *) myalloc (sizeof (PlyFile));
  plyfile->nelems = 0;
  plyfile->comments = NULL;
  plyfile->num_comments = 0;
  plyfile->obj_info = NULL;
  plyfile->num_obj_info = 0;
  plyfile->fp = fp;
  plyfile->other_elems = NULL;

  

  words = get_words (plyfile->fp, &nwords, &orig_line);
  if (!words || !equal_strings (words[0], "ply"))
  {
      if (words)
	 free(words);
      return (NULL);
  }
  
  while (words) {

    

    if (equal_strings (words[0], "format")) {
      if (nwords != 3) {
	free(words);
	return (NULL);
      }
      if (equal_strings (words[1], "ascii"))
        plyfile->file_type = PLY_ASCII;
      else if (equal_strings (words[1], "binary_big_endian"))
        plyfile->file_type = PLY_BINARY_BE;
      else if (equal_strings (words[1], "binary_little_endian"))
        plyfile->file_type = PLY_BINARY_LE;
      else {
	free(words);
        return (NULL);
      }
      plyfile->version = atof (words[2]);
    }
    else if (equal_strings (words[0], "element"))
      add_element (plyfile, words);
    else if (equal_strings (words[0], "property"))
      add_property (plyfile, words);
    else if (equal_strings (words[0], "comment"))
      add_comment (plyfile, orig_line);
    else if (equal_strings (words[0], "obj_info"))
      add_obj_info (plyfile, orig_line);
    else if (equal_strings (words[0], "end_header")) {
      free(words);
      break;
    }
    
    
    free (words);

    words = get_words (plyfile->fp, &nwords, &orig_line);
  }

  
  

  for (i = 0; i < plyfile->nelems; i++) {
    elem = plyfile->elems[i];
    elem->store_prop = (char *) myalloc (sizeof (char) * elem->nprops);
    for (j = 0; j < elem->nprops; j++)
      elem->store_prop[j] = DONT_STORE_PROP;
    elem->other_offset = NO_OTHER_PROPS; 
  }

  

  elist = (char **) myalloc (sizeof (char *) * plyfile->nelems);
  for (i = 0; i < plyfile->nelems; i++)
    elist[i] = strdup (plyfile->elems[i]->name);

  *elem_names = elist;
  *nelems = plyfile->nelems;

  

  return (plyfile);
}




PlyFile *ply_open_for_reading(
  char *filename,
  int *nelems,
  char ***elem_names,
  int *file_type,
  float *version
)
{
  FILE *fp;
  PlyFile *plyfile;
  char *name;

  

  name = (char *) myalloc (sizeof (char) * (strlen (filename) + 5));
  strcpy (name, filename);
  if (strlen (name) < 4 ||
      strcmp (name + strlen (name) - 4, ".ply") != 0)
      strcat (name, ".ply");

  

  fp = fopen (name, "r");
  if (fp == NULL)
    return (NULL);

  

  plyfile = ply_read (fp, nelems, elem_names);

  

  *file_type = plyfile->file_type;
  *version = plyfile->version;

  

  return (plyfile);
}




PlyProperty **ply_get_element_description(
  PlyFile *plyfile,
  char *elem_name,
  int *nelems,
  int *nprops
)
{
  int i;
  PlyElement *elem;
  PlyProperty *prop;
  PlyProperty **prop_list;

  
  elem = find_element (plyfile, elem_name);
  if (elem == NULL)
    return (NULL);

  *nelems = elem->num;
  *nprops = elem->nprops;

  
  prop_list = (PlyProperty **) myalloc (sizeof (PlyProperty *) * elem->nprops);
  for (i = 0; i < elem->nprops; i++) {
    prop = (PlyProperty *) myalloc (sizeof (PlyProperty));
    copy_property (prop, elem->props[i]);
    prop_list[i] = prop;
  }

  
  return (prop_list);
}




void ply_get_element_setup(
  PlyFile *plyfile,
  char *elem_name,
  int nprops,
  PlyProperty *prop_list
)
{
  int i;
  PlyElement *elem;
  PlyProperty *prop;
  int index;

  
  elem = find_element (plyfile, elem_name);
  plyfile->which_elem = elem;

  
  for (i = 0; i < nprops; i++) {

    
    prop = find_property (elem, prop_list[i].name, &index);
    if (prop == NULL) {
      fprintf (stderr, "Warning:  Can't find property '%s' in element '%s'\n",
               prop_list[i].name, elem_name);
      continue;
    }

    
    prop->internal_type = prop_list[i].internal_type;
    prop->offset = prop_list[i].offset;
    prop->count_internal = prop_list[i].count_internal;
    prop->count_offset = prop_list[i].count_offset;

    
    elem->store_prop[index] = STORE_PROP;
  }
}




void ply_get_property(
  PlyFile *plyfile,
  char *elem_name,
  PlyProperty *prop
)
{
  PlyElement *elem;
  PlyProperty *prop_ptr;
  int index;

  
  elem = find_element (plyfile, elem_name);
  plyfile->which_elem = elem;

  

  prop_ptr = find_property (elem, prop->name, &index);
  if (prop_ptr == NULL) {
    fprintf (stderr, "Warning:  Can't find property '%s' in element '%s'\n",
             prop->name, elem_name);
    return;
  }
  prop_ptr->internal_type  = prop->internal_type;
  prop_ptr->offset         = prop->offset;
  prop_ptr->count_internal = prop->count_internal;
  prop_ptr->count_offset   = prop->count_offset;

  
  elem->store_prop[index] = STORE_PROP;
}




void ply_get_element(PlyFile *plyfile, void *elem_ptr)
{
  if (plyfile->file_type == PLY_ASCII)
    ascii_get_element (plyfile, (char *) elem_ptr);
  else
    binary_get_element (plyfile, (char *) elem_ptr);
}




char **ply_get_comments(PlyFile *plyfile, int *num_comments)
{
  *num_comments = plyfile->num_comments;
  return (plyfile->comments);
}




char **ply_get_obj_info(PlyFile *plyfile, int *num_obj_info)
{
  *num_obj_info = plyfile->num_obj_info;
  return (plyfile->obj_info);
}




void setup_other_props(PlyElement *elem)
{
  int i;
  PlyProperty *prop;
  int size = 0;
  int type_size;

  
  
  

  for (type_size = 8; type_size > 0; type_size /= 2) {

    
    

    for (i = 0; i < elem->nprops; i++) {

      
      if (elem->store_prop[i])
        continue;

      prop = elem->props[i];

      
      prop->internal_type = prop->external_type;
      prop->count_internal = prop->count_external;

      
      if (prop->is_list) {

        
        if (type_size == sizeof (void *)) {
          prop->offset = size;
          size += sizeof (void *);    
        }

        
        if (type_size == ply_type_size[prop->count_external]) {
          prop->count_offset = size;
          size += ply_type_size[prop->count_external];
        }
      }
      
      else if (type_size == ply_type_size[prop->external_type]) {
        prop->offset = size;
        size += ply_type_size[prop->external_type];
      }
    }

  }

  
  elem->other_size = size;
}




PlyOtherProp *ply_get_other_properties(
  PlyFile *plyfile,
  char *elem_name,
  int offset
)
{
  int i;
  PlyElement *elem;
  PlyOtherProp *other;
  PlyProperty *prop;
  int nprops;

  
  elem = find_element (plyfile, elem_name);
  if (elem == NULL) {
    fprintf (stderr, "ply_get_other_properties: Can't find element '%s'\n",
             elem_name);
    return (NULL);
  }

  
  plyfile->which_elem = elem;

  
  elem->other_offset = offset;

  
  setup_other_props (elem);

  
  other = (PlyOtherProp *) myalloc (sizeof (PlyOtherProp));
  other->name = strdup (elem_name);
#if 0
  if (elem->other_offset == NO_OTHER_PROPS) {
    other->size = 0;
    other->props = NULL;
    other->nprops = 0;
    return (other);
  }
#endif
  other->size = elem->other_size;
  other->props = (PlyProperty **) myalloc (sizeof(PlyProperty) * elem->nprops);
  
  
  nprops = 0;
  for (i = 0; i < elem->nprops; i++) {
    if (elem->store_prop[i])
      continue;
    prop = (PlyProperty *) myalloc (sizeof (PlyProperty));
    copy_property (prop, elem->props[i]);
    other->props[nprops] = prop;
    nprops++;
  }
  other->nprops = nprops;

#if 1
  
  if (other->nprops == 0) {
    elem->other_offset = NO_OTHER_PROPS;
  }
#endif
  
  
  return (other);
}













PlyOtherElems *ply_get_other_element (
  PlyFile *plyfile,
  char *elem_name,
  int elem_count
)
{
  int i;
  PlyElement *elem;
  PlyOtherElems *other_elems;
  OtherElem *other;

  
  elem = find_element (plyfile, elem_name);
  if (elem == NULL) {
    fprintf (stderr,
             "ply_get_other_element: can't find element '%s'\n", elem_name);
    exit (-1);
  }

  
  

  if (plyfile->other_elems == NULL) {
    plyfile->other_elems = (PlyOtherElems *) myalloc (sizeof (PlyOtherElems));
    other_elems = plyfile->other_elems;
    other_elems->other_list = (OtherElem *) myalloc (sizeof (OtherElem));
    other = &(other_elems->other_list[0]);
    other_elems->num_elems = 1;
  }
  else {
    other_elems = plyfile->other_elems;
    other_elems->other_list = (OtherElem *) realloc (other_elems->other_list,
                              sizeof (OtherElem) * other_elems->num_elems + 1);
    other = &(other_elems->other_list[other_elems->num_elems]);
    other_elems->num_elems++;
  }

  
  other->elem_count = elem_count;

  
  other->elem_name = strdup (elem_name);

  
  other->other_data = (OtherData **)
                  malloc (sizeof (OtherData *) * other->elem_count);

  
  other->other_props = ply_get_other_properties (plyfile, elem_name,
                         offsetof(OtherData,other_props));

  
  for (i = 0; i < other->elem_count; i++) {
    
    other->other_data[i] = (OtherData *) malloc (sizeof (OtherData));
    ply_get_element (plyfile, (void *) other->other_data[i]);
  }

  
  return (other_elems);
}




void ply_describe_other_elements (
  PlyFile *plyfile,
  PlyOtherElems *other_elems
)
{
  int i;
  OtherElem *other;
  PlyElement *elem;
  
  
  if (other_elems == NULL)
    return;

  
  plyfile->other_elems = other_elems;

  
  
  
  REALLOCN(plyfile->elems, PlyElement *,
	   plyfile->nelems, plyfile->nelems + other_elems->num_elems);
  for (i = 0; i < other_elems->num_elems; i++) {
      other = &(other_elems->other_list[i]);
      elem = (PlyElement *) myalloc (sizeof (PlyElement));
      plyfile->elems[plyfile->nelems++] = elem;
      elem->name = strdup (other->elem_name);
      elem->num = other->elem_count;
      elem->nprops = 0;
      ply_describe_other_properties (plyfile, other->other_props,
				     offsetof(OtherData,other_props));
  }
}




void ply_put_other_elements (PlyFile *plyfile)
{
  int i,j;
  OtherElem *other;

  
  if (plyfile->other_elems == NULL)
    return;

  

  for (i = 0; i < plyfile->other_elems->num_elems; i++) {

    other = &(plyfile->other_elems->other_list[i]);
    ply_put_element_setup (plyfile, other->elem_name);

    
    for (j = 0; j < other->elem_count; j++)
      ply_put_element (plyfile, (void *) other->other_data[j]);
  }
}




void ply_free_other_elements (PlyOtherElems *other_elems)
{
  other_elems = other_elems;
}











void ply_close(PlyFile *plyfile)
{
  fclose (plyfile->fp);

  
  free (plyfile);
}




void ply_get_info(PlyFile *ply, float *version, int *file_type)
{
  if (ply == NULL)
    return;

  *version = ply->version;
  *file_type = ply->file_type;
}




int equal_strings(char *s1, char *s2)
{

  while (*s1 && *s2)
    if (*s1++ != *s2++)
      return (0);

  if (*s1 != *s2)
    return (0);
  else
    return (1);
}




PlyElement *find_element(PlyFile *plyfile, char *element)
{
  int i;

  for (i = 0; i < plyfile->nelems; i++)
    if (equal_strings (element, plyfile->elems[i]->name))
      return (plyfile->elems[i]);

  return (NULL);
}




PlyProperty *find_property(PlyElement *elem, char *prop_name, int *index)
{
  int i;

  for (i = 0; i < elem->nprops; i++)
    if (equal_strings (prop_name, elem->props[i]->name)) {
      *index = i;
      return (elem->props[i]);
    }

  *index = -1;
  return (NULL);
}




void ascii_get_element(PlyFile *plyfile, char *elem_ptr)
{
  int j,k;
  PlyElement *elem;
  PlyProperty *prop;
  char **words;
  int nwords;
  int which_word;
  char *elem_data,*item=NULL;
  char *item_ptr;
  int item_size;
  int int_val;
  unsigned int uint_val;
  double double_val;
  int list_count;
  int store_it;
  char **store_array;
  char *orig_line;
  char *other_data=NULL;
  int other_flag;

  
  elem = plyfile->which_elem;

  

  if (elem->other_offset != NO_OTHER_PROPS) {
    char **ptr;
    other_flag = 1;
    
    other_data = (char *) myalloc (elem->other_size);
    
    ptr = (char **) (elem_ptr + elem->other_offset);
    *ptr = other_data;
  }
  else
    other_flag = 0;

  

  words = get_words (plyfile->fp, &nwords, &orig_line);
  if (words == NULL) {
    fprintf (stderr, "ply_get_element: unexpected end of file\n");
    exit (-1);
  }

  which_word = 0;

  for (j = 0; j < elem->nprops; j++) {

    prop = elem->props[j];
    store_it = (elem->store_prop[j] | other_flag);

    
    if (elem->store_prop[j])
      elem_data = elem_ptr;
    else
      elem_data = other_data;

    if (prop->is_list) {       

      
      get_ascii_item (words[which_word++], prop->count_external,
                      &int_val, &uint_val, &double_val);
      if (store_it) {
        item = elem_data + prop->count_offset;
        store_item(item, prop->count_internal, int_val, uint_val, double_val);
      }

      
      list_count = int_val;
      item_size = ply_type_size[prop->internal_type];
      store_array = (char **) (elem_data + prop->offset);

      if (list_count == 0) {
        if (store_it)
          *store_array = NULL;
      }
      else {
        if (store_it) {
          item_ptr = (char *) myalloc (sizeof (char) * item_size * list_count);
          item = item_ptr;
          *store_array = item_ptr;
        }

        
        for (k = 0; k < list_count; k++) {
          get_ascii_item (words[which_word++], prop->external_type,
                          &int_val, &uint_val, &double_val);
          if (store_it) {
            store_item (item, prop->internal_type,
                        int_val, uint_val, double_val);
            item += item_size;
          }
        }
      }

    }
    else {                     
      get_ascii_item (words[which_word++], prop->external_type,
                      &int_val, &uint_val, &double_val);
      if (store_it) {
        item = elem_data + prop->offset;
        store_item (item, prop->internal_type, int_val, uint_val, double_val);
      }
    }

  }

  free (words);
}




void binary_get_element(PlyFile *plyfile, char *elem_ptr)
{
  int j,k;
  PlyElement *elem;
  PlyProperty *prop;
  FILE *fp = plyfile->fp;
  char *elem_data,*item=NULL;
  char *item_ptr;
  int item_size;
  int int_val;
  unsigned int uint_val;
  double double_val;
  int list_count;
  int store_it;
  char **store_array;
  char *other_data=NULL;
  int other_flag;

  
  elem = plyfile->which_elem;

  

  if (elem->other_offset != NO_OTHER_PROPS) {
    char **ptr;
    other_flag = 1;
    
    other_data = (char *) myalloc (elem->other_size);
    
    ptr = (char **) (elem_ptr + elem->other_offset);
    *ptr = other_data;
  }
  else
    other_flag = 0;

  

  for (j = 0; j < elem->nprops; j++) {

    prop = elem->props[j];
    store_it = (elem->store_prop[j] | other_flag);

    
    if (elem->store_prop[j])
      elem_data = elem_ptr;
    else
      elem_data = other_data;

    if (prop->is_list) {       

      
      get_binary_item (fp, plyfile->file_type, prop->count_external,
                      &int_val, &uint_val, &double_val);
      if (store_it) {
        item = elem_data + prop->count_offset;
        store_item(item, prop->count_internal, int_val, uint_val, double_val);
      }

      
      list_count = int_val;
      item_size = ply_type_size[prop->internal_type];
      store_array = (char **) (elem_data + prop->offset);
      if (list_count == 0) {
        if (store_it)
          *store_array = NULL;
      }
      else {
        if (store_it) {
          item_ptr = (char *) myalloc (sizeof (char) * item_size * list_count);
          item = item_ptr;
          *store_array = item_ptr;
        }

        
        for (k = 0; k < list_count; k++) {
          get_binary_item (fp, plyfile->file_type, prop->external_type,
                          &int_val, &uint_val, &double_val);
          if (store_it) {
            store_item (item, prop->internal_type,
                        int_val, uint_val, double_val);
            item += item_size;
          }
        }
      }

    }
    else {                     
      get_binary_item (fp, plyfile->file_type, prop->external_type,
                      &int_val, &uint_val, &double_val);
      if (store_it) {
        item = elem_data + prop->offset;
        store_item (item, prop->internal_type, int_val, uint_val, double_val);
      }
    }

  }
}




void write_scalar_type (FILE *fp, int code)
{
  

  if (code <= PLY_START_TYPE || code >= PLY_END_TYPE) {
    fprintf (stderr, "write_scalar_type: bad data code = %d\n", code);
    exit (-1);
  }

  

  fprintf (fp, "%s", type_names[code]);
}



void swap_bytes(char *bytes, int num_bytes)
{
    int i;
    char temp;
    
    for (i=0; i < num_bytes/2; i++)
    {
	temp = bytes[i];
	bytes[i] = bytes[(num_bytes-1)-i];
	bytes[(num_bytes-1)-i] = temp;
    }
}



void get_native_binary_type()
{
    endian_test_type test;

    test.int_value = 0;
    test.int_value = 1;
    if (test.byte_values[0] == 1)
       native_binary_type = PLY_BINARY_LE;
    else if (test.byte_values[sizeof(int)-1] == 1)
       native_binary_type = PLY_BINARY_BE;
    else
    {
	fprintf(stderr, "ply: Couldn't determine machine endianness.\n");
	fprintf(stderr, "ply: Exiting...\n");
	exit(1);
    }
}



void check_types()
{
    if ((ply_type_size[PLY_CHAR] != sizeof(char)) ||
	(ply_type_size[PLY_SHORT] != sizeof(short)) ||	
	(ply_type_size[PLY_INT] != sizeof(int)) ||	
	(ply_type_size[PLY_UCHAR] != sizeof(unsigned char)) ||	
	(ply_type_size[PLY_USHORT] != sizeof(unsigned short)) ||	
	(ply_type_size[PLY_UINT] != sizeof(unsigned int)) ||	
	(ply_type_size[PLY_FLOAT] != sizeof(float)) ||	
	(ply_type_size[PLY_DOUBLE] != sizeof(double)))
    {
	fprintf(stderr, "ply: Type sizes do not match built-in types\n");
	fprintf(stderr, "ply: Exiting...\n");
	exit(1);
    }
    
    types_checked = 1;
}



char **get_words(FILE *fp, int *nwords, char **orig_line)
{
#define BIG_STRING 4096
  static char str[BIG_STRING];
  static char str_copy[BIG_STRING];
  char **words;
  int max_words = 10;
  int num_words = 0;
  char *ptr,*ptr2;
  char *result;

  words = (char **) myalloc (sizeof (char *) * max_words);

  
  result = fgets (str, BIG_STRING, fp);
  if (result == NULL) {
    *nwords = 0;
    *orig_line = NULL;
    return (NULL);
  }

  
  
  

  str[BIG_STRING-2] = ' ';
  str[BIG_STRING-1] = '\0';

  for (ptr = str, ptr2 = str_copy; *ptr != '\0'; ptr++, ptr2++) {
    *ptr2 = *ptr;
    if (*ptr == '\t') {
      *ptr = ' ';
      *ptr2 = ' ';
    }
    else if (*ptr == '\n') {
      *ptr = ' ';
      *ptr2 = '\0';
      break;
    }
  }

  

  ptr = str;
  while (*ptr != '\0') {

    
    while (*ptr == ' ')
      ptr++;

    
    if (*ptr == '\0')
      break;

    
    if (num_words >= max_words) {
      max_words += 10;
      words = (char **) realloc (words, sizeof (char *) * max_words);
    }
    words[num_words++] = ptr;

    
    while (*ptr != ' ')
      ptr++;

    
    *ptr++ = '\0';
  }

  
  *nwords = num_words;
  *orig_line = str_copy;
  return (words);
}




double get_item_value(char *item, int type)
{
  unsigned char *puchar;
  char *pchar;
  short int *pshort;
  unsigned short int *pushort;
  int *pint;
  unsigned int *puint;
  float *pfloat;
  double *pdouble;
  int int_value;
  unsigned int uint_value;
  double double_value;

  switch (type) {
    case PLY_CHAR:
      pchar = (char *) item;
      int_value = *pchar;
      return ((double) int_value);
    case PLY_UCHAR:
      puchar = (unsigned char *) item;
      int_value = *puchar;
      return ((double) int_value);
    case PLY_SHORT:
      pshort = (short int *) item;
      int_value = *pshort;
      return ((double) int_value);
    case PLY_USHORT:
      pushort = (unsigned short int *) item;
      int_value = *pushort;
      return ((double) int_value);
    case PLY_INT:
      pint = (int *) item;
      int_value = *pint;
      return ((double) int_value);
    case PLY_UINT:
      puint = (unsigned int *) item;
      uint_value = *puint;
      return ((double) uint_value);
    case PLY_FLOAT:
      pfloat = (float *) item;
      double_value = *pfloat;
      return (double_value);
    case PLY_DOUBLE:
      pdouble = (double *) item;
      double_value = *pdouble;
      return (double_value);
    default:
      fprintf (stderr, "get_item_value: bad type = %d\n", type);
      exit (-1);
  }
}




void write_binary_item(
  FILE *fp,
  int file_type,
  int int_val,
  unsigned int uint_val,
  double double_val,
  int type
)
{
  unsigned char uchar_val;
  char char_val;
  unsigned short ushort_val;
  short short_val;
  float float_val;
  void  *value;
  
  switch (type) {
    case PLY_CHAR:
      char_val = int_val;
      value = &char_val;
      break;
    case PLY_SHORT:
      short_val = int_val;
      value = &short_val;
      break;
    case PLY_INT:
      value = &int_val;
      break;
    case PLY_UCHAR:
      uchar_val = uint_val;
      value = &uchar_val;
      break;
    case PLY_USHORT:
      ushort_val = uint_val;
      value = &ushort_val;
      break;
    case PLY_UINT:
      value = &uint_val;
      break;
    case PLY_FLOAT:
      float_val = double_val;
      value = &float_val;
      break;
    case PLY_DOUBLE:
      value = &double_val;
      break;
    default:
      fprintf (stderr, "write_binary_item: bad type = %d\n", type);
      exit (-1);
  }

  if ((file_type != native_binary_type) && (ply_type_size[type] > 1))
     swap_bytes((char *)value, ply_type_size[type]);
  
  fwrite (value, ply_type_size[type], 1, fp);
}




void write_ascii_item(
  FILE *fp,
  int int_val,
  unsigned int uint_val,
  double double_val,
  int type
)
{
  switch (type) {
    case PLY_CHAR:
    case PLY_SHORT:
    case PLY_INT:
      fprintf (fp, "%d ", int_val);
      break;
    case PLY_UCHAR:
    case PLY_USHORT:
    case PLY_UINT:
      fprintf (fp, "%u ", uint_val);
      break;
    case PLY_FLOAT:
    case PLY_DOUBLE:
      fprintf (fp, "%g ", double_val);
      break;
    default:
      fprintf (stderr, "write_ascii_item: bad type = %d\n", type);
      exit (-1);
  }
}




double old_write_ascii_item(FILE *fp, char *item, int type)
{
  unsigned char *puchar;
  char *pchar;
  short int *pshort;
  unsigned short int *pushort;
  int *pint;
  unsigned int *puint;
  float *pfloat;
  double *pdouble;
  int int_value;
  unsigned int uint_value;
  double double_value;

  switch (type) {
    case PLY_CHAR:
      pchar = (char *) item;
      int_value = *pchar;
      fprintf (fp, "%d ", int_value);
      return ((double) int_value);
    case PLY_UCHAR:
      puchar = (unsigned char *) item;
      int_value = *puchar;
      fprintf (fp, "%d ", int_value);
      return ((double) int_value);
    case PLY_SHORT:
      pshort = (short int *) item;
      int_value = *pshort;
      fprintf (fp, "%d ", int_value);
      return ((double) int_value);
    case PLY_USHORT:
      pushort = (unsigned short int *) item;
      int_value = *pushort;
      fprintf (fp, "%d ", int_value);
      return ((double) int_value);
    case PLY_INT:
      pint = (int *) item;
      int_value = *pint;
      fprintf (fp, "%d ", int_value);
      return ((double) int_value);
    case PLY_UINT:
      puint = (unsigned int *) item;
      uint_value = *puint;
      fprintf (fp, "%u ", uint_value);
      return ((double) uint_value);
    case PLY_FLOAT:
      pfloat = (float *) item;
      double_value = *pfloat;
      fprintf (fp, "%g ", double_value);
      return (double_value);
    case PLY_DOUBLE:
      pdouble = (double *) item;
      double_value = *pdouble;
      fprintf (fp, "%g ", double_value);
      return (double_value);
    default:
      fprintf (stderr, "old_write_ascii_item: bad type = %d\n", type);
      exit (-1);
  }
}




void get_stored_item(
  void *ptr,
  int type,
  int *int_val,
  unsigned int *uint_val,
  double *double_val
)
{
  switch (type) {
    case PLY_CHAR:
      *int_val = *((char *) ptr);
      *uint_val = *int_val;
      *double_val = *int_val;
      break;
    case PLY_UCHAR:
      *uint_val = *((unsigned char *) ptr);
      *int_val = *uint_val;
      *double_val = *uint_val;
      break;
    case PLY_SHORT:
      *int_val = *((short int *) ptr);
      *uint_val = *int_val;
      *double_val = *int_val;
      break;
    case PLY_USHORT:
      *uint_val = *((unsigned short int *) ptr);
      *int_val = *uint_val;
      *double_val = *uint_val;
      break;
    case PLY_INT:
      *int_val = *((int *) ptr);
      *uint_val = *int_val;
      *double_val = *int_val;
      break;
    case PLY_UINT:
      *uint_val = *((unsigned int *) ptr);
      *int_val = *uint_val;
      *double_val = *uint_val;
      break;
    case PLY_FLOAT:
      *double_val = *((float *) ptr);
      *int_val = (int) *double_val;
      *uint_val = (unsigned int) *double_val;
      break;
    case PLY_DOUBLE:
      *double_val = *((double *) ptr);
      *int_val = (int) *double_val;
      *uint_val = (unsigned int) *double_val;
      break;
    default:
      fprintf (stderr, "get_stored_item: bad type = %d\n", type);
      exit (-1);
  }
}




void get_binary_item(
  FILE *fp,
  int file_type,
  int type,
  int *int_val,
  unsigned int *uint_val,
  double *double_val
)
{
  char c[8];
  void *ptr;

  ptr = (void *) c;

  fread (ptr, ply_type_size[type], 1, fp);

  if ((file_type != native_binary_type) && (ply_type_size[type] > 1))
     swap_bytes((char *)ptr, ply_type_size[type]);

  switch (type) {
    case PLY_CHAR:
      *int_val = *((char *) ptr);
      *uint_val = *int_val;
      *double_val = *int_val;
      break;
    case PLY_UCHAR:
      *uint_val = *((unsigned char *) ptr);
      *int_val = *uint_val;
      *double_val = *uint_val;
      break;
    case PLY_SHORT:
      *int_val = *((short int *) ptr);
      *uint_val = *int_val;
      *double_val = *int_val;
      break;
    case PLY_USHORT:
      *uint_val = *((unsigned short int *) ptr);
      *int_val = *uint_val;
      *double_val = *uint_val;
      break;
    case PLY_INT:
      *int_val = *((int *) ptr);
      *uint_val = *int_val;
      *double_val = *int_val;
      break;
    case PLY_UINT:
      *uint_val = *((unsigned int *) ptr);
      *int_val = *uint_val;
      *double_val = *uint_val;
      break;
    case PLY_FLOAT:
      *double_val = *((float *) ptr);
      *int_val = (int) *double_val;
      *uint_val = (unsigned int) *double_val;
      break;
    case PLY_DOUBLE:
      *double_val = *((double *) ptr);
      *int_val = (int) *double_val;
      *uint_val = (unsigned int) *double_val;
      break;
    default:
      fprintf (stderr, "get_binary_item: bad type = %d\n", type);
      exit (-1);
  }
}




void get_ascii_item(
  char *word,
  int type,
  int *int_val,
  unsigned int *uint_val,
  double *double_val
)
{
  switch (type) {
    case PLY_CHAR:
    case PLY_UCHAR:
    case PLY_SHORT:
    case PLY_USHORT:
    case PLY_INT:
      *int_val = atoi (word);
      *uint_val = (unsigned int) *int_val;
      *double_val = (double) *int_val;
      break;

    case PLY_UINT:
      *uint_val = strtol (word, (char **) NULL, 10);
      *int_val = (int) *uint_val;
      *double_val = (double) *uint_val;
      break;

    case PLY_FLOAT:
    case PLY_DOUBLE:
      *double_val = atof (word);
      *int_val = (int) *double_val;
      *uint_val = (unsigned int) *double_val;
      break;

    default:
      fprintf (stderr, "get_ascii_item: bad type = %d\n", type);
      exit (-1);
  }
}




void store_item (
  char *item,
  int type,
  int int_val,
  unsigned int uint_val,
  double double_val
)
{
  unsigned char *puchar;
  short int *pshort;
  unsigned short int *pushort;
  int *pint;
  unsigned int *puint;
  float *pfloat;
  double *pdouble;

  switch (type) {
    case PLY_CHAR:
      *item = int_val;
      break;
    case PLY_UCHAR:
      puchar = (unsigned char *) item;
      *puchar = uint_val;
      break;
    case PLY_SHORT:
      pshort = (short *) item;
      *pshort = int_val;
      break;
    case PLY_USHORT:
      pushort = (unsigned short *) item;
      *pushort = uint_val;
      break;
    case PLY_INT:
      pint = (int *) item;
      *pint = int_val;
      break;
    case PLY_UINT:
      puint = (unsigned int *) item;
      *puint = uint_val;
      break;
    case PLY_FLOAT:
      pfloat = (float *) item;
      *pfloat = double_val;
      break;
    case PLY_DOUBLE:
      pdouble = (double *) item;
      *pdouble = double_val;
      break;
    default:
      fprintf (stderr, "store_item: bad type = %d\n", type);
      exit (-1);
  }
}




void add_element (PlyFile *plyfile, char **words)
{
  PlyElement *elem;

  
  elem = (PlyElement *) myalloc (sizeof (PlyElement));
  elem->name = strdup (words[1]);
  elem->num = atoi (words[2]);
  elem->nprops = 0;

  
  if (plyfile->nelems == 0)
    plyfile->elems = (PlyElement **) myalloc (sizeof (PlyElement *));
  else
    plyfile->elems = (PlyElement **) realloc (plyfile->elems,
                     sizeof (PlyElement *) * (plyfile->nelems + 1));

  
  plyfile->elems[plyfile->nelems] = elem;
  plyfile->nelems++;
}




int get_prop_type(char *type_name)
{
  int i;

  for (i = PLY_START_TYPE + 1; i < PLY_END_TYPE; i++)
    if (equal_strings (type_name, type_names[i]))
      return (i);

  
  return (0);
}




void add_property (PlyFile *plyfile, char **words)
{
  PlyProperty *prop;
  PlyElement *elem;

  

  prop = (PlyProperty *) myalloc (sizeof (PlyProperty));

  if (equal_strings (words[1], "list")) {       
    prop->count_external = get_prop_type (words[2]);
    prop->external_type = get_prop_type (words[3]);
    prop->name = strdup (words[4]);
    prop->is_list = 1;
  }
  else {                                        
    prop->external_type = get_prop_type (words[1]);
    prop->name = strdup (words[2]);
    prop->is_list = 0;
  }

  

  elem = plyfile->elems[plyfile->nelems - 1];

  if (elem->nprops == 0)
    elem->props = (PlyProperty **) myalloc (sizeof (PlyProperty *));
  else
    elem->props = (PlyProperty **) realloc (elem->props,
                  sizeof (PlyProperty *) * (elem->nprops + 1));

  elem->props[elem->nprops] = prop;
  elem->nprops++;
}




void add_comment (PlyFile *plyfile, char *line)
{
  int i;

  
  i = 7;
  while (line[i] == ' ' || line[i] == '\t')
    i++;

  ply_put_comment (plyfile, &line[i]);
}




void add_obj_info (PlyFile *plyfile, char *line)
{
  int i;

  
  i = 8;
  while (line[i] == ' ' || line[i] == '\t')
    i++;

  ply_put_obj_info (plyfile, &line[i]);
}




void copy_property(PlyProperty *dest, PlyProperty *src)
{
  dest->name = strdup (src->name);
  dest->external_type = src->external_type;
  dest->internal_type = src->internal_type;
  dest->offset = src->offset;

  dest->is_list = src->is_list;
  dest->count_external = src->count_external;
  dest->count_internal = src->count_internal;
  dest->count_offset = src->count_offset;
}




char *my_alloc(int size, int lnum, char *fname)
{
  char *ptr;

  ptr = (char *) malloc (size);

  if (ptr == 0) {
    fprintf(stderr, "Memory allocation bombed on line %d in %s\n", lnum, fname);
  }

  return (ptr);
}

