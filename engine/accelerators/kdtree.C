// kdtree.cpp

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "kdtree.H"
#include "../error.h"
#include "Float.h"
#include "pools.h"



// KD Tree with one data element per node

float *Ckdtree::s_distances = NULL;


Ckdtree::Ckdtree( int dimension, int dataSize, bool CopyData )
{
  m_dimension = dimension;
  m_dataSize = dataSize;
  m_numNodes = 0;

  m_numUnbalanced = 0;
  m_root = NULL;

  m_numBalanced = 0;
  m_broot = NULL;

  m_dataPool = NULL;
  m_CopyData = CopyData;

  if(s_distances == NULL) s_distances = new float[1000]; // max 1000 !!
}


void Ckdtree::DeleteNodes(Ckdnode *node, bool deleteData)
{
  if(node == NULL) return;

  DeleteNodes(node->loson, deleteData);
  DeleteNodes(node->hison, deleteData);

  if(deleteData)
  {
    Dispose(node->m_data, &m_dataPool);
  }

  delete node;
}

void Ckdtree::DeleteBNodes(bool deleteData)
{
  if(m_broot == NULL) return;

  if(deleteData)
  {
    for(int node = 0; node < m_numBalanced; node++)
    {
      Dispose(m_broot[node].m_data, &m_dataPool);
    }
  }

  delete[] m_broot;
  m_broot = NULL;
}


Ckdtree::~Ckdtree(void)
{
  // Delete tree 

  // printf("Delete treeeeeeee\n");
  DeleteNodes(m_root, m_CopyData);
  m_root = NULL;
  DeleteBNodes(m_CopyData);
}

void Ckdtree::IterateNodes(void (*cb)(void *, void *), void *data)
{
  if(m_numUnbalanced > 0)
  {
    Error(" Ckdtree::IterateNodes", "Cannot iterate unbalanced trees");
    return;
  }

  for(int i = 0; i < m_numBalanced; i++)  // m_numBalanced == m_numNodes
  {
    cb(data, m_broot[i].m_data);
  }
}


////////////////////////////////////////////////////

void Ckdtree::AddPoint(void *data, short flags)
{
  // Add the point to the unbalanced part

  Ckdnode **nodePtr, *newNode, *parent;
  float *newPoint;
  int discr = 0;

  newNode = new Ckdnode;

  newNode->m_data = AssignData(data);
  newNode->m_flags = flags;
  newNode->SetDiscriminator(0); // Default

  newNode->loson = NULL;
  newNode->hison = NULL;

  newPoint = (float *)data;
  m_numNodes++;
  m_numUnbalanced++;

  nodePtr = &m_root;
  parent = NULL;

  while(*nodePtr != NULL)
  {
    parent = *nodePtr;  // The parent

    discr = parent->Discriminator();

    

    if( newPoint[discr] <= ((float *)(parent->m_data))[discr] )
      nodePtr = &(parent->loson);
    else
      nodePtr = &(parent->hison);
  }


  if((parent != NULL) && (parent->loson == NULL) && (parent->hison == NULL))
  {
    // Choose an appropriate discriminator for the parent
    float dx,dy,dz;
    float *pdata = (float *)(parent->m_data);

    dx = fabs(newPoint[0] - pdata[0]);
    dy = fabs(newPoint[1] - pdata[1]);
    dz = fabs(newPoint[2] - pdata[2]);

    if(dx > dy)
    {
      if(dx > dz) discr = 0;
      else discr = 2;
    }
    else
    {
      if(dy > dz) discr = 1;
      else discr = 2;
    }

    parent->SetDiscriminator(discr);
    // printf("Chose discr %i\n", discr);

    // Choose correct side
    if(newPoint[discr] <= pdata[discr])
      parent->loson = newNode;
    else
      parent->hison = newNode;
  }
  else
  {
    // Parent is NULL or discriminator is fixed...
    *nodePtr = newNode;
  }
}


// Query

class Ckdquery
{
public:
  float *point;
  int Nwanted;
  int Nfound;
  bool notfilled;
  float **results;
  float *distances;
  float maxdist;
  int maxpos;
  float radius;
  float sqrRadius;
  short excludeFlags;

  void Print(void)
    {
      printf("Point X %g, Y %g, Z %g\n", point[0], point[1], point[2]);
      printf("Nwanted %i, Nfound %i\n", Nwanted, Nfound);
      printf("maxdist %g\n", maxdist);
      printf("sqrRadius %g\n", sqrRadius);
      printf("excludeFlags %x\n", (int)excludeFlags);
    }
      
};  

static Ckdquery qdat_s;


int Ckdtree::Query(const float *point, int N, void *results,
		   float *distances, float radius, short excludeFlags)
{
  int numberFound;
  float *used_distances;
  // float maxdist = HUGE;

  if(distances == NULL)
  {
    if(N > 1000) 
    {
      Error("Ckdtree::Query", "Too many nodes requested");
      return 0;
    }
    used_distances = s_distances;
  }
  else
    used_distances = distances;

  // Fill in static class data

  qdat_s.point = (float *)point;
  qdat_s.Nwanted = N;
  qdat_s.Nfound = 0;
  qdat_s.results = (float **)results;
  qdat_s.distances = used_distances;
  qdat_s.maxdist = radius; // maxdist;
  qdat_s.maxpos = 0;  // Actually it is undefined
  qdat_s.radius = radius;
  qdat_s.sqrRadius = radius; // * radius;
  qdat_s.excludeFlags = excludeFlags;
  qdat_s.notfilled = true;
  //  qdat_s.Print();

  // First query balanced part
  if(m_broot)
    BQuery_rec(0);  

  // Now query unbalanced part using the already found nodes 
  // from the balanced part
  if(m_root)
    Query_rec(m_root);  

  numberFound = qdat_s.Nfound;

  return(numberFound);
}


//////////// Ckdtree - private methods

// Assign Data

void *Ckdtree::AssignData(void *data)
{
  if(m_CopyData)
  {
    void *newData;

    newData = NewPoolCell(m_dataSize, 0, "kdtree data", &m_dataPool);
    memcpy((char *)newData, (char *)data, m_dataSize);
    return newData;
  }
  else
  {
    return data;
  }
}



////////////////// Protected methods //


// Distance calculation
inline static float SqrDistance3D(float *a, float *b)
{
  float result,tmp;

  tmp = *(a++) - *(b++);
  result = tmp*tmp;
  //  result = *ptmp++

  tmp = *(a++) - *(b++);
  result += tmp*tmp;

  tmp = *a - *b;
  result += tmp*tmp;

  return result;
}


#ifdef NEVER
static float SqrDistance3D(float *a, float *b, float &planedist,
			   const int discr)
{
  static float distarr_s[3];
  float result, *ptmp = distarr_s, tmp;

  tmp = *a++ - *b++;
  result = *ptmp++ = tmp*tmp;
  //  result = *ptmp++

  tmp = *a++ - *b++;
  *ptmp++ = tmp*tmp;
  result += *ptmp++;

  tmp = *a - *b;
  *ptmp = tmp*tmp;
  result += *ptmp;

  planedist = distarr_s[discr];

  return result;
}
#endif

// Max heap stuff, using static qdat_s  (fastest !!)
// Adapted from patched POVRAY (megasrc), who took it from Sejwick
inline static void fixUp()
{
  // Ripple the node (qdat_s.Nfound) upward. There are qdat_s.Nfound + 1 nodes
  // in the tree

  int son,parent;
  float tmpDist;
  float *tmpData;

  son = qdat_s.Nfound;
  parent = (son-1)>>1;  // Root of tree == index 0 so parent = any son - 1 / 2

  while((son > 0) && qdat_s.distances[parent] < qdat_s.distances[son])
  {
    tmpDist = qdat_s.distances[parent];
    tmpData = qdat_s.results[parent];

    qdat_s.distances[parent] = qdat_s.distances[son];
    qdat_s.results[parent] = qdat_s.results[son];

    qdat_s.distances[son] = tmpDist;
    qdat_s.results[son] = tmpData;

    son = parent;
    parent = (son-1)>>1;
  }    
}

inline static void MHInsert(float *data, float dist)
{
  qdat_s.distances[qdat_s.Nfound] = dist;
  qdat_s.results[qdat_s.Nfound] = data;

  fixUp();

  // If all the photons are filled, we can use the actual maximum distance
  if(++qdat_s.Nfound == qdat_s.Nwanted)
  {
    qdat_s.maxdist = qdat_s.distances[0];
    qdat_s.notfilled = false;
  }

  //  printf("El0 %g, Max %g\n", qdat_s.distances[0], qdat_s.maxdist);
}


inline static void fixDown()
{
  // Ripple the top node, which may not be max anymore downwards
  // There are qdat_s.Nfound nodes in the tree, starting at index 0

  int son,parent, max;
  float tmpDist;
  float *tmpData;

  max = qdat_s.Nfound;

  parent = 0;
  son = 1;

  while(son < max)
  {
    if(qdat_s.distances[son] <= qdat_s.distances[parent])
    {
      if((++son >= max) || qdat_s.distances[son] <= qdat_s.distances[parent])
      {
	return; // Node in place, left son and right son smaller
      }
    }
    else
    {
      if((son+1<max) && qdat_s.distances[son + 1] > qdat_s.distances[son])
      {
	son++; // Take maximum of the two sons
      }
    }

    // Swap because son > parent

    tmpDist = qdat_s.distances[parent];
    tmpData = qdat_s.results[parent];
    
    qdat_s.distances[parent] = qdat_s.distances[son];
    qdat_s.results[parent] = qdat_s.results[son];
    
    qdat_s.distances[son] = tmpDist;
    qdat_s.results[son] = tmpData;
    
    parent = son;
    son = (parent<<1) + 1;
  }  
}


inline static void MHReplaceMax(float *data, float dist)
{
  // Top = maximum element. Replace it with new and ripple down
  // The heap is full (Nfound == Nwanted), but this is not required

  *qdat_s.distances = dist; // Top
  *qdat_s.results = data;

  fixDown();

  qdat_s.maxdist = *qdat_s.distances; // Max = top of heap
}




// Query_rec for the unbalanced kdtree part

void Ckdtree::Query_rec(const Ckdnode *node)
{
  //if(!node)
  //  return;

  int discr = node->Discriminator();

  float dist;
  Ckdnode *nearNode, *farNode;

  // if(!(node->m_flags & qdat_s.excludeFlags))
  {
    dist = SqrDistance3D((float *)node->m_data, qdat_s.point);

    //printf("Q dist %g , dat %g %g %g\n", dist, ((float *)node->m_data)[0],
    //   ((float *)node->m_data)[1], ((float *)node->m_data)[2]);

    //printf("Q maxdist %g ", qdat_s.maxdist);
    
    if(dist < qdat_s.maxdist)
    {      
      if(qdat_s.notfilled) // Nfound < qdat_s.Nwanted)
      {
	// Add this point anyway, because we haven't got enough points yet.
	// We have to check for the radius only here, since if N points
	// are added, maxdist <= radius
	
	//if(dist < qdat_s.sqrRadius)      
	//      {
	MHInsert((float *)node->m_data, dist);
      }
      else
      {
	// Add point if distance < maxdist
      
	//if(dist < qdat_s.maxdist)
	//{
	// Add point in results
	
	//for(int i = 0; i<N && !FLOATEQUAL(distances[i], *maxdist, EPSILON); i++);
	//Assert(i<N, "Maxdist does not exist in distances");

	MHReplaceMax((float *)node->m_data, dist);
      }
    } // if(dist < maxdist)

    // printf("nmaxdist %g\n", qdat_s.maxdist);
  }
    
  // Recursive call to the child nodes
   
  // Test discr

  // reuse dist

  dist = ((float *)node->m_data)[discr] - qdat_s.point[discr];

  if( dist >= 0.0 )  // qdat_s.point[discr] <= ((float *)node->m_data)[discr]
  {
    nearNode = node->loson;
    farNode = node->hison;
  }
  else
  {
    nearNode = node->hison;
    farNode = node->loson;
  }


  // Always call near node recursively

  //  int newdiscr = (discr + 1) % m_dimension;

  if(nearNode)
  {
    Query_rec(nearNode);
  }

  dist *= dist; // Square distance to the separator plane
  if((farNode) && (((qdat_s.Nfound < qdat_s.Nwanted) && 
		    (dist < qdat_s.sqrRadius)) ||
		   (dist < qdat_s.maxdist)) )
  {
    // Discriminator line closer than maxdist : nearer points can lie
    // on the far side. Or there are still not enough nodes found

    Query_rec(farNode);
  }
}


///// Balanced KDTREE QUERY

// Query_rec for the unbalanced kdtree part


void Ckdtree::BQuery_rec(const int index)
{
  const Cbkdnode &node=m_broot[index];
  int discr = node.Discriminator();

  float dist;

  int nearIndex, farIndex;
    
  // Recursive call to the child nodes
   
  // Test discr (reuse dist)

  if(index < m_firstLeaf)
  {
    dist = ((float *)node.m_data)[discr] - qdat_s.point[discr];
    
    if( dist >= 0.0 )  // qdat_s.point[discr] <= ((float *)node->m_data)[discr]
    {
      nearIndex = (index << 1) + 1; //node->loson;
      farIndex = nearIndex+1; //node->hison;
    }
    else
    {
      farIndex = (index << 1) + 1;//node->loson;
      nearIndex = farIndex+1;//node->hison;
    }

    // Always call near node recursively
    
    if(nearIndex < m_numBalanced)
    {
      BQuery_rec(nearIndex);
    }
    
    dist *= dist; // Square distance to the separator plane
    if((farIndex < m_numBalanced) && (((qdat_s.notfilled) && // qdat_s.Nfound < qdat_s.Nwanted
		      (dist < qdat_s.sqrRadius)) ||
		     (dist < qdat_s.maxdist)) )
    {
      // Discriminator line closer than maxdist : nearer points can lie
      // on the far side. Or there are still not enough nodes found
      BQuery_rec(farIndex);
    }
  }

  // if(!(node->m_flags & qdat_s.excludeFlags))
  {
    dist = SqrDistance3D((float *)node.m_data, qdat_s.point);

    if(dist < qdat_s.maxdist)
    {      
      if(qdat_s.notfilled) // Nfound < qdat_s.Nwanted)
      {
	// Add this point anyway, because we haven't got enough points yet.
	// We have to check for the radius only here, since if N points
	// are added, maxdist <= radius

	MHInsert((float *)node.m_data, dist);
      }
      else
      {
	// Add point if distance < maxdist
	MHReplaceMax((float *)node.m_data, dist);
      }
    } // if(dist < maxdist)
  }

}

#ifdef NEVER   // Working version, balanced recursive
void Ckdtree::BQuery_rec(const int index)
{
  //if(!node)
  //  return;

  Cbkdnode &node=m_broot[index];
  int discr = node.Discriminator();

  float dist;

  int nearIndex, farIndex;
  // Ckdnode *nearNode, *farNode;

  // if(!(node->m_flags & qdat_s.excludeFlags))
  {
    dist = SqrDistance3D((float *)node.m_data, qdat_s.point);

    if(dist < qdat_s.maxdist)
    {      
      if(qdat_s.notfilled) // Nfound < qdat_s.Nwanted)
      {
		MHInsert((float *)node.m_data, dist);
      }
      else
      {

		MHReplaceMax((float *)node.m_data, dist);
      }
    } // if(dist < maxdist)

   }
    
  // Recursive call to the child nodes
   
 
  if(index < m_firstLeaf)
  {
    dist = ((float *)node.m_data)[discr] - qdat_s.point[discr];
    
    if( dist >= 0.0 )  // qdat_s.point[discr] <= ((float *)node->m_data)[discr]
    {
      nearIndex = (index << 1) + 1; //node->loson;
      farIndex = nearIndex+1; //node->hison;
    }
    else
    {
      farIndex = (index << 1) + 1;//node->loson;
      nearIndex = farIndex+1;//node->hison;
    }

    // Always call near node recursively
    
    //  int newdiscr = (discr + 1) % m_dimension;
    
    if(nearIndex < m_numBalanced)
    {
      BQuery_rec(nearIndex);
    }
    
    dist *= dist; // Square distance to the separator plane
    if((farIndex < m_numBalanced) && (((qdat_s.Nfound < qdat_s.Nwanted) && 
		      (dist < qdat_s.sqrRadius)) ||
		     (dist < qdat_s.maxdist)) )
    {
      
      BQuery_rec(farIndex);
    }
  }
}
#endif

////////////////////// OLD ////////////////////////

#ifdef NEVER
int Ckdtree::Query_rec_old(const Ckdnode *node, const float *point, int N,
		       int currentN, float **results, float *distances,
		       float *maxdist, int discr, float radius,
		       short excludeFlags)
{
  float dist;
  int i;
  Ckdnode *nearNode, *farNode;

  if(node == NULL)
    return(currentN);

  if(!(node->m_flags & excludeFlags))
  {
    dist = Distance((float *)node->m_data, point);
    
    if(currentN < N)
    {
      // Add this point anyway, because we haven't got enough points yet.
      // We have to check for the radius only here, since if N points
      // are added, maxdist <= radius
      
      
      if(dist <= radius)
      {
	results[currentN] = (float *)node->m_data;
	distances[currentN] = dist;
	
	// Maximum distance can increase when adding the first N points
	
	*maxdist = MAX(*maxdist, dist);
	
	currentN++;
      }
    }
    else
    {
      // Add point if distance < maxdist
      
      if(dist < *maxdist)
      {
	// Add point in results
	
	for(i = 0; i<N && !FLOATEQUAL(distances[i], *maxdist, EPSILON); i++);
	
	Assert(i<N, "Maxdist does not exist in distances");
	
	distances[i] = dist;
	results[i] = (float *)node->m_data;
	
	// Update maxdist
	
	*maxdist = 0;
	
	for(i = 0; i < N; i++)
	{
	  *maxdist = MAX(*maxdist, distances[i]);
	}
      }
    }
  }
    
  // Recursive call to the child nodes
   
  // Test discr

  if( point[discr] <= ((float *)node->m_data)[discr] )
  {
    nearNode = node->loson;
    farNode = node->hison;
  }
  else
  {
    nearNode = node->hison;
    farNode = node->loson;
  }


  // Always call near node recursively

  if(nearNode != NULL)
  {
    currentN = Query_rec(nearNode,
			 point, N, currentN,
			 results, distances, maxdist,
			 (discr + 1) % m_dimension, radius,
			 excludeFlags);
  }

  if(farNode != NULL && 
     fabs(((float *)node->m_data)[discr] - point[discr]) < *maxdist)
  {
    // Discriminator line closer than maxdist : nearer points can lie
    // on the far side

    currentN = Query_rec(farNode,
			 point, N, currentN,
			 results, distances, maxdist,
			 (discr + 1) % m_dimension, radius,
			 excludeFlags);
  }

  return(currentN);
}
#endif

// Slow Distance calculation

float Ckdtree::Distance(const float *a, const float *b)
{
  double result = 0;
  int i;

  for(i=0; i < m_dimension; i++)
    result += pow(a[i] - b[i], 2);

  return (float)sqrt(result);
}






////////////////////////////// NODE //////////////////////////

POOL *Ckdnode::Ckdnode_Pool = NULL;

void *Ckdnode::operator new(size_t size)
{
  if(size == sizeof(Ckdnode))
  {
    return NewPoolCell(size, 0, "kdtree nodes", &Ckdnode_Pool);
  }


  // else
  return new char[size];
}

void Ckdnode::operator delete(void *ptr, size_t size)
{
  if(size == sizeof(Ckdnode))
  {
    Dispose((char *)ptr, &Ckdnode_Pool);
  }
  else delete[] (char *)ptr;
}


void Ckdnode::FindMinMaxDepth(int depth, int *minDepth, int *maxDepth)
{
  if((loson == NULL) && (hison == NULL))
  {
    *maxDepth = MAX(*maxDepth, depth);
    *minDepth = MIN(*minDepth, depth);
    //    printf("Min %i Max %i\n", *minDepth, *maxDepth);
  }
  else
  {
    if(loson) loson->FindMinMaxDepth(depth + 1, minDepth, maxDepth);
    if(hison) hison->FindMinMaxDepth(depth + 1, minDepth, maxDepth);
  }     
}

//** KD TREE addititional routines and old stuff

void Ckdtree::FindMinMaxDepth(int *minDepth, int *maxDepth)
{
  *minDepth = m_numNodes + 1;
  *maxDepth = 0;

  if(m_root)
    m_root->FindMinMaxDepth(1, minDepth, maxDepth);
  else
  {
    *minDepth = 0;
    *maxDepth = 0;
  }
}

void Ckdtree::BalanceAnalysis()
{
  int deepest, shortest;

  printf("KD Tree Analysis\n====================\n");

  FindMinMaxDepth(&shortest, &deepest);

  printf("Numnodes %i, Shortest %i, Deepest %i\n", m_numNodes, shortest, 
	 deepest);
}

#ifdef NEVER
void Ckdtree::OldAddPoint(void *data, short flags)
{
  // Add the point

  Ckdnode **nodePtr, *newNode, *tmpNode;
  float *newPoint;
  int discr = 0;

  newNode = new Ckdnode;

  newNode->m_data = AssignData(data);
  newNode->m_flags = flags;

  newPoint = (float *)data;
  nodePtr = &m_root;

  while(*nodePtr != NULL)
  {
    tmpNode = *nodePtr;

    /* Test discriminator */

    if( newPoint[discr] <= ((float *)(tmpNode->m_data))[discr] )
      nodePtr = &(tmpNode->loson);
    else
      nodePtr = &(tmpNode->hison);

    discr = (discr + 1) % (m_dimension);
  }

  m_numNodes++;
  newNode->SetDiscriminator(discr);

  *nodePtr = newNode;
}
#endif
