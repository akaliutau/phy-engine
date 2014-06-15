

#include "vertexlist.h"
#include "vertex.h"

VERTEX *VertexListFind(VERTEXLIST *vl, VECTOR *point, VECTOR *norm, VECTOR *texCoord)
{
  VERTEX ref;
  ref.point = point;
  ref.normal = norm;
  ref.texCoord = texCoord;	

  ForAllVertices(v, vl) {
    if (VertexCompare(v, &ref) == XYZ_EQUAL)
      return v;
  } EndForAll;

  return (VERTEX *)NULL;
}

