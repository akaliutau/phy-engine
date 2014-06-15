

#include <stdio.h>
#include <math.h>
#include <ply.h>




int main(int argc, char *argv[])
{
  int i,j,k;
  PlyFile *in_ply;
  PlyFile *out_ply;
  int nelems;
  char **elist;
  int nprops;
  int num_elems;
  PlyProperty **plist;
  PlyProperty **plist_copy;
  PlyProperty *prop;
  char *elem_name;
  int num_comments;
  char **comments;
  int num_obj_info;
  char **obj_info;
  char *data;
  int offset;
  int *offset_list;
  int **lists;
  int *list_count;
  char **list_ptr;
  int verbose_flag = 0;

#ifndef WRITE_ASCII
#ifndef WRITE_BINARY

  fprintf (stderr, "'%s' compiled incorrectly.\n", argv[0]);
  fprintf (stderr,
           "Must have WRITE_ASCII or WRITE_BINARY defined during compile.\n");
  exit (-1);

#endif
#endif

  

#ifdef WRITE_ASCII
  if (argc > 2 || (argc == 2 && !equal_strings (argv[1], "-p"))) {
    fprintf (stderr, "usage: %s [flags] <infile >outfile\n", argv[0]);
    fprintf (stderr, "          -p (print element labels)\n");
    exit (0);
  }
#endif

#ifdef WRITE_BINARY
  if (argc > 1) {
    fprintf (stderr, "usage: %s <infile >outfile\n", argv[0]);
    exit (0);
  }
#endif

  if (argc == 2 && equal_strings (argv[1], "-p"))
    verbose_flag = 1;

  

  in_ply  = ply_read  (stdin,  &nelems, &elist);

#ifdef WRITE_ASCII
  out_ply = ply_write (stdout, nelems, elist, PLY_ASCII);
#endif

#ifdef WRITE_BINARY
  out_ply = ply_write (stdout, nelems, elist, PLY_BINARY_NATIVE);
#endif

  

  plist_copy = (PlyProperty **) malloc (sizeof (PlyProperty *) * nelems);
  offset_list = (int *) malloc (sizeof (int) * nelems);
  lists = (int **) malloc (sizeof (int *) * nelems);
  list_count = (int *) malloc (sizeof (int));

  
  

  for (i = 0; i < nelems; i++) {

    
    elem_name = elist[i];
    plist = ply_get_element_description(in_ply, elem_name, &num_elems, &nprops);

    
    

    list_count[i] = 0;
    lists[i] = (int *) malloc (sizeof (int) * nprops);

    

    offset = 0;
    for (j = 0; j < nprops; j++) {
      plist[j]->offset = offset;
      offset += 8;
      if (plist[j]->is_list) {
        plist[j]->count_offset = offset;
        lists[i][list_count[i]] = offset - 8;
        list_count[i]++;
        offset += 8;
      }
    }

    offset_list[i] = offset;

    

    plist_copy[i] = (PlyProperty *) malloc (sizeof (PlyProperty) * nprops);
    prop = plist_copy[i];

    for (j = 0; j < nprops; j++) {
      prop->name = plist[j]->name;
      prop->external_type = plist[j]->external_type;
      prop->internal_type = plist[j]->external_type;
      prop->offset = plist[j]->offset;
      prop->is_list = plist[j]->is_list;
      prop->count_external = plist[j]->count_external;
      prop->count_internal = plist[j]->count_external;
      prop->count_offset = plist[j]->count_offset;
      prop++;
    }

    ply_describe_element (out_ply, elem_name, num_elems, nprops, plist_copy[i]);
  }

  

  comments = ply_get_comments (in_ply, &num_comments);
  for (i = 0; i < num_comments; i++)
    ply_put_comment (out_ply, comments[i]);

  obj_info = ply_get_obj_info (in_ply, &num_obj_info);
  for (i = 0; i < num_obj_info; i++)
    ply_put_obj_info (out_ply, obj_info[i]);

  
  ply_header_complete (out_ply);

  

  for (i = 0; i < nelems; i++) {

    
    elem_name = elist[i];
    plist = ply_get_element_description(in_ply, elem_name, &num_elems, &nprops);

    
    data = (char *) malloc (8 * offset_list[i]);

    
    ply_get_element_setup (in_ply, elem_name, nprops, plist_copy[i]);
    ply_put_element_setup (out_ply, elem_name);

    
    if (verbose_flag)
      fprintf (out_ply->fp, "%s:\n", elem_name);

    

    if (list_count[i]) {
      
      for (j = 0; j < num_elems; j++) {
        ply_get_element (in_ply, (void *) data);
        ply_put_element (out_ply, (void *) data);
        for (k = 0; k < list_count[i]; k++) {
          list_ptr = (char **) (data + lists[i][k]);
          free (*list_ptr);
        }
      }
    }
    else {
      
      for (j = 0; j < num_elems; j++) {
        ply_get_element (in_ply, (void *) data);
        ply_put_element (out_ply, (void *) data);
      }
    }

  }

  

  ply_close (in_ply);
  ply_close (out_ply);

  return 0;
}

