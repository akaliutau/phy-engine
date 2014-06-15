

#include "Octree.h"
#include "pools.h"

#ifndef NOPOOLS
static POOL *octreePool = (POOL *)NULL;
#define NEWOCTREENODE()  	(OCTREE *)NewPoolCell(sizeof(OCTREE), 0, "generics Octree cells", &octreePool)
#define DISPOSEOCTREENODE(ptr) Dispose((unsigned char *)(ptr), &octreePool)
#else 
#define NEWOCTREENODE()	(OCTREE *)Alloc(sizeof(OCTREE))
#define DISPOSEOCTREENODE(ptr) Free((char *)ptr, sizeof(OCTREE))
#endif 

OCTREE *NewOctreeNode(void *pelement)
{
	OCTREE *nt;
	int i;

	nt = NEWOCTREENODE();
	nt->pelement = pelement;
	for (i=0; i<8; i++)
		nt->child[i] = (void *)NULL;

	return nt;
}

OCTREE *OctreeAddWithDuplicates(OCTREE *octree, 
				void *pelement, 
				int (*nodecmp)(void *pelem1, void *pelem2))
{
	OCTREE *o, *p;
	int cmp=8;

	o = (OCTREE *)NULL;
	p = octree;
	while (p) {
		cmp = nodecmp(p->pelement, pelement);
		o = p;
		p = p->child[cmp%8];	
	}

	
	p = NewOctreeNode(pelement);

	if (o) {		
		o->child[cmp%8] = p;
		return octree;
	} else 
		return p;	
}


OCTREE *OctreeAdd(OCTREE *octree, 
		  void *pelement, 
		  int (*nodecmp)(void *pelem1, void *pelem2))
{
	OCTREE *o, *p;
	int cmp=8;

	o = (OCTREE *)NULL;
	p = octree;
	while (p) {
		cmp = nodecmp(p->pelement, pelement);
		if (cmp >= 8)	
		  return octree;
		else {
		  o = p;
		  p = p->child[cmp];	
		}
	}

	
	p = NewOctreeNode(pelement);

	if (o) {		
		o->child[cmp] = p;
		return octree;
	} else 
		return p;	
}

OCTREE *OctreeFindSubtree(OCTREE *octree, 
			  void *pelement, 
			  int (*nodecmp)(void *pelem1, void *pelem2))
{
	OCTREE *p;
	int cmp;

	p = octree;
	while (p) {
		cmp = nodecmp(p->pelement, pelement);
		if (cmp >= 8)
			return p;
		else 
			p = p->child[cmp];
	}

	return (OCTREE *)NULL;
}

void *OctreeFind(OCTREE *octree, 
		 void *pelement, 
		 int (*nodecmp)(void *pelem1, void *pelem2))
{
  OCTREE *p = OctreeFindSubtree(octree, pelement, nodecmp);

  if (p)
    return p->pelement;
  else
    return (void *)NULL;
}

void OctreeIterate(OCTREE *octree, void (*func)(void *pelem))
{
	int i;

	if (octree) {
		for (i=0; i<8; i++)
			OctreeIterate(octree->child[i], func);
		func(octree->pelement);
	}
}

void OctreeIterate1A(OCTREE *octree, void (*func)(void *pelem, void *parm), void *parm)
{
	int i;

	if (octree) {
		for (i=0; i<8; i++)
			OctreeIterate1A(octree->child[i], func, parm);
		func(octree->pelement, parm);
	}
}

void OctreeIterate2A(OCTREE *octree, void (*func)(void *pelem, void *parm1, void *parm2), void *parm1, void *parm2)
{
	int i;

	if (octree) {
		for (i=0; i<8; i++)
			OctreeIterate2A(octree->child[i], func, parm1, parm2);
		func(octree->pelement, parm1, parm2);
	}
}

void OctreeDestroy(OCTREE *octree)
{
	int i;

	if (octree) {
		for (i=0; i<8; i++)
			OctreeDestroy(octree->child[i]);
		DISPOSEOCTREENODE(octree);
	}
}



