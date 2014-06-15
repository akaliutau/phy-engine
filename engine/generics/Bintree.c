

#include "Bintree.h"
#include "pools.h"
#include "private.h"

#ifndef NOPOOLS
static POOL *bintreePool = (POOL *)NULL;
#define NEWBINTREENODE()  	(BINTREE *)NewPoolCell(sizeof(BINTREE), 0, "generics bintree cells", &bintreePool)
#define DISPOSEBINTREENODE(ptr) Dispose((unsigned char *)(ptr), &bintreePool)
#else 
#define NEWBINTREENODE()	(BINTREE *)Alloc(sizeof(BINTREE))
#define DISPOSEBINTREENODE(ptr) Free((char *)ptr, sizeof(BINTREE))
#endif 

BINTREE *BinTreeNewNode(void *pelement)
{
	BINTREE *p;

	p = NEWBINTREENODE();
	p->pelement = pelement;
	p->left = p->right = (BINTREE *)NULL;

	return p;
}

void *BinTreeFind(BINTREE *bintree, void *pelement, int (*nodecmp)(void *, void *))
{
	BINTREE *p;
	int cmp;
	
	p = bintree;
	while (p) {
		cmp = nodecmp(p->pelement, pelement);
		if (cmp < 0) 
			p = p->left;
		else if (cmp == 0)
			return p->pelement;
		else
			p = p->right;
	}

	return (void *)NULL;
}

BINTREE *BinTreeAdd(BINTREE *bintree, void *pelement, int (*nodecmp)(void *, void *))
{
	BINTREE *o, *p;
	int cmp=0;

	if (!pelement)
		return bintree;

	o = (BINTREE *)NULL;
	p = bintree;
	while (p) {
		o = p;
		cmp = nodecmp(p->pelement, pelement);
		if (cmp < 0)
			p = p->left;
		else if (cmp == 0)
			p = p->left; 	
		else 
			p = p->right;
	}

	p = BinTreeNewNode(pelement);

	if (o) {	
		if (cmp < 0)
			o->left = p;
		else
			o->right = p;
		return bintree;
	} else
		return p;
}

void BinTreeIterate(BINTREE *bintree, void (*func)(void *))
{
	if (bintree) {
		BinTreeIterate(bintree->left, func);
		func(bintree->pelement);
		BinTreeIterate(bintree->right, func);
	}
}

void BinTreeDestroy(BINTREE *bintree)
{
	if (bintree) {
		BinTreeDestroy(bintree->left);
		BinTreeDestroy(bintree->right);
		DISPOSEBINTREENODE(bintree);
	}
}


