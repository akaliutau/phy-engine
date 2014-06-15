

#include "kdtree.H"
#include "vector.h"
#include "error.h"

// Median finding




static inline void bkdswap(Cbkdnode root[], int a,int b) 
{ 
  Cbkdnode tmp = root[a];
  root[a] = root[b];
  root[b]= tmp;
}

static inline float bkdval(Cbkdnode root[], int index, int discr)
{
  return(((float *)root[index].m_data)[discr]);
}

// Shorthand notations

#define E_SWAP(a,b) bkdswap(broot, (a), (b))
#define E_VAL(index) bkdval(broot, (index), discr)

static int s_num;

int GetBalancedMedian(int low, int high)
{
  int N = high - low + 1;  // high inclusive

  if(N <= 1) return low;

  int FL;
  // Add 0.1 because integer powers of 2 sometimes gave a smaller FL (8 -> 2)
  FL = (int)(log(N + 0.1)/M_LN2); // frexp((double)N, &FL);
  //FL++; // Filled levels

  int P2FL = (1<<FL); // 2^FL
  int LASTN = N - (P2FL - 1);  // Number of elements on last level
  int LASTS_2 = P2FL/2; // Half the room for elements on last level
  int left;

  if(LASTN < LASTS_2)
  {
    // All in left subtree
    left = LASTN + (LASTS_2 - 1);
    //right = LASTS_2 - 1;
  }
  else
  {
    // Full bottom level in left subtree
    left = LASTS_2 + LASTS_2 -1;
    // Rest in right subtree
    //right = LASTN - LASTS_2 + LASTS_2 - 1;
  }

  return low + left; //+1;
}

// quick_select: return index of median element
static int quick_select(Cbkdnode broot[], int low, int high, int discr) 
{
  // int low, high ;
  int median;
  int middle, ll, hh;
  
   
  median = GetBalancedMedian(low, high);

  for (;;) 
  {
    if (high <= low) 
    {
      // fprintf(stderr, "Median %i\n", median);
      return median;
    }
    
    if (high == low + 1) 
    {  
      if (E_VAL(low) > E_VAL(high))
	E_SWAP(low, high) ;
      return median;
    }
    
    
    middle = (low + high + 1) / 2;
    if (E_VAL(middle) > E_VAL(high))    E_SWAP(middle, high) ;
    if (E_VAL(low) > E_VAL(high))       E_SWAP(low, high) ;
    if (E_VAL(middle) > E_VAL(low))     E_SWAP(middle, low) ;
    
    
    E_SWAP(middle, low+1) ;
    
    
    ll = low + 1;
    hh = high;
    for (;;) 
    {
      do ll++; while (E_VAL(low) > E_VAL(ll)) ;
      do hh--; while (E_VAL(hh)  > E_VAL(low)) ;
      
      if (hh < ll)
        break;
      
      E_SWAP(ll, hh) ;
    }

    
    E_SWAP(low, hh) ;
    
    
    if (hh <= median)
      low = ll;
    if (hh >= median)
      high = hh - 1;
  }
}


//////// Balance the kdtree

static void CopyUnbalanced_rec(Ckdnode* node, Cbkdnode* broot, int *pindex)
{
  if(node)
  {
    broot[(*pindex)++].Copy(*node);
    CopyUnbalanced_rec(node->loson, broot, pindex);
    CopyUnbalanced_rec(node->hison, broot, pindex);
  }
}

static int BestDiscriminator(Cbkdnode broot[], int low, int high)
{
  float bmin[3] = {HUGE,HUGE,HUGE};
  float bmax[3] = {-HUGE,-HUGE,-HUGE};
  float tmp;

  for(int i = low; i <= high; i++)
  {
    tmp = bkdval(broot, i, 0);
    if(bmin[0] > tmp) bmin[0] = tmp;
    if(bmax[0] < tmp) bmax[0] = tmp;

    tmp = bkdval(broot, i, 1);
    if(bmin[1] > tmp) bmin[1] = tmp;
    if(bmax[1] < tmp) bmax[1] = tmp;

    tmp = bkdval(broot, i, 2);
    if(bmin[2] > tmp) bmin[2] = tmp;
    if(bmax[2] < tmp) bmax[2] = tmp;
  }

  int discr = 0;
  float spread = bmax[0] - bmin[0]; // X spread

  tmp = bmax[1] - bmin[1];
  if(tmp > spread)
  {
    discr = 1;
    spread = tmp;
  }

  tmp = bmax[2] - bmin[2];
  if(tmp > spread)
  {
    discr = 2;
    spread = tmp;
  }

  // fprintf(stderr, "Best %i\n", discr);
  return discr;
}


// Balance the tree recursively
void Ckdtree::Balance_rec(Cbkdnode broot[], Cbkdnode dest[], int destIndex,
			  int low, int high)  // High inclusive!
{
  if(low == high)
  {
    //put it in dest
    dest[destIndex] = broot[low];
    dest[destIndex].SetDiscriminator(0); // don't care...
    return;
  }

  int discr = BestDiscriminator(broot, low, high);
  // find the balance median element
  int median = quick_select(broot, low, high, discr);

  //put it in dest
  dest[destIndex] = broot[median];
  dest[destIndex].SetDiscriminator(discr);


#ifdef NEVER
  // Check
  int i;
  int lower=0, higher=0;
  for(i = low; i < median; i++)
  {
    if(E_VAL(i) < E_VAL(median))
      lower++;
    else
      higher++;
  }
  fprintf(stderr, "lower %i, higher %i\n", lower, higher);

  lower=0; higher=0;
  for(i = median+1; i <= high; i++)
  {
    if(E_VAL(i) < E_VAL(median))
      lower++;
    else
      higher++;
  }
  fprintf(stderr, "lower %i, higher %i\n", lower, higher);
#endif

  // Recurse low and high array

  if(low < median)
    Balance_rec(broot, dest, (destIndex<<1)+1, low, median-1);  // High inclusive!  
  if(high > median)
    Balance_rec(broot, dest, (destIndex<<1)+2, median+1, high);  // High inclusive!  
}

void Ckdtree::Balance(void)
{
  // Make an unsorted Cbkdnode array pointing to the nodes

  if(m_numUnbalanced == 0)
    return; // No balancing needed.

  fprintf(stderr, "Balancing kd-tree: %i nodes...\n", m_numNodes);

  Cbkdnode *broot = new Cbkdnode[m_numNodes+1];

  broot[m_numNodes].m_data = NULL;
  broot[m_numNodes].m_flags = 128;

  int i, index = 0;

  // Copy balanced

  for(i = 0; i < m_numBalanced; i++)
  {
    broot[index++] = m_broot[i];
  }

  // fprintf(stderr, "After balanced copy: index %i\n", index);

  // Copy unbalanced

  CopyUnbalanced_rec(m_root, broot, &index);
  
  // fprintf(stderr, "After unbalanced copy: index %i\n", index);

  // Clear old balanced and unbalanced part (but no data delete)

  DeleteNodes(m_root, false);
  m_root = NULL;
  m_numUnbalanced = 0;

  DeleteBNodes(false);

  m_numBalanced = m_numNodes;
  Cbkdnode *dest = new Cbkdnode[m_numNodes+1]; // Could we do with just 1 array???

  dest[m_numNodes].m_data = NULL;
  dest[m_numNodes].m_flags = 64;

  s_num = m_numNodes;

  // Now balance the tree recursively
  Balance_rec(broot, dest, 0, 0, m_numNodes-1);  // High inclusive!

  m_broot = dest;
  delete[] broot;

  m_firstLeaf = (m_numBalanced+1)/2;

  fprintf(stderr, "done\n");
}


#ifdef NEVER
// Original Quick select taken from http://www.eso.org/~ndevilla/median/




#define ELEM_SWAP(a,b) { register elem_type t=(a);(a)=(b);(b)=t; }

elem_type quick_select(elem_type arr[], int n) 
{
    int low, high ;
    int median;
    int middle, ll, hh;

    low = 0 ; high = n-1 ; median = (low + high) / 2;
    for (;;) {
        if (high <= low) 
            return arr[median] ;

        if (high == low + 1) {  
            if (arr[low] > arr[high])
                ELEM_SWAP(arr[low], arr[high]) ;
            return arr[median] ;
        }

    
    middle = (low + high) / 2;
    if (arr[middle] > arr[high])    ELEM_SWAP(arr[middle], arr[high]) ;
    if (arr[low] > arr[high])       ELEM_SWAP(arr[low], arr[high]) ;
    if (arr[middle] > arr[low])     ELEM_SWAP(arr[middle], arr[low]) ;

    
    ELEM_SWAP(arr[middle], arr[low+1]) ;

    
    ll = low + 1;
    hh = high;
    for (;;) {
        do ll++; while (arr[low] > arr[ll]) ;
        do hh--; while (arr[hh]  > arr[low]) ;

        if (hh < ll)
        break;

        ELEM_SWAP(arr[ll], arr[hh]) ;
    }

    
    ELEM_SWAP(arr[low], arr[hh]) ;

    
    if (hh <= median)
        low = ll;
        if (hh >= median)
        high = hh - 1;
    }
}

#undef ELEM_SWAP

#endif
