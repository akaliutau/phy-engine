/* Bintree.h: binary trees */

#ifndef _BINTREE_H_
#define _BINTREE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BINTREE {
	void *pelement;
	struct BINTREE *left, *right;
} BINTREE;

#define BinTreeCreate()	(BINTREE *)NULL

extern void *BinTreeFind(BINTREE *bintree, void *pelement, int (*nodecmp)(void *, void *));

extern BINTREE *BinTreeAdd(BINTREE *bintree, void *pelement, int (*nodecmp)(void *, void *));

extern void BinTreeIterate(BINTREE *bintree, void (*func)(void *));

extern void BinTreeDestroy(BINTREE *bintree);

#ifdef __cplusplus
}
#endif

#endif /* _BINTREE_H_ */
