/* list.h: generic doubly linked linear lists */

#ifndef _DLIST_H_
#define _DLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DLIST {
	void 	*pelement;		/* pointer to an element */
	struct DLIST *prev, *next;	/* pointer to the rest of the dlist */
} DLIST;

/* some of the DList precodures are implemented both as a macro and as a
 * C function. Define NODLISTMACROS to use the C-function implementation. Macros
 * have the advantage of avoiding a function call however. These routines are
 * so often used that you really save time by using the macros. */
/* #define NODLISTMACROS */

/* creates an empty DList */
#ifdef NODLISTMACROS
extern DLIST *DListCreate(void);
#else /*NODLISTMACROS*/
#define DListCreate()  ((DLIST *)NULL)
#endif /*NODLISTMACROS*/

/*
 * iterator: iterates over all elements in the list 'list', containing
 * pointers to elements of type 'TYPE'. The elements are named 'p'.
 * Use this as follows:
 * 
 * ForAllInDList(PATCH, P, patches) {
 *  do something with 'P';
 * } EndForAll;
 *
 * PS: you can make things even nicer if you define macro's such as
 * #define ForAllPatches(p, patches) ForAllInDList(PATCH, p, patches)
 * You can than use
 * ForAllPatches(P, patches) {
 *  do something with 'P';
 * } EndForAll;
 */
#define ForAllInDList(TYPE, p, list) { 		\
  DLIST *_list_ = (DLIST *)(list);		\
  if (_list_) {					\
    DLIST *_l_;					\
    for (_l_ = _list_; _l_; _l_ = _l_->next) {	\
      TYPE *p = (TYPE *)(_l_->pelement);

#ifndef EndForAll	/* all definitions of EndForAll are the same */
#define EndForAll }}}
#endif

/* adds an element in front of the dlist, returns a pointer to the new dlist */
extern DLIST *DListAdd(DLIST *dlist, void *element);

/* counts the number of elements in a dlist */
extern int DListCount(DLIST *dlist);

/* returns the index-th element from the dlist or NULL if there are less
 * than index elements in the dlist. Indices count from 0 */
extern void *DListGet(DLIST *dlist, int index);

/* the first argument is the adres of a DLIST *. First call this function
 * with the adres to the DLIST * being the adres of a pointer to the first 
 * element of the dlist. By calling this function successively, a
 * pointer to each element of the dlist is returned in sequence. 
 * This function returns NULL after the last element of the dlist
 * has been encountered. 
 *
 * WARNING: DListNext() calls cannot be nested! */
#ifdef NODLISTMACROS
extern void *DListNext(DLIST **dlist);
#else /*NODLISTMACROS*/
extern void *__pdlistel__;
#define DListNext(pdlist)	((*(pdlist)) ? (__pdlistel__=(*(pdlist))->pelement, (*(pdlist))=(*(pdlist))->next, __pdlistel__) : (void *)NULL)
#endif /*NODLISTMACROS*/

/* merge two dlists: the elements of dlist2 are prepended to the elements
 * of dlist1. Returns a pointer to the enlarged dlist1 */
extern DLIST *DListMerge(DLIST *dlist1, DLIST *dlist2);

/* duplicates a dlist: the elements are not duplicated: only a pointer
 * to the elements is copied. */
extern DLIST *DListDuplicate(DLIST *dlist);

/* verwijdert de cel p uit de lijst dlist */
extern DLIST *DListRemoveCell(DLIST *dlist, DLIST *p);

/* removes an element from the dlist. Returns a pointer to the updated
 * dlist. */
extern DLIST *DListRemove(DLIST *dlist, void *pelement);

/* iterators: executes the procedure for each element of the dlist.
 * There are a number of iterators here: use DlistIterate with
 * a procedure that accepts only one parameter: a pointer to an element */
#ifdef NODLISTMACROS
extern void DListIterate(DLIST *dlist, void (*proc)(void *));
#else /*NODLISTMACROS*/
#define DListIterate(dlist, proc)						\
{									\
	DLIST *_dlist = (dlist);						\
	void *pelement;							\
	while (_dlist) {							\
		pelement = _dlist->pelement;				\
		_dlist = _dlist->next;					\
		((void (*)(void *))proc)(pelement);			\
	}								\
}
#endif /*NODLISTMACROS*/

/* use DListIterate1A with procedures that accepts two parameters: first
 * a pointer to the dlist element, then a pointer to the "extra" data */
#ifdef NODLISTMACROS
extern void DListIterate1A(DLIST *dlist, void (*proc)(void *, void *), void *extra);
#else /*NODLISTMACROS*/
#define DListIterate1A(dlist, proc, extra)				\
{									\
	DLIST *_dlist = (dlist);						\
	void *pelement;							\
	while (_dlist) {							\
		pelement = _dlist->pelement;				\
		_dlist = _dlist->next;					\
		((void (*)(void *, void *))proc)(pelement, (void *)(extra));\
	}								\
}
#endif /*NODLISTMACROS*/

/* use DListIterate1B with procedures that accepts two parameters: first
 * a pointer to the "extra" data, then a pointer to a dlist element */
#ifdef NODLISTMACROS
extern void DListIterate1B(DLIST *dlist, void (*proc)(void *, void *), void *extra);
#else /*NODLISTMACROS*/
#define DListIterate1B(dlist, proc, extra)				\
{									\
	DLIST *_dlist = (dlist);						\
	void *pelement;							\
	while (_dlist) {							\
		pelement = _dlist->pelement;				\
		_dlist = _dlist->next;					\
		((void (*)(void *, void *))proc)((void *)(extra), pelement);\
	}								\
}
#endif /*NODLISTMACROS*/

/* destroys a dlist. Does not destroy the elements. */
extern void DListDestroy(DLIST *dlist);

/* sorts a doubly linked list with simple insertion sort. */
extern DLIST *DListSort(DLIST *list, int (*compare)(void *, void *));

#ifdef __cplusplus
}
#endif

#endif /* _DLIST_H_ */

