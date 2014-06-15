

#include "vertex_octree.h"
#include "bbox.h"
#include "private.h"


static BBOX_COMPARE_FUNC brep_vertex_clientdata_compare = (BBOX_COMPARE_FUNC)NULL,
                         brep_vertex_location_compare = (BBOX_COMPARE_FUNC)NULL;


BBOX_COMPARE_FUNC BBoxSetVertexCompareRoutine(BBOX_COMPARE_FUNC routine)
{
  BBOX_COMPARE_FUNC oldroutine = brep_vertex_clientdata_compare;
  brep_vertex_clientdata_compare = routine;
  return oldroutine;
}


BBOX_COMPARE_FUNC BBoxSetVertexCompareLocationRoutine(BBOX_COMPARE_FUNC routine)
{
  BBOX_COMPARE_FUNC oldroutine = brep_vertex_location_compare;
  brep_vertex_location_compare = routine;
  return oldroutine;
}


int BBoxVertexCompare(BBOX_VERTEX *v1, BBOX_VERTEX *v2)
{
  if (!brep_vertex_clientdata_compare)
    BBoxFatal(v1->client_data, "BBoxVertexCompare", "no user specified vertex compare routine!");

  return brep_vertex_clientdata_compare(v1->client_data, v2->client_data);
}


int BBoxVertexCompareLocation(BBOX_VERTEX *v1, BBOX_VERTEX *v2)
{
  if (!brep_vertex_location_compare)
    BBoxFatal(v1->client_data, "BBoxVertexCompareLocation", "no user specified routine for comparing the location of two vertices");

  return brep_vertex_location_compare(v1->client_data, v2->client_data);
}


BBOX_VERTEX *BBoxFindVertex(void *vertex_data, BBOX_VERTEX_OCTREE *vertices)
{
  BBOX_VERTEX vert;
  vert.client_data = vertex_data;
  
  return BBoxVertexOctreeFind(vertices, &vert);
}


void BBoxAttachVertex(BBOX_VERTEX *vertex, BBOX_VERTEX_OCTREE **vertices)
{
  *vertices = BBoxVertexOctreeAdd(*vertices, vertex);
}


void BBoxReleaseVertex(BBOX_VERTEX *vertex, BBOX_VERTEX_OCTREE **vertices)
{
  BBoxError(vertex->client_data, "BBoxReleaseVertex", "not yet implemented");
}


BBOX_VERTEX *BBoxInstallVertex(void *vertex_data, BBOX_VERTEX_OCTREE **vertices)
{
  BBOX_VERTEX *vert;

  vert = BBoxCreateVertex(vertex_data);
  *vertices = BBoxVertexOctreeAdd(*vertices, vert);

  return vert;
}


BBOX_VERTEX_OCTREE *BBoxCreateVertexOctree(void)
{
  return BBoxVertexOctreeCreate();
}


void BBoxDestroyVertexOctree(BBOX_VERTEX_OCTREE *vertices)
{
  BBoxVertexOctreeDestroy(vertices);
}


void BBoxIterateVertices(BBOX_VERTEX_OCTREE *vertices, void (*func)(BBOX_VERTEX *))
{
  BBoxVertexOctreeIterate(vertices, func);
}


void BBoxDestroyVertices(BBOX_VERTEX_OCTREE *vertices)
{
  BBoxIterateVertices(vertices, BBoxDestroyVertex);
  BBoxDestroyVertexOctree(vertices);
}


BBOX_VERTEX_OCTREE *BBoxFindVertexAtLocation(void *vertex_data, BBOX_VERTEX_OCTREE *vertices)
{
  BBOX_VERTEX vert;
  vert.client_data = vertex_data;
  
  return (BBOX_VERTEX_OCTREE *)OctreeFindSubtree((OCTREE *)vertices, (void *)&vert, (int (*)(void *, void *))BBoxVertexCompareLocation);
}


void BBoxIterateVerticesAtLocation(void *vertex_data, BBOX_VERTEX_OCTREE *vertices,
				   void (*routine)(BBOX_VERTEX *vertex))
{
  BBOX_VERTEX_OCTREE *vertoct;

  vertoct = BBoxFindVertexAtLocation(vertex_data, vertices);
  OctreeIterate((OCTREE *)vertoct, (void (*)(void *))routine);
}


void BBoxIterateVerticesAtLocation1A(void *vertex_data, BBOX_VERTEX_OCTREE *vertices,
				     void (*routine)(BBOX_VERTEX *vertex, void *parm), 
				     void *parm)
{
  BBOX_VERTEX_OCTREE *vertoct;

  vertoct = BBoxFindVertexAtLocation(vertex_data, vertices);
  OctreeIterate1A((OCTREE *)vertoct, (void (*)(void *, void *))routine, parm);
}


void BBoxIterateVerticesAtLocation2A(void *vertex_data, BBOX_VERTEX_OCTREE *vertices,
				     void (*routine)(BBOX_VERTEX *vertex, void *parm1, void *parm2),
				     void *parm1, void *parm2)
{
  BBOX_VERTEX_OCTREE *vertoct;

  vertoct = BBoxFindVertexAtLocation(vertex_data, vertices);
  OctreeIterate2A((OCTREE *)vertoct, (void (*)(void *, void *, void *))routine, parm1, parm2);
}

static void BBoxIterateWingsWithVertices(BBOX_VERTEX *v2, BBOX_VERTEX_OCTREE *v1octree, 
					 void (*func)(BBOX_WING *))
{
  OctreeIterate2A((OCTREE *)v1octree, (void (*)(void *, void *, void *))BBoxIterateWingsWithVertex, (void *)v2, (void *)func);
}


void BBoxIterateWingsBetweenLocations(void *v1data, void *v2data,
				      BBOX_VERTEX_OCTREE *vertices,
				      void (*func)(BBOX_WING *))
{
  BBOX_VERTEX_OCTREE *v1octree, *v2octree;

  v1octree = BBoxFindVertexAtLocation(v1data, vertices);
  if (!v1octree)
    return;

  v2octree = BBoxFindVertexAtLocation(v2data, vertices);
  if (!v2octree)
    return;

  OctreeIterate2A((OCTREE *)v2octree, (void (*)(void *, void *, void *))BBoxIterateWingsWithVertices, (void *)v1octree, (void *)func);
}
