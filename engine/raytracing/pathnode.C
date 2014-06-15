

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>

#include "pathnode.H"
#include "error.h"
#include "scene.h"



int CPathNode::m_dmaxsize = 0;

CPathNode::CPathNode(void)
{ 
  m_next = NULL; 
  m_previous = NULL;

}


CPathNode::~CPathNode(void)
{ 
}

// Delete all nodes, includind the supplied 'node'
void CPathNode::ReleaseAll(CPathNode *node)
{
  CPathNode *tmp;

  while(node)
  {
	tmp = node;
	node = node->Next();
	delete tmp;
  }
}

void CPathNode::Print(FILE *out)
{
  fprintf(out, "Pathnode at depth %i\n", m_depth);
  fprintf(out, "Pos : "); VectorPrint(out, m_hit.point); fprintf(out,"\n");
  fprintf(out, "Norm: "); VectorPrint(out, m_normal); fprintf(out,"\n");
  if(m_previous)
  {
	fprintf(out, "InF: "); VectorPrint(out, m_inDirF); fprintf(out,"\n");
	fprintf(out, "Cos in  %f\n", VECTORDOTPRODUCT(m_normal, m_inDirF));
	fprintf(out, "GCos in %f\n", VECTORDOTPRODUCT(m_hit.patch->normal, 
												  m_inDirF));
  }
  if(m_next)
  {
	fprintf(out, "OutF: "); VectorPrint(out, m_next->m_inDirT); fprintf(out,"\n");
	fprintf(out, "Cos out %f\n", VECTORDOTPRODUCT(m_normal, m_next->m_inDirT));
	fprintf(out, "GCos out %f\n", VECTORDOTPRODUCT(m_hit.patch->normal, 
												  m_next->m_inDirT));
  }
}







CPathNode *CPathNode::GetMatchingNode(void)
{
  BSDF *thisBsdf;
  int backhits;
  CPathNode *tmpNode = Previous();
  CPathNode *matchedNode = NULL;

  thisBsdf = m_useBsdf;
  backhits = 1;

  while(tmpNode && backhits > 0)
  {
    switch(tmpNode->m_rayType)
    {
    case Enters:
      if(tmpNode->m_hit.patch->surface->material->bsdf == thisBsdf)
      {
	backhits--; // Aha an entering point in this material
      }
      break;
    case Leaves:
      if(tmpNode->m_inBsdf == thisBsdf)
      {
	backhits++; // Leaves the same material more than one time
      }
      break;
    case Reflects:
      break;
    default:
      Error("CPathNode::GetMatchingNode", "Wrong ray type in path");
    }
    
    matchedNode = tmpNode;
    tmpNode = tmpNode->Previous();
  }

  if(backhits == 0)
  {
    return(matchedNode);
  }
  else
  {
    return NULL;  // No matching node
  }
}


BSDF *CPathNode::GetPreviousBsdf(void)
{
  CPathNode *matchedNode;

  if(!(m_hit.flags & HIT_BACK))
  {
    Error("CPathNode::GetPreviousBsdf", "Last node not a back hit");
    return(m_inBsdf);  // Should not happen
  }
  
  if(m_hit.patch->surface->material->bsdf != 
     m_inBsdf)
  {
    Warning("CPathNode::GetPreviousBtdf", "Last back hit has wrong bsdf");
  }    

  // Find the corresponding ray that enters the material

  matchedNode = GetMatchingNode();

  if(matchedNode == NULL)
  {
    Warning("CPathNode::GetPreviousBtdf", "No corresponding entering ray");
    return(m_inBsdf);  // Should not happen
  }    

  return matchedNode->m_inBsdf;
}




void CPathNode::AssignBsdfAndNormal(void)
{
  MATERIAL *thisMaterial;

  if(m_hit.patch == NULL)
  {
    // Invalid node
    return;
  }

  thisMaterial = m_hit.patch->surface->material;

  VECTORCOPY(m_hit.normal, m_normal); // Possible double format

  // Assign bsdf's

  m_useBsdf = thisMaterial->bsdf;

  if(m_hit.flags & HIT_FRONT)
  {
    m_outBsdf = m_useBsdf; // in filled in when creating this node
  }
  else // BACK HIT
  {
    m_outBsdf = GetPreviousBsdf();
  }
}
