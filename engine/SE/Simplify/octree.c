





#include <octree.h>
#include <geometry.h>
#include <ply.h>






#define MAX_RESOLUTION 64
#define MIN_MAX_NODE_OBJS 2






static int get_child_overlap(Octree *tree, OctreeNode *node, Extents bbox);
static void init_children(Octree *tree, OctreeNode *node);
static void ot_insert(Octree *tree, OctreeNode *node, OctreeData *data);
static void ot_remove(Octree *tree, OctreeNode *node, OctreeData *data);
static void destroy_node(OctreeNode *node);
static void range_query(Octree *tree, OctreeNode *node, Extents bbox,
			OctreeData **element_list, int *num_elements);
static OctreeNode *point_query(Octree *tree, OctreeNode *node, Point point);
static void print_leaf_stats(Octree *tree, OctreeNode *node, int *leaf_count,
			     int *element_count, int *maxout_count);









static int get_child_overlap(Octree *tree, OctreeNode *node, Extents bbox)
{
#if 0
    int overlap;

    overlap = 0;

    if (bbox[HI][X] > node->center[X] - tree->delta)
    {
	if (bbox[HI][Y] > node->center[Y] - tree->delta)
	{
	    if (bbox[HI][Z] > node->center[Z] - tree->delta)
		overlap |= 1 << HXHYHZ;
	    if (bbox[LO][Z] <= node->center[Z] + tree->delta)
		overlap |= 1 << HXHYLZ;
	    
	}
	if (bbox[LO][Y] <= node->center[Y] + tree->delta)
	{
	    if (bbox[HI][Z] > node->center[Z] - tree->delta)
		overlap |= 1 << HXLYHZ;
	    if (bbox[LO][Z] <= node->center[Z] + tree->delta)
		overlap |= 1 << HXLYLZ;
	}
    }
    if (bbox[LO][X] <= node->center[X] + tree->delta)
    {
	if (bbox[HI][Y] > node->center[Y] - tree->delta)
	{
	    if (bbox[HI][Z] > node->center[Z] - tree->delta)
		overlap |= 1 << LXHYHZ;
	    if (bbox[LO][Z] <= node->center[Z] + tree->delta)
		overlap |= 1 << LXHYLZ;
	}
	if (bbox[LO][Y] <= node->center[Y] + tree->delta)
	{
	    if (bbox[HI][Z] > node->center[Z] - tree->delta)
		overlap |= 1 << LXLYHZ;
	    if (bbox[LO][Z] <= node->center[Z] + tree->delta)
		overlap |= 1 << LXLYLZ;
	}
    }
#else
    int overlap;
    char hix, hiy, hiz, lox, loy, loz;
    
    overlap = 0;
    hix = bbox[HI][X] > node->center[X] - tree->delta;
    hiy = bbox[HI][Y] > node->center[Y] - tree->delta;
    hiz = bbox[HI][Z] > node->center[Z] - tree->delta;
    lox = bbox[LO][X] <= node->center[X] + tree->delta;
    loy = bbox[LO][Y] <= node->center[Y] + tree->delta;
    loz = bbox[LO][Z] <= node->center[Z] + tree->delta;
    
    if (hix)
    {
	if (hiy)
	{
	    if (hiz)
		overlap |= 1 << HXHYHZ;
	    if (loz)
		overlap |= 1 << HXHYLZ;
	    
	}
	if (loy)
	{
	    if (hiz)
		overlap |= 1 << HXLYHZ;
	    if (loz)
		overlap |= 1 << HXLYLZ;
	}
    }
    if (lox)
    {
	if (hiy)
	{
	    if (hiz)
		overlap |= 1 << LXHYHZ;
	    if (loz)
		overlap |= 1 << LXHYLZ;
	}
	if (loy)
	{
	    if (hiz)
		overlap |= 1 << LXLYHZ;
	    if (loz)
		overlap |= 1 << LXLYLZ;
	}
    }
#endif
    return overlap;
} 



static void init_children(Octree *tree, OctreeNode *node)
{
    int			i;
    Point		newsides;
    OctreeNode      	*child;
    
    VEC3_V_OP_S(newsides, node->sides, *, 0.5);
    
    ALLOCN(node->child, OctreeNode, 8);
    
    
    for (i = 0 ; i < 8; i++)
	VEC3_ASN_OP(node->child[i].sides, =, newsides);

    
    node->child[LXLYLZ].center[X] = node->center[X] - newsides[X];
    node->child[LXLYLZ].center[Y] = node->center[Y] - newsides[Y];
    node->child[LXLYLZ].center[Z] = node->center[Z] - newsides[Z];

    node->child[LXLYHZ].center[X] = node->center[X] - newsides[X];
    node->child[LXLYHZ].center[Y] = node->center[Y] - newsides[Y];
    node->child[LXLYHZ].center[Z] = node->center[Z] + newsides[Z];

    node->child[LXHYLZ].center[X] = node->center[X] - newsides[X];
    node->child[LXHYLZ].center[Y] = node->center[Y] + newsides[Y];
    node->child[LXHYLZ].center[Z] = node->center[Z] - newsides[Z];

    node->child[LXHYHZ].center[X] = node->center[X] - newsides[X];
    node->child[LXHYHZ].center[Y] = node->center[Y] + newsides[Y];
    node->child[LXHYHZ].center[Z] = node->center[Z] + newsides[Z];

    node->child[HXLYLZ].center[X] = node->center[X] + newsides[X];
    node->child[HXLYLZ].center[Y] = node->center[Y] - newsides[Y];
    node->child[HXLYLZ].center[Z] = node->center[Z] - newsides[Z];
    
    node->child[HXLYHZ].center[X] = node->center[X] + newsides[X];
    node->child[HXLYHZ].center[Y] = node->center[Y] - newsides[Y];
    node->child[HXLYHZ].center[Z] = node->center[Z] + newsides[Z];

    node->child[HXHYLZ].center[X] = node->center[X] + newsides[X];
    node->child[HXHYLZ].center[Y] = node->center[Y] + newsides[Y];
    node->child[HXHYLZ].center[Z] = node->center[Z] - newsides[Z];

    node->child[HXHYHZ].center[X] = node->center[X] + newsides[X];
    node->child[HXHYHZ].center[Y] = node->center[Y] + newsides[Y];
    node->child[HXHYHZ].center[Z] = node->center[Z] + newsides[Z];

    for (i = 0; i < 8; i++)
    {
	child = &(node->child[i]);
	child->parent = node;
	child->num_elements = 0;
	child->elements = NULL;
	child->child_elements = 0;
	child->child = NULL;
	child->max_elements = 0;
    }
} 


static void ot_insert(Octree *tree, OctreeNode *node, OctreeData *data)
{
    int         i, j;
    double      minside;
    int         overlap_flags;
    OctreeData *element;
    
    if (node->child == NULL)
    {
	if ((node->num_elements + 1) > node->max_elements)
	{
	    REALLOCN(node->elements, OctreeData *, node->max_elements,
		     FMAX((node->num_elements + 1),
			  (tree->max_node_objs+1)));
	    node->max_elements =
		FMAX((node->num_elements+1), (tree->max_node_objs+1));
	}
	
	node->elements[node->num_elements++] = data;

	if (node->num_elements > tree->max_node_objs)
	{
	    minside = FMIN(node->sides[X],
			   FMIN(node->sides[Y], node->sides[Z]));
	    if ((minside / 2.0) > tree->min_side)
	    {
		init_children(tree, node);
		for (i=0; i<node->num_elements; i++)
		{
		    element = node->elements[i];
		    overlap_flags =
			get_child_overlap(tree, node, element->bbox);
		    for (j=0; j<8; j++)
		    {
			if (overlap_flags & (1 << j))
			    ot_insert(tree, &(node->child[j]), element);
		    }
		}
		node->child_elements = node->num_elements;
		node->num_elements = 0;
		FREE(node->elements);
	    }
	    
	}
    }
    else
    {
	overlap_flags = get_child_overlap(tree, node, data->bbox);
	for (i=0; i<8; i++)
	{
	    if (overlap_flags & (1 << i))
		ot_insert(tree, &(node->child[i]), data);
	}
        node->child_elements++;
    }
} 


static void ot_remove(Octree *tree, OctreeNode *node, OctreeData *data)
{
    int i;
    OctreeData *element;
    int overlap_flags;
    
    if (node->child == NULL)
    {
	for (i=0; i<node->num_elements; i++)
	{
	    element = node->elements[i];
	    if (element == data)
	    {
		node->elements[i] = node->elements[node->num_elements-1];
		node->num_elements--;
		return;
	    }
	}
	fprintf(stderr, "octree_remove: data not found\n");
	return;
    }
    else
    {
	overlap_flags = get_child_overlap(tree, node, data->bbox);
	for (i=0; i<8; i++)
	{
	    if (overlap_flags & (1 << i))
		ot_remove(tree, &(node->child[i]), data);
	}
	node->child_elements--;
    }
    return;
} 


static void range_query(Octree *tree, OctreeNode *node, Extents bbox,
			OctreeData **element_list, int *num_elements)
{
    int i, overlap;
    OctreeData *element;
    
    if (node->child != NULL)
    {
	overlap = get_child_overlap(tree, node, bbox);
	for (i=0; i<8; i++)
	{
	    if (overlap & (1 << i))
		range_query(tree, &(node->child[i]), bbox,
			    element_list, num_elements);
	}
    }
    else
    {
	for (i=0; i<node->num_elements; i++)
	{
	    element = node->elements[i];
	    if ((element->found == FALSE) &&
		(BBOX_OVERLAP(bbox, element->bbox) == TRUE))
	    {
		element_list[(*num_elements)++] = element;
		element->found = TRUE;
	    }
	}
    }
    return;
} 





static OctreeNode *point_query(Octree *tree, OctreeNode *node, Point point)
{
    int overlap_flags;
    Extents bbox;
    int i, count;
    
    if (node->child == NULL)
	return node;

    VEC3_ASN_OP(bbox[HI], =, point);
    VEC3_ASN_OP(bbox[LO], =, point);
    
    overlap_flags = get_child_overlap(tree, node, bbox);
    for (i=0, count=0; i<8; i++)
	if (overlap_flags & (1<<i))
	    count++;
    if (count < 1)
    {
	fprintf(stderr, "octree_point_query: couldn't find node\n");
	return NULL;
    }
    if (count > 1)
    {
	fprintf(stderr, "octree_point_query: point too close to cell boundary\n");
	return NULL;
    }
    for (i=0; i<8; i++)
	if (overlap_flags & (1<<i))
	    return point_query(tree, &(node->child[i]), point);

    fprintf(stderr, "octree_point_query: couldn't find node\n");
    return NULL;
    
} 




static void destroy_node(OctreeNode *node)
{
    int i;
    
    if (node->child != NULL)
    {
	for (i=0; i<7; i++)
	    destroy_node(&(node->child[i]));
	FREE(node->child);
    }
    if (node->elements != NULL)
	FREE(node->elements);
    return;
} 



static void print_leaf_stats(Octree *tree, OctreeNode *node, int *leaf_count,
			     int *element_count, int *maxout_count)
{
    int i;

    if (node->child == NULL)
    {
#if 0
	fprintf(stderr, "leaf #%d: %d elements\n", *leaf_count,
	       node->num_elements);
#endif
	(*leaf_count)++;
	(*element_count) += node->num_elements;
	if (node->num_elements > tree->max_node_objs)
	    (*maxout_count)++;
    }
    else
    {
	for (i=0; i<8; i++)
	    print_leaf_stats(tree, &(node->child[i]), leaf_count,
			     element_count, maxout_count);
    }
    return;
} 





void octree_create(Octree **tree, Extents bbox,
		   double min_side, int max_node_objects)
{
    int i;
    OctreeNode *root;
    double bbox_minside;
    
    ALLOCN(*tree, Octree, 1);
    ALLOCN((*tree)->root, OctreeNode, 1);
    root = (*tree)->root;
    root->parent = NULL;
    root->num_elements = 0;
    root->elements = NULL;
    root->child_elements = 0;
    root->child = NULL;
    
    for (i=0; i<3; i++)
    {
	root->center[i] = (bbox[LO][i] + bbox[HI][i]) / 2.0;
	root->sides[i]  = (bbox[HI][i] - bbox[LO][i]) / 2.0;
    }

    bbox_minside = FMIN(root->sides[X], FMIN(root->sides[Y], root->sides[Z]));
    (*tree)->min_side = FMAX((min_side/2.0), (bbox_minside / MAX_RESOLUTION));
    (*tree)->delta = (*tree)->min_side * 0.0001;
    (*tree)->max_node_objs = FMAX(max_node_objects, MIN_MAX_NODE_OBJS);
} 


void octree_insert(Octree *tree, OctreeData *data)
{
    OctreeNode *root;
    
    root = tree->root;

    if ((data->bbox[HI][X] < (root->center[X] - root->sides[X])) ||
	(data->bbox[HI][Y] < (root->center[Y] - root->sides[Y])) ||
	(data->bbox[HI][Z] < (root->center[Z] - root->sides[Z])) ||
	(data->bbox[LO][X] > (root->center[X] + root->sides[X])) ||
	(data->bbox[LO][Y] > (root->center[Y] + root->sides[Y])) ||
	(data->bbox[LO][Z] > (root->center[Z] + root->sides[Z])))
    {
	fprintf(stderr, "octree_insert: data outside range of octree\n");
	return;
    }

    data->found = FALSE;
    ot_insert(tree, root, data);

    return;
} 


void octree_remove(Octree *tree, OctreeData *data)
{
    OctreeNode *root;
    
    root = tree->root;

    if ((data->bbox[HI][X] < (root->center[X] - root->sides[X])) ||
	(data->bbox[HI][Y] < (root->center[Y] - root->sides[Y])) ||
	(data->bbox[HI][Z] < (root->center[Z] - root->sides[Z])) ||
	(data->bbox[LO][X] > (root->center[X] + root->sides[X])) ||
	(data->bbox[LO][Y] > (root->center[Y] + root->sides[Y])) ||
	(data->bbox[LO][Z] > (root->center[Z] + root->sides[Z])))
    {
	fprintf(stderr, "octree_remove: data outside range of octree\n");
	return;
    }

    ot_remove(tree, root, data);
    
} 


void octree_destroy(Octree **tree)
{
    if ((*tree)->root != NULL)
	destroy_node((*tree)->root);
    FREE(*tree);
    return;
} 


void octree_range_query(Octree *tree, Extents bbox,
			OctreeData **element_list, int *num_elements)
{
    int         i;
    OctreeNode *root;
    
    root = tree->root;
    
    if ((bbox[HI][X] < (root->center[X] - root->sides[X])) ||
	(bbox[HI][Y] < (root->center[Y] - root->sides[Y])) ||
	(bbox[HI][Z] < (root->center[Z] - root->sides[Z])) ||
	(bbox[LO][X] > (root->center[X] + root->sides[X])) ||
	(bbox[LO][Y] > (root->center[Y] + root->sides[Y])) ||
	(bbox[LO][Z] > (root->center[Z] + root->sides[Z])))
    {
	fprintf(stderr, "octree_range_query: ");
	fprintf(stderr, "query range outside of tree range (not fatal)\n");
	*num_elements = 0;
	return;
    }

    *num_elements = 0;

    range_query(tree, tree->root, bbox, element_list, num_elements);

    for (i=0; i < (*num_elements); i++)
	element_list[i]->found = FALSE;
    
} 




OctreeNode *octree_point_query(Octree *tree, Point point)
{
    OctreeNode *root;

    root = tree->root;
    
    if ((point[X] < (root->center[X] - root->sides[X])) ||
	(point[Y] < (root->center[Y] - root->sides[Y])) ||
	(point[Z] < (root->center[Z] - root->sides[Z])) ||
	(point[X] > (root->center[X] + root->sides[X])) ||
	(point[Y] > (root->center[Y] + root->sides[Y])) ||
	(point[Z] > (root->center[Z] + root->sides[Z])))
    {
	fprintf(stderr, "octree_point_query: point outside of tree range\n");
	return NULL;
    }
    
    return point_query(tree, tree->root, point);
    
} 



void octree_print_stats(Octree *tree)
{
    int leaf_count, total_leaf_elements, maxout_count;
    float average_leaf_elements;
    
    fprintf(stderr, "octree statistics:\n");

    leaf_count = 0;
    total_leaf_elements = 0;
    maxout_count = 0;
    print_leaf_stats(tree, tree->root, &leaf_count, &total_leaf_elements,
		     &maxout_count);

    average_leaf_elements = (float)total_leaf_elements/(float)leaf_count;

    fprintf(stderr, "\tnumber of elements    : %d\n",
	    FMAX(tree->root->child_elements, tree->root->num_elements));
    fprintf(stderr, "\tmax_node_objects      : %d\n", tree->max_node_objs);
    fprintf(stderr, "\ttotal number of leaves: %d\n", leaf_count);
    fprintf(stderr, "\taverage elements/leaf : %f\n", average_leaf_elements);
    fprintf(stderr, "\tmaxed-out leaves      : %d\n", maxout_count);
    fprintf(stderr, "\n");

    return;
} 





