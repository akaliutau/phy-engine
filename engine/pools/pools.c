

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "poolsP.h"
#include "pools.h"
#include "error.h"

int pools_debug = 0;
#ifdef DEBUG
#define DPRINTF 	if (pools_debug) printf
#endif





static unsigned GetMagic(int which)
{
  static char magic[2][4] = {
    {'P', 'o', 'E', 'l'},
    {'p', 'O', 'e', 'L'}
  };
  union {char c[4]; unsigned i;} magic_union;

  which = which-1;
  magic_union.i = 0;
  magic_union.c[0] = magic[which][0];
  magic_union.c[1] = magic[which][1];
  magic_union.c[2] = magic[which][2];
  magic_union.c[3] = magic[which][3];
  return magic_union.i;
}


static unsigned MakePower2(unsigned n)
{
  unsigned i = 1;
  while (i<n)
    i <<= 1;
  return i;
}

static POOL_GLOBALS global = { FALSE, NULL, 0, 0, 0, 0, 0, POOL_ALIGN, NULL, NULL, NULL, 0 };


#ifndef POOL_PAGESIZE
#define POOL_PAGESIZE getpagesize()
#endif

static void PoolsInit(void)
{
  if (!global.inited) {
    global.magic1 = GetMagic(1);
    global.magic2 = GetMagic(2);
    global.inited = TRUE;
    global.bytesOverhead = sizeof(POOL_GLOBALS);
    global.bytesAllocated = sizeof(POOL_GLOBALS);

    
    global.pagesize = MakePower2(POOL_PAGESIZE);
    if (global.pagesize != POOL_PAGESIZE)
      PoolsWarning("PoolsInit", "Using page size of %lu bytes instead of POOL_PAGESIZE = %lu bytes", (unsigned long)global.pagesize, (unsigned long)POOL_PAGESIZE);

    
    global.align = POOL_ALIGN;
    if (sizeof(int) > global.align) global.align = sizeof(int);
    if (sizeof(void*) > global.align) global.align = sizeof(void*);
    
    global.align = MakePower2(global.align);
    if (global.align != POOL_ALIGN)
      PoolsWarning("PoolsInit", "Using minimum cell alignement of %u bytes instead of POOL_ALIGN = %u bytes", global.align, POOL_ALIGN);
  }
}

#define POOLS_INIT   { if (!global.inited) PoolsInit(); }


#ifdef DEBUG
static void PrintPoolState(POOL_STATE *pool)
{
  DPRINTF("nr_pages = %d, cells_per_page = %d, nr_used_cells = %d, req_cell_size = %d, cell_size=%d, cell_align = %d, cell_offset = %d, next_free_cell = %p, pages = %p, prev = %p, next = %p, name = '%s'\n",
	  pool->nr_pages,
	  pool->cells_per_page,
	  pool->nr_used_cells,
	  pool->req_cell_size,
	  pool->cell_size,
	  pool->cell_align,
	  pool->cell_offset,
	  pool->next_free_cell,
	  pool->pages,
	  pool->prev,
	  pool->next,
	  pool->name);
}

static void PrintPageHeader(POOL *page)
{
  DPRINTF("page %p: state = %p (name '%s', next_free_cell = %p), prev = %p, next = %p, cells_used_this_page = %d\n",
	  page,
	  page->state,
	  page->state ? page->state->name : "????",
	  page->state ? page->state->next_free_cell : 0,
	  page->prev,
	  page->next,
	  page->cells_used_this_page);
}

static void PrintGlobals(void)
{
  DPRINTF("inited = %s\npools = %p\nmagic1 = %08x, magic 2 = %08x\nbytesAllocated = %ld, bytesOverhead = %ld\npagesize = %ld, align = %d\nmasterblock = %p, nextpagetotake = %p, nextfreepage = %p\n",
	  global.inited ? "TRUE" : "FALSE", 
	  global.pools,
	  global.magic1, global.magic2,
	  global.bytesAllocated, global.bytesOverhead,
	  global.pagesize, global.align,
	  global.masterblock, global.nextpagetotake, global.nextfreepage);
}
#endif



void *Alloc(unsigned long nrbytes)
{
  char *buf;

  POOLS_INIT;
  if ((buf = (char *)malloc(nrbytes)) == NULL) {
    perror("Alloc: ");
    fprintf(stderr, "Alloc: %ld bytes of memory allocated that we know of.\n", global.bytesAllocated);
    PoolsFatal("Alloc", "failed to allocate %d bytes", nrbytes);
  }
  global.bytesAllocated += nrbytes;
#ifdef DDEBUG
  DPRINTF("Alloc %d bytes OK: %ld bytes allocated in total now.", nrbytes, global.bytesAllocated);
#endif

  return buf;
}


void Free(void *buf, unsigned long nrbytes)
{
  POOLS_INIT;
  if (!buf) return;
  free(buf);
  global.bytesAllocated -= nrbytes;
#ifdef DDEBUG
  DPRINTF("Free %d bytes ... OK: %ld bytes allocated in total now.", nrbytes, global.bytesAllocated);
#endif
}

void FakeFree(void *buf, unsigned long nrbytes)
{
  global.bytesAllocated -= nrbytes;
}


void *Realloc(void *buf, unsigned long size, int extra)
{
  POOLS_INIT;
  if ((buf = (char *)realloc(buf, size+extra)) == NULL) {
    perror("Realloc: ");
    fprintf(stderr, "Realloc: %ld bytes of memory allocated that we know of.\n", global.bytesAllocated);
    PoolsFatal("Relloc", "failed to reallocate %d bytes", size+extra);
  }
  global.bytesAllocated += extra;
#ifdef DDEBUG
  DPRINTF("Realloc %d to %d bytes ... OK: %ld bytes allocated in total now.", size, size+extra, global.bytesAllocated);
#endif
  return buf;
}





static POOL *RetakeFreePage(void)
{
  POOL *page = (POOL*)global.nextfreepage;
  if (page)	
    global.nextfreepage = *(unsigned char **)page;
  return page;
}

static void AllocNewMasterBlock(void)
{
  unsigned char *p = (unsigned char *)Alloc(MASTERBLOCKSIZE);
  
  
  global.masterblock = (unsigned char *)ALIGN_UP(p, global.pagesize);
  global.bytesOverhead += MASTERBLOCKOVERHEAD;
  global.nrmasterblocks ++;
}

static POOL* AllocPage(void)
{
  
  POOL *new = RetakeFreePage();
  if (new)
    return new;

  
  if (!global.nextpagetotake) { 	
    AllocNewMasterBlock();
    global.nextpagetotake = global.masterblock;
  }

  new = (POOL*)global.nextpagetotake;

  global.nextpagetotake += global.pagesize;
  if (global.nextpagetotake-global.masterblock >= PAGES_IN_MASTERBLOCK*global.pagesize)
    global.nextpagetotake = NULL;	

  return new;
}


static void FreePage(POOL *page)
{
  *(unsigned char **)page = global.nextfreepage; 
  global.nextfreepage = (unsigned char *)page;
}



static void RegisterPool(POOL_STATE *newpool)
{
  
  newpool->next = global.pools;
  if (global.pools)
    global.pools->prev = newpool;
  global.pools = newpool;
}


static void UnregisterPool(POOL_STATE *pool)
{
  if (global.pools == pool)  
    global.pools = pool->next;
  if (pool->prev) pool->prev->next = pool->next;
  if (pool->next) pool->next->prev = pool->prev;
}


static POOL_STATE* NewPoolState(unsigned cell_size, unsigned cell_align, char *name)
{
  POOL_STATE *pool = (POOL_STATE*)Alloc(sizeof(POOL_STATE));
  global.bytesOverhead += sizeof(POOL_STATE);

  pool->magic = global.magic1;
  pool->name = name;

  if (cell_align <= 0) cell_align = global.align;
  
  pool->cell_align = ROUND_UP(cell_align, global.align);

  pool->req_cell_size = cell_size;
  
  if (cell_size < sizeof(FREE_CHAIN)) cell_size = sizeof(FREE_CHAIN);
  
  pool->cell_size = ROUND_UP(cell_size, pool->cell_align);

  
  pool->cell_offset = (unsigned int)ROUND_UP(sizeof(POOL), pool->cell_align);
  pool->cells_per_page = (global.pagesize - pool->cell_offset) / pool->cell_size;
  if (pool->cells_per_page < 1) {
    PoolsFatal("NewPoolState", "cell size or alignement too large for page size (%d bytes). Don't put such large cells in pools.", global.pagesize);
  }

  
  pool->nr_pages = pool->nr_used_cells = 0;
  pool->next_free_cell = (void*)NULL;
  pool->pages = (POOL*)NULL;
  pool->next = pool->prev = (POOL_STATE*)NULL;

  return pool;
}


static void FreePoolState(POOL_STATE *state)
{
  Free((char*)state, sizeof(POOL_STATE));
  global.bytesOverhead -= sizeof(POOL_STATE);
}


static POOL *NewPage(void)
{
  POOL *page = AllocPage();
  page->state = (POOL_STATE*)NULL;
  page->next = page->prev = (POOL*)NULL;
  page->cells_used_this_page = 0;
  return page;
}


static void DisposePage(POOL *page)
{
  FreePage(page);
}


static void GiveFree(FREE_CHAIN *cell, POOL_STATE *pool)
{
  cell->next = pool->next_free_cell;
#ifndef POOL_LAZY_GARBAGE_COLLECTION  
  if (cell->next) cell->next->prev = cell;
  cell->prev = NULL;
#endif
  pool->next_free_cell = cell;
}


static void *TakeFree(POOL_STATE *pool)
{
  FREE_CHAIN *cell = pool->next_free_cell;
  pool->next_free_cell = pool->next_free_cell->next;
#ifndef POOL_LAZY_GARBAGE_COLLECTION  
  if (pool->next_free_cell) pool->next_free_cell->prev = NULL;
#endif
  return cell;
}

#ifndef POOL_LAZY_GARBAGE_COLLECTION  

static void DetachFreeCell(FREE_CHAIN *cell, POOL_STATE *pool)
{
  if (cell == pool->next_free_cell)
    pool->next_free_cell = cell->next;
  if (cell->next) cell->next->prev = cell->prev;
  if (cell->prev) cell->prev->next = cell->next;
}
#endif 


static POOL *AttachPage(POOL_STATE *state, POOL *newpage)
{
  newpage->state = state;

  
  if (state->pages)
    state->pages->prev = newpage;
  newpage->next = state->pages;
  state->pages = newpage;
  state->nr_pages ++;

  
  ForAllCellsInPage(cell, newpage) {
    GiveFree(cell, state);
  } EndForAll;

  global.bytesOverhead += PAGE_OVERHEAD(state);
  return state->pages;
}


static POOL *DetachPage(POOL_STATE *state, POOL *page)
{
  global.bytesOverhead -= PAGE_OVERHEAD(state);

  if (state->pages == page)
    state->pages = page->next;
  if (page->next) page->next->prev = page->prev;
  if (page->prev) page->prev->next = page->next;
  page->state = (POOL_STATE*)NULL;
  state->nr_pages --;
  return state->pages;
}

#ifndef POOL_LAZY_GARBAGE_COLLECTION  

static POOL *ReleasePage(POOL *page)
{
  ForAllCellsInPage(cell, page) {
    DetachFreeCell(cell, page->state);
  } EndForAll;
  return DetachPage(page->state, page);
}
#endif


static POOL *PageContainingCell(void *cell)
{
  return (POOL*)ALIGN_DOWN(cell, global.pagesize);
}

#ifdef POOL_CLEAR_CELLS

static void ClearCell(unsigned *cell, unsigned size)
{
  size /= sizeof(unsigned);	
  while (size>=4) { *cell++ = *cell++ = *cell++ = *cell++ = 0; size -= 4; }
  if    (size>=2) { *cell++ = *cell++ = 0; size -= 2; }
  if    (size>=1) { *cell++ = 0; size--; }
}
#endif 


static void *TakeCell(POOL *page)
{
  void *cell = TakeFree(page->state);
#ifdef POOL_CLEAR_CELLS
  ClearCell(cell, page->state->cell_size);
#endif 
  page->cells_used_this_page ++;
  page->state->nr_used_cells ++;
  return cell;
}


static void GiveCell(void *cell, POOL *page)
{
  GiveFree(cell, page->state);
  page->cells_used_this_page --;
  page->state->nr_used_cells --;
}

void *NewPoolCell(unsigned cell_size, unsigned cell_align, char *name, POOL **poolptr)
{
  POOL *pool, *page;
  void *newcell;

  POOLS_INIT;
#ifdef SANITY_CHECKS
  if (!poolptr)
    PoolsFatal("NewPoolCell", "Invalid argument: POOL pointer shall not be a NULL pointer");
#endif
  pool = *poolptr;
  
  if (!pool) { 
    POOL *page = NewPage();
    POOL_STATE *newpoolstate = NewPoolState(cell_size, cell_align, name);
    RegisterPool(newpoolstate);
    pool = AttachPage(newpoolstate, page);
  }
#ifdef SANITY_CHECKS
  if (!VALID_POOL(pool)) {
    PoolsFatal("NewPoolCell", "Trying to allocate a cell from an invlid pool");
  }
#endif

  if (!pool->state->next_free_cell) {  
    pool = AttachPage(pool->state, NewPage());
  }

  page = PageContainingCell(pool->state->next_free_cell);
  newcell = TakeCell(page);

  *poolptr = pool;
  return newcell;
}

void Dispose(void *cell, POOL **poolptr)
{
  POOL *pool, *page;

  POOLS_INIT;
#ifdef SANITY_CHECKS
  if (!poolptr || !VALID_POOL(*poolptr))
    PoolsFatal("Dispose", "Invalid pool pointer");
#endif
  pool = *poolptr;
#ifdef SANITY_CHECKS
  if (!cell)
    PoolsFatal("Dispose", "Invalid cell pointer");
#endif
  page = PageContainingCell(cell);
#ifdef SANITY_CHECKS
  if (!VALID_POOL(page) || page->state != pool->state)
    PoolsFatal("Dispose", "Trying to dispose a cell from an invalid or wrong pool");
#endif
  GiveCell(cell, page);

#ifndef POOL_LAZY_GARBAGE_COLLECTION
  if (page->cells_used_this_page == 0) {
    
    POOL_STATE *state = pool->state;
    pool = ReleasePage(page);
    DisposePage(page);
    if (!pool) {
      UnregisterPool(state);
      FreePoolState(state);
    }
  }
#else 
  if (page->state->nr_used_cells == 0) {
    DisposeAll(poolptr);   
    pool = NULL;
  }
#endif 

  *poolptr = pool;
}

void DisposeAll(POOL **poolptr)
{
  POOL_STATE *state;

  POOLS_INIT;
#ifdef SANITY_CHECKS
  if (!poolptr || !VALID_POOL(*poolptr))
    PoolsFatal("DisposeAll", "Invalid pool pointer");
#endif

  state = (*poolptr)->state;

  while (state->pages) {
    POOL *page = state->pages;
    DetachPage(state, page);
    DisposePage(page);
  }
  UnregisterPool(state);
  FreePoolState(state);

  *poolptr = (POOL*)NULL;
}


static void PrintPoolStatHeader(FILE *out)
{
  fprintf(out, "%-23s %7s %7s %7s %7s %7s %7s %7s\n",
	  "Pool name",
	  "Alloc'd",
	  "Used",
	  "Ovrhead",
	  "NrCells",
	  "Size",
	  "ReqSize",
	  "Align"
	  );
}


char *GetPoolUsage(POOL *pool,
		   unsigned long *alloced,
		   unsigned long *used,
		   unsigned long *overhead)
{
  POOL_STATE *state = pool ? pool->state : NULL;
  if (!state) {
    *alloced = *used = *overhead = 0;
  } else {
    *alloced = sizeof(POOL_STATE) + state->nr_pages * global.pagesize;
    *used = sizeof(POOL_STATE) + state->nr_pages * PAGE_OVERHEAD(state) + state->nr_used_cells * state->cell_size;
    *overhead = sizeof(POOL_STATE) + state->nr_pages * PAGE_OVERHEAD(state);
  }
  return state ? state->name : NULL;
}


void PrintPoolStats(FILE *out, POOL *pool)
{
  POOL_STATE *state = pool->state;
  unsigned long alloced, used, overhead;
  GetPoolUsage(pool, &alloced, &used, &overhead);
  PrintPoolStatHeader(out);
  fprintf(out, "%-23s %7lu %7lu %7lu %7u %7u %7u %7u\n",
	  state->name,
	  alloced/1024, used/1024, overhead/1024,
	  state->nr_used_cells,
	  state->cell_size,
	  state->req_cell_size,
	  state->cell_align
	  );
}


void PrintPooledMemoryBreakdown(FILE *out)
{
  POOL_STATE *pool;
  unsigned long total_alloced=0, total_used=0, total_overhead=0;

  PrintPoolStatHeader(out);
  for (pool = global.pools; pool; pool = pool->next) {
    unsigned long alloced, used, overhead;
    GetPoolUsage(pool->pages, &alloced, &used, &overhead);
    total_alloced += alloced;
    total_used += used;
    total_overhead += overhead;
    fprintf(out, "%-23s %7lu %7lu %7lu %7u %7u %7u %7u\n",
	    pool->name,
	    alloced/1024, used/1024, overhead/1024,
	    pool->nr_used_cells,
	    pool->cell_size,
	    pool->req_cell_size,
	    pool->cell_align
	    );
  }
  total_overhead += global.nrmasterblocks*MASTERBLOCKOVERHEAD + sizeof(POOL_GLOBALS);
  fprintf(out, "%-23s (%4dM) (%4dM) %7d\n",
	  "Pools reserve:",
	  global.nrmasterblocks*MASTERBLOCKSIZE/1024/1024, 
	  global.nrmasterblocks*MASTERBLOCKSIZE/1024/1024, 
	  global.nrmasterblocks*MASTERBLOCKOVERHEAD/1024);
  fprintf(out, "%-23s %7lu %7lu %7lu\n",
	  "Pools total:",
	  total_alloced/1024, 
	  total_used/1024, 
	  total_overhead/1024);
  fprintf(out, "%-23s %7lu %7lu %7lu\n",
	  "Non-pooled:",
	  (global.bytesAllocated - global.nrmasterblocks*MASTERBLOCKSIZE)/1024,
	  (global.bytesAllocated - global.nrmasterblocks*MASTERBLOCKSIZE)/1024,
	  (global.bytesOverhead - total_overhead)/1024);
  total_alloced += global.bytesAllocated - global.nrmasterblocks*MASTERBLOCKSIZE;
  total_used += global.bytesAllocated - global.nrmasterblocks*MASTERBLOCKSIZE;
  total_overhead += global.bytesOverhead - total_overhead;
  fprintf(out, "%-23s %7lu %7lu %7lu\n",
	  "Total:",
	  total_alloced/1024,
	  total_used/1024,
	  total_overhead/1024);
}


unsigned long GetMemoryUsage(void)
{
  POOL_STATE *pool;
  unsigned long n;

  POOLS_INIT;

  
  n = global.bytesAllocated - global.nrmasterblocks*MASTERBLOCKSIZE;

  
  for (pool = global.pools; pool; pool = pool->next) {
    unsigned long alloced, used, overhead;
    GetPoolUsage(pool->pages, &alloced, &used, &overhead);
    n += used;
  }

  return n;
}


unsigned long GetMemoryOverhead(void)
{
  POOLS_INIT;
  return global.bytesOverhead;
}

