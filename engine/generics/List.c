

#include "List.h"
#include "private.h"
#include "pools.h"

#ifndef NOPOOLS
static POOL *listPool = (POOL *)NULL;
#define NEWLIST()  	(LIST *)NewPoolCell(sizeof(LIST), 0, "generics list cells", &listPool)
#define DISPOSELIST(ptr) Dispose((unsigned char *)(ptr), &listPool)
#else 
#define NEWLIST()	(LIST *)Alloc(sizeof(LIST))
#define DISPOSELIST(ptr) Free((char *)ptr, sizeof(LIST))
#endif 


#ifndef NOLISTMACROS
void *__plistel__;	
#endif 

#ifdef NOLISTMACROS

LIST *ListCreate(void)
{
	return (LIST *)NULL;
}
#endif 


LIST *ListAdd(LIST *list, void *pelement)
{
	LIST *newlist;


	if (pelement == (void *)NULL)
		return list;

	newlist = NEWLIST();
	newlist->pelement = pelement;
	newlist->next = list;

	return newlist;
}


int ListCount(LIST *list)
{
	int count = 0;

	while (list) {
		count++;
		list = list->next;
	}

	return count;
}


void *ListGet(LIST *list, int index)
{
	while (list && index > 0) {
		list = list->next;
		index--;
	}

	if (!list)
		return (void *)NULL;
	else
		return list->pelement;
}

#ifdef NOLISTMACROS

void *ListNext(LIST **list)
{
	void *pelement = (void *)NULL;

	if (*list) {
		pelement = (*list)->pelement;
		*list = (*list)->next;
	} else
		pelement = (void *)NULL;

	return pelement;
}
#endif 


LIST *ListMerge(LIST *list1, LIST *list2)
{
	void *pelement = (void *)NULL;

	while ((pelement = ListNext(&list2)))
		list1 = ListAdd(list1, pelement);

	return list1;
}


LIST *ListDuplicate(LIST *list)
{
	LIST *newlist = (LIST *)NULL;
	void *pelement = (void *)NULL;

	while ((pelement = ListNext(&list)))
		newlist = ListAdd(newlist, pelement);

	return newlist;
}


LIST *ListRemove(LIST *list, void *pelement)
{
	LIST *p, *q;


	if (!list) {
		GdtError("ListRemove", "attempt to remove an element from an empty list");
		return list;	
	}


	if (pelement == list->pelement) {
		p = list->next;
		DISPOSELIST(list);
		return p;
	}


	q = list;
	p = list->next;
	while (p && p->pelement != pelement) {
		q = p;
		p = p->next;
	}


	if (!p) {
		GdtError("ListRemove", "attempt to remove a nonexisting element from a list");
		return list;
	}


	else {
		q->next = p->next;
		DISPOSELIST(p);
	}


	return list;
}


LIST *ListIterateRemove(LIST *list, int (*func)(void *))
{
  LIST *p, *q;

  
  while (list && func(list->pelement) != 0) {
    
    p = list;
    list = list->next;
    DISPOSELIST(p);
  }

  if (!list)
    return list;	

  
  q = list;
  p = list->next;
  while (p) {
    if (func(p->pelement) != 0) {
      
      q->next = p->next;
      DISPOSELIST(p);
      p = q->next;
    } else {
      
      q = p; p = p->next;
    }
  }

  return list;
}

#ifdef NOLISTMACROS

void ListIterate(LIST *list, void (*proc)(void *))
{
	void *pelement;

	while (list) {
		pelement = list->pelement;
		list = list->next;
		proc(pelement);
	}		
}
#endif 

#ifdef NOLISTMACROS

void ListIterate1A(LIST *list, void (*proc)(void *, void *), void *extra)
{
	void *pelement;

	while (list) {
		pelement = list->pelement;
		list = list->next;
		proc(pelement, extra);
	}		
}
#endif 

#ifdef NOLISTMACROS

void ListIterate1B(LIST *list, void (*proc)(void *, void *), void *extra)
{
	void *pelement;

	while (list) {
		pelement = list->pelement;
		list = list->next;
		proc(extra, pelement);
	}		
}
#endif 


void ListDestroy(LIST *list)
{
	LIST *p;

	while (list) {
		p = list->next;
		DISPOSELIST(list);
		list = p;
	}
}

#ifdef TEST
#include <stdio.h>

static int odd(void *bla)
{
  int num = (int)bla;
  return (num%2 == 1);
}

static void print(void *bla)
{
  fprintf(stderr, "%d ", (int)bla);
}

int main(int argc, char **argv)
{
  LIST *list = ListCreate();
  int n;

  fprintf(stderr, "ListIterateRemove() test: removes all odd number from a list:\n\n");
  fprintf(stderr, "Enter a series of integer numbers. Terminate the list with a 0 (zero):\n");
  
  scanf("%d", &n);
  while (n != 0) {
    list = ListAdd(list, (void *)n);
    scanf("%d", &n);
  }

  fprintf(stderr, "The list you entered is (reversed and without terminating zero):\n");
  ListIterate(list, print);
  fprintf(stderr, "\n");

  fprintf(stderr, "Now removing all odd numbers ... \n");
  list = ListIterateRemove(list, odd);
	  
  fprintf(stderr, "The result should contain all even numbers of the list:\n");
  ListIterate(list, print);
  fprintf(stderr, "\n");

  return 0;
}

#endif 
