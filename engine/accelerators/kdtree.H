/* KD Tree
 *
 *
 * Ref : - Bentley, J.L. (1975) Multidimensional search trees used for 
 *         associative searching. Comm. of the ACM 18(9) p. 509-517
 *       - Friedman et al. (1977) An algorithm for finding best matches
 *         in logarithmic expected time. ACM Trans. on Math. Software 3(3)
 *         p. 209-226
 */


#ifndef _KDTREE_H_
#define _KDTREE_H_

#include <math.h>
#include "../pools/pools.h"
#include "../math/Float.h"


/*
** KD node
*/

const float KDMAXRADIUS = 1e10; // Not HUGE, since we need to square it.

class Ckdnode
{
public:
  Ckdnode *loson, *hison;

  // Flags can be used to exclude certaint nodes from a query
  // 4 LOWER BITS ARE RESERVED !!
  short m_flags;

  void *m_data;

  void* operator new(size_t size);
  void operator delete(void* ptr, size_t size);
  void FindMinMaxDepth(int depth, int *minDepth, int *maxDepth);

  inline int Discriminator() const { return (m_flags & 0xF); }
  inline void SetDiscriminator(int discr) 
    { m_flags = (m_flags & 0xFFF0)|(short)discr; }
  inline int Flags() const { return (m_flags & (0xFFF0)); }
protected:
  static POOL *Ckdnode_Pool;
};

// Node for a balanced kdtree, this nodes are placed in arrays
// and no loson, hison pointers are necessary
class Cbkdnode
{
public:
  void *m_data;
  short m_flags;

  // void FindMinMaxDepth(int depth, int *minDepth, int *maxDepth);

  inline void Copy(const Ckdnode &kdnode) 
  { m_data = kdnode.m_data; m_flags = kdnode.Flags(); }
  inline int Discriminator() const { return (m_flags & 0xF); }
  inline void SetDiscriminator(int discr) 
    { m_flags = (m_flags & 0xFFF0)|(short)discr; }
  inline int Flags() const { return (m_flags & (0xFFF0)); }
};



class Ckdtree
{
public:
  int m_dimension; // Should be 3!
protected:
  int m_numNodes;
  long m_dataSize;

  int m_numUnbalanced;
  Ckdnode *m_root;  // Start of non balanced part of the kdtree

  int m_numBalanced;
  int m_firstLeaf; // (numBalanced+1) / 2 : index of first leaf element
  Cbkdnode *m_broot; // Start of balanced part of the kdtree

  bool m_CopyData;
  POOL *m_dataPool;

  static float *s_distances;

  // Member functions
public:
  Ckdtree( int dimension, int dataSize, bool CopyData = true );
  virtual ~Ckdtree( void );

  // Add a point in the kd tree, this is always to the unbalanced part
  virtual void AddPoint(void *data, short flags = 0);
  virtual int Query(const float *point, int N, void *results, 
		    float *distances = NULL, float radius = KDMAXRADIUS,
		    short excludeFlags = 0);

  // Iterate nodes : iterate all nodes (only for balanced ?!)
  virtual void IterateNodes(void (*cb)(void *, void *), void *data);

  // Analyses how balanced the tree is, and prints a report to stdout
  virtual void BalanceAnalysis();

  // Balance the tree, it is possible that a part is already balanced!!
  virtual void Balance();
  virtual void Balance_rec(Cbkdnode broot[], Cbkdnode dest[], int destIndex,
			   int low, int high);
private:
  void *AssignData(void *data);
protected:
  void DeleteNodes(Ckdnode *node, bool deleteData);
  void DeleteBNodes(bool deleteData);
  void FindMinMaxDepth(int *minDepth, int *maxDepth); // Unbalanced part!
  void Query_rec(const Ckdnode *node); // Unbalanced part
  void BQuery_rec(const int node); // Balanced part

  virtual float Distance(const float *a, const float *b);
};


#endif
