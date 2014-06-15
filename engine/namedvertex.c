

#include <string.h>
#include "namedvertex.h"
#include "Bintree.h"
#include "vertexlist.h"
#include "pools.h"

#ifdef NOPOOLS
#define NEWNAMEDVERTEXLISTNODE()	(NAMEDVERTEXLIST *)Alloc(sizeof(NAMEDVERTEXLIST))
#define DISPOSENAMEDVERTEXLISTNODE(ptr) Free((char *)ptr, sizeof(NAMEDVERTEXLIST))
#else
static POOL *namedvertexlistPool = (POOL *)NULL;
#define NEWNAMEDVERTEXLISTNODE()	(NAMEDVERTEXLIST *)NewPoolCell(sizeof(NAMEDVERTEXLIST), 0, "namedvertices", &namedvertexlistPool)
#define DISPOSENAMEDVERTEXLISTNODE(ptr) Dispose((char *)ptr, &namedvertexlistPool)
#endif

static int NamedVertexListNameCompare(NAMEDVERTEXLIST *nvl, char *name)
{
  return strcmp(nvl->name, name);
}

static int NamedVertexListCompare(NAMEDVERTEXLIST *nvl1, NAMEDVERTEXLIST *nvl2)
{
  return strcmp(nvl1->name, nvl2->name);
}

static NAMEDVERTEXLIST *NewNamedVertexList(char *name, VERTEX *vertex)
{
  NAMEDVERTEXLIST *nvl;

  nvl = NEWNAMEDVERTEXLISTNODE();
  nvl->name = Alloc(strlen(name)+1);
  strcpy(nvl->name, name);
  nvl->vertices = VertexListAdd(VertexListCreate(), vertex);

  return nvl;
}

NAMEDVERTEXTREE *NamedVertexAdd(NAMEDVERTEXTREE *namedvertices, char *name, VERTEX *vertex)
{
  NAMEDVERTEXLIST *nvl;

  if (!(nvl = (NAMEDVERTEXLIST *)BinTreeFind((BINTREE *)namedvertices, name, (int (*)(void *, void *))NamedVertexListNameCompare))) {
    nvl = NewNamedVertexList(name, vertex);
    return (NAMEDVERTEXTREE *)BinTreeAdd((BINTREE *)namedvertices, nvl, (int (*)(void *, void *))NamedVertexListCompare);
  } else {
    nvl->vertices = VertexListAdd(nvl->vertices, vertex);
    return namedvertices;
  }
}

VERTEX *NamedVertexFind(NAMEDVERTEXTREE *namedvertices, char *name, POINT *point, VECTOR *normal)
{
  NAMEDVERTEXLIST *nvl;

  if ((nvl = (NAMEDVERTEXLIST *)BinTreeFind((BINTREE *)namedvertices, name, (int (*)(void *, void *))NamedVertexListNameCompare))) {
    return VertexListFind(nvl->vertices, point, normal, (VECTOR*)NULL);
  }

  return (VERTEX *)NULL;
}

static void DisposeNamedVertexList(NAMEDVERTEXLIST *nvl)
{
  Free(nvl->name, strlen(nvl->name)+1);
  VertexListDestroy(nvl->vertices);
  DISPOSENAMEDVERTEXLISTNODE(nvl);
}

void NamedVertexTreeDestroy(NAMEDVERTEXTREE *namedvertices)
{
  BinTreeIterate((BINTREE *)namedvertices, (void (*)(void *))DisposeNamedVertexList);
  BinTreeDestroy((BINTREE *)namedvertices);
}


