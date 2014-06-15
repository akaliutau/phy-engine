/* poolsP.h: pools library private declarations - include this one
 * before the public header file pools.h !!! */

#ifndef _POOLSP_H_
#define _POOLSP_H_

/* Memory cells allocated with New() will be aligned to a POOL_ALIGN
 * byte boundary. POOL_ALIGN shall be a power of 2, greater of equal to
 * the machine word size. Failing to compile with a proper value for
 * POOL_ALIGN yields segmentation faults. */
#ifndef POOL_ALIGN
#define POOL_ALIGN (sizeof(double))
#endif

/* Define the following symbol if you want new cells in pools to be cleared
 * to zero. This takes some extra computation work in NewPoolCell(), but
 * avoids problems in software that assumes that allocated memory is cleared
 * (e.g. software that assumes that uninitialised variables or fields in
 * structs contain zeroes) */
#define POOL_CLEAR_CELLS

/* Define the following symbol for lazy garbage collection: empty pool pages
 * will then only be made available to other pools when the whole pool is 
 * empty.
 * The default behaviour is to release a page as soon as it is empty.
 * The advantage of lazy garbage collection are:
 * - minimum cell size is lower: size of just 1 pointer instead of 2 pointers
 * (= size of FREE_CHAIN struct below).
 * - faster Dispose().
 * Drawback:
 * - total memory consumption of your program may be higher because empty
 * pages are not immediately re-used. */
#undef POOL_LAZY_GARBAGE_COLLECTION

/* Define the following symbol if you want the pools library to check the
 * arguments to NewPoolCell(), Dispose() and DisposeAll(). The diadvantage
 * is that it takes some additional computations to do so. */
#undef SANITY_CHECKS

/* The following symbol determines the size of pool pages. The default is to
 * use the system memory page size, as returned with the getpagesize()
 * routine (see man getpagesize). */
/* #define POOL_PAGESIZE (16*1024) */

/* Pool pages are allocated in chuncks of this many pages. This grouping
 * has two advantages:
 * - works around a malloc() limitation of Linux: such large chunks
 * of memory are allocated in a different way by the kernel, not suffering 
 * a 0.5GB storage allocation capacity limit 
 * - economical alignement of pages. */
#define PAGES_IN_MASTERBLOCK 1024

/* free cells chain link. Each unused cell in a pool starts with
 * a free cell chain link struct like this. The free list thus takes
 * no additional storage. The only drawback is that the minimum cell
 * size shall be large enough to contain such a free chain link. */
typedef struct FREE_CHAIN {
  struct FREE_CHAIN *next;
#ifndef POOL_LAZY_GARBAGE_COLLECTION  
  struct FREE_CHAIN *prev;     /* doubly linked list */
#endif
} FREE_CHAIN;

/* global parameters for a pool, shared by all the pages in the pool */
typedef struct POOL_STATE {
  unsigned magic;              /* magic number */
  unsigned nr_pages;           /* nr of pages in this pool */
  unsigned cells_per_page;     /* nr of cells per page */
  unsigned nr_used_cells;      /* nr of cells used in total */
  unsigned req_cell_size;      /* requested cell size */
  unsigned cell_size;          /* effective cell size, corrected for alignement */
  unsigned cell_align;         /* cell alignement */
  unsigned cell_offset;        /* nr of bytes in page before first cell */
  FREE_CHAIN *next_free_cell;  /* pointer to next free cell in pool */
  struct POOL *pages;          /* pointer to last page in pool */
  struct POOL_STATE *prev, *next; /* doubly linked list */
  char *name;                  /* name for the pool */
} POOL_STATE;

/* overhead per pool page in bytes */
#define PAGE_OVERHEAD(state) (global.pagesize - state->cells_per_page * state->req_cell_size)

/* pool page data structure - this is only the header. The pages themselves
 * are of equal size and alignement as the system memory pages as determined
 * with the getpagesize() C run time library routine. Their layout is
 * as follows:
 * - first this header
 * - next potentially some ununsed bytes up to the right byte boundary
 * - finally state->cells_per_page pages of state->cell_size bytes. 
 * cell_size is automatically computed to ensure proper alignement. */
typedef struct POOL {
  struct POOL_STATE *state;    /* global data for the pool */
  struct POOL *prev, *next;    /* pages form a doubly linked list */
  unsigned cells_used_this_page; /* cells used in this page */
} POOL;

/* global variables for the pools library */
typedef struct POOL_GLOBALS {
  int  inited;               /* TRUE if library has been initialised */
  POOL_STATE *pools;         /* pointer to registered pools */
  unsigned magic1, magic2;   /* magic numbers */
  unsigned long bytesAllocated, /* total bytes allocated */
    bytesOverhead;           /* total bytes overhead */
  unsigned pagesize;         /* system memory page size */
  unsigned align;            /* default cell alignement in bytes */
  unsigned char *masterblock, /* current group of PAGES_IN_MASTERBLOCK 
			      * pages to allocate new pages from */
    *nextpagetotake,         /* next page to take in the current master
			      * block */
    *nextfreepage;           /* next page to take from the free list
			      * (if there are pages to be recycled) */
  unsigned nrmasterblocks;   /* nr of allocated master blocks */
} POOL_GLOBALS;

/* Each masterblock contains this many additional bytes besides the pages 
 * in order to do alignement: alignement is obtained by rounding up the
 * masterblock adres returned by Alloc() to the nearest global.pagesize
 * byte boundary. */
#define MASTERBLOCKOVERHEAD (global.pagesize-1)

/* Total size of each masterblock, includes the pages + the overhead */
#define MASTERBLOCKSIZE (PAGES_IN_MASTERBLOCK*global.pagesize + MASTERBLOCKOVERHEAD)

/* Rounds up n to the next integer multiple of size */
#define ROUND_UP(n, size)  (((n + ((size)-1))/size)*size)

/* Rounds up ptr to the next integer multiple of 'align'. 'align' needs 
 * to be a power of 2. */
#define ALIGN_UP(ptr, align)  (((unsigned long)ptr + ((align)-1)) & ~((align)-1))
/* same, but rounds down */
#define ALIGN_DOWN(ptr, align) (((unsigned long)ptr) & ~((align)-1))

/* macros to compute the adress of the first, last, next and previous 
 * cell in a page. These macros do no boundary checking! */
#define FIRST_CELL_IN_PAGE(page)  ((unsigned char *)(page) + (page)->state->cell_offset)
#define LAST_CELL_IN_PAGE(page)   ((unsigned char *)FIRST_CELL_IN_PAGE(page) + ((page)->state->cells_in_page-1)*(page)->state->cell_size)
#define PREVIOUS_CELL_IN_PAGE(p, page)  ((unsigned char *)p - (page)->state->cell_size)
#define NEXT_CELL_IN_PAGE(p, page)  ((unsigned char *)p + (page)->state->cell_size)

/* iterates over all cells in a page */
#define ForAllCellsInPage(cell, page) { \
  int _i_; \
  unsigned char *_p_; \
  for (_i_=0, _p_ = FIRST_CELL_IN_PAGE(page); _i_<(page)->state->cells_per_page; _i_++, _p_ += (page)->state->cell_size) { \
    void *cell = _p_; {

#ifndef EndForAll
#define EndForAll }}}
#endif

/* checks whether page points to a valid POOL page by inspecting the magic
 * number of the pool state. 'page' shall not be a null pointer! */
#define VALID_POOL(page)   ((page)->state && (page)->state->magic == global.magic1)

#endif /*_POOLSP_H_*/
