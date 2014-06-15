
#include <math.h>

#include "mcradP.h"

#include "hierarchy.h"
#include "scene.h"
#include "vertex.h"
#include "statistics.h"
#include "error.h"
#include "options.h"



ELEM_HIER_STATE hierarchy;


void ElementHierarchyDefaults(void)
{
  hierarchy.epsilon         = DEFAULT_EH_EPSILON;
  hierarchy.minarea         = DEFAULT_EH_MINAREA;
  hierarchy.do_h_meshing    = DEFAULT_EH_HIERARCHICAL_MESHING;
  hierarchy.clustering      = DEFAULT_EH_CLUSTERING;
  hierarchy.tvertex_elimination = DEFAULT_EH_TVERTEX_ELIMINATION;
  hierarchy.oracle = PowerOracle;

  hierarchy.nr_elements     = 0;
  hierarchy.nr_clusters     = 0;
}


static CMDLINEOPTDESC hierOptions[] = {
  {NULL	, 	0,	TYPELESS, 	NULL, 	DEFAULT_ACTION,
   NULL}
};

void ElementHierarchyParseOptions(int *argc, char **argv)
{
  ParseOptions(hierOptions, argc, argv);
}

void ElementHierarchyPrintOptions(FILE *fp)
{
  fprintf(fp, "\nElement hierarchy options:\n");
  PrintOptions(fp, hierOptions);
}

void ElementHierarchyInit(void)
{  
  

  hierarchy.maxlinkpow     = hierarchy.epsilon * 
                             COLORMAXCOMPONENT(max_selfemitted_power);

  

  hierarchy.coords         = VectorListCreate();
  hierarchy.normals        = VectorListCreate();
  hierarchy.texCoords	   = VectorListCreate();
  hierarchy.vertices       = VertexListCreate();

  hierarchy.topcluster     = McrCreateClusterHierarchy(ClusteredWorldGeom);
}

void ElementHierarchyTerminate(void)
{
  
  McrDestroyClusterHierarchy(hierarchy.topcluster);
  hierarchy.topcluster = (ELEMENT *)NULL;

  
  ForAllPatches(P, Patches) {
    
    McrDestroyToplevelSurfaceElement(TOPLEVEL_ELEMENT(P));
    P->radiance_data = (void *)NULL; 
  } EndForAll;

  VertexListIterate(hierarchy.vertices, VertexDestroy);
  VertexListDestroy(hierarchy.vertices);
  hierarchy.vertices = VertexListCreate();
  VectorListIterate(hierarchy.coords, VectorDestroy);
  VectorListDestroy(hierarchy.coords);
  hierarchy.coords = VectorListCreate();
  VectorListIterate(hierarchy.normals, VectorDestroy);
  VectorListDestroy(hierarchy.normals);
  hierarchy.normals = VectorListCreate();
  VectorListIterate(hierarchy.texCoords, VectorDestroy);
  VectorListDestroy(hierarchy.texCoords);
  hierarchy.texCoords = VectorListCreate();
}

