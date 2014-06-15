/* pools.h: memory management routines on top of the C runtime library */

#ifndef _POOLS_H_
#define _POOLS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* ------------------------------------------------------------------ */
/* Allocate nrbytes bytes of memory and return a pointer to the allocated
 * memory. Basically calls malloc(), but also checks the result
 * (it catches out-of-memory errors e.g.) and keeps a track how much memory
 * has been allocated. */
extern void *Alloc(unsigned long nrbytes);

/* Return the memory pointed to by buf to the system. The second argument
 * is only used to keep track of how much memory is being used by the
 * library. It should be the number of bytes being pointed to by buf,
 * same argument as when it was allocate dusing Alloc() above. */
extern void Free(void *buf, unsigned long nrbytes);

/* Changes the size of the piece of memory being pointed to by buf:
 * new size is size+extra. extra can be negative. */
extern void *Realloc(void *buf, unsigned long size, int extra);

/* Returns the total number of used bytes allocated with this library. */
extern unsigned long GetMemoryUsage(void);

/* Returns total bytes of overhead caused by the memory allocator. */
extern unsigned long GetMemoryOverhead(void);

/* ------------------------------------------------------------------ */
/* A memory management system on top of the standard C runtime library
 * storage allocator. It reduces the overhead (both time and space) when
 * dealing with a large quantity of small cells of equal size. Such cells
 * are grouped in pools. Each pool consists of zero, one or more blocks
 * of memory (pages). In each page there is place for a certain number of
 * cells. Only when a new page is needed (because e.g. all others are filled 
 * up), the C runtime library storage allocator is called to create a new, 
 * empty page. The pages are of same alignement and size as system memory 
 * pages, regardless of the size of the cells they contain.
 * A new cell is allocated or disposed of using the routines New()
 * and Dispose() below. A user of these routines should never self
 * manipulate the POOL structures. 
 *
 *
 * Example of usage:
 *
 * Somewhere on top of a source file:
 *
 * 	typedef some_type THE_THING;
 *
 * 	#ifndef NOPOOLS
 *
 *	#include "pools.h"
 *	static POOL *thingPool = (POOL *)NULL; 
 *	#define NEWTHING()	(THE_THING *)New(sizeof(THE_THING), &thingPool)
 *	#define DISPOSETHING(ptr) Dispose((void *)ptr, &thingPool)
 *
 *	#else
 *
 *	#include <stdlib.h>
 *	#define NEWTHING	(THE_THING *)malloc(sizeof(THE_THING))
 *	#define DISPOSETHING(ptr) free((void *)ptr)
 *
 *	#endif
 *
 * Use it as follows:
 * 
 *	THE_THING *thing;
 *	...
 *	thing = NEWTHING();
 *	...
 *	DISPOSETHING(thing);
 *
 * Using NewPoolCell() below directly instead of the New() macro, allows
 * to specify to what byte boundaries the cells shall be aligned and
 * to give a name to a pool. With the routine PrintPooledMemoryBreakdown(),
 * you can then obtain a detailled overview of how much memory is allocated
 * and used + the overhead in all pools.
 */

#ifndef _POOLSP_H_
  /* opaque data type */
typedef void *POOL;
#endif

/* Allocate a new cell of 'cell_size' bytes, aligned to a 'cell_align' byte
 * boundary from the POOL pointed to by '*poolptr', with name 'pool_name'.
 * If '* poolptr' is NULL, a new POOL is created.
 * If 'cell_align' is 0, a system-dependent minimum alignement will be
 * used (minimum of POOL_ALIGN in poolsP.h and the machine word size).
 * If 'cell_align' is not 0, it will be rounded up to the next integer multiple
 * of the minimum alignement.
 * Next, 'cell_size' is rounded up to an integer multiple of the 
 * corrected 'cell_align'. The effective cell size may thus be larger
 * than the requested cell size.
 * The pool pointer '*poolptr' can be modified as a result of allocating a
 * cell from the pool, hence the need to pass the address of a POOL* instead
 * of a POOL* itself. */
extern void* NewPoolCell(unsigned cell_size, unsigned cell_align, char *pool_name, POOL **poolptr);

/* For backward compatibility with previous versions of this library: */
/* Unnamed pool with user-specified alignement */
#define NewAligned(cell_size, cell_align, poolptr) NewPoolCell(cell_size, cell_align, "unnamed", poolptr)

/* Unnamed pool with default alignement */
#define New(cell_size, poolptr) NewPoolCell(cell_size, 0, "unnamed", poolptr)

/* Dispose the cell pointed to by cellptr from a POOL. */
extern void Dispose(void *cellptr, POOL **poolptr);

/* Disposes all cells in the pool at once (faster than disposing 
 * all the cells one by one). */
extern void DisposeAll(POOL **poolptr);

/* Prints a detailed overview of allocated and used memory +
 * overhead in all pools */
extern void PrintPooledMemoryBreakdown(FILE *out);

/* Same, but for a single pool. */
extern void PrintPoolStats(FILE *out, POOL *pool);

/* just decreases our counter of how much memory we have in use */
extern void FakeFree(void *buf, unsigned long nrbytes);


/* ------------------------------------------------------------------ */
/* Routines for communication of informational (debug) and fatal error
 * messages to the user of the library. The routines to be specified
 * have one char* parameter: the message being passed. */
typedef void (*POOLS_MSG_CALLBACK_FUNC)(char *);

/* max. length of a message communicated by the pools library. */
#define POOLS_MAX_MESSAGE_LENGTH 200

/* default informational message callback function: unset. */
extern POOLS_MSG_CALLBACK_FUNC PoolsSetInfoCallback(POOLS_MSG_CALLBACK_FUNC func);

/* default fatal error callback function prints the message and exits
 * the program with -1 return code. */
extern POOLS_MSG_CALLBACK_FUNC PoolsSetFatalCallback(POOLS_MSG_CALLBACK_FUNC func);

/* ------------------------------------------------------------------ */
/* some handy macros */
#ifndef NULL
#define NULL (char *)0
#endif /*NULL*/

#ifndef FALSE
#define FALSE 0
#endif /*FALSE*/

#ifndef TRUE
#define TRUE 1
#endif /*TRUE*/

/* ------------------------------------------------------------------ */

#ifdef NOPOOLS
#ifndef PoolDecl
#define PoolDecl(classname, poolid)
#endif

#ifndef PoolImpl
#define PoolImpl(classname)
#endif
#endif

  /* C++ new() and delete replacements that use pools: */
#ifndef PoolDecl
#define PoolDecl(classname, poolid) 			\
  static POOL* pool; 					\
                                                        \
  void* operator new(size_t size)       		\
  {                                     		\
    return size==sizeof(classname) ? NewPoolCell(size, 0, poolid, &pool) : Alloc(size); \
  }							\
                                                        \
  void operator delete(void *p, size_t size)     	\
  {							\
    if (size==sizeof(classname))			\
      Dispose(p, &pool);				\
    else						\
      Free(p, size);					\
  }
#endif

#ifndef PoolImpl
#define PoolImpl(classname)				\
POOL* classname::pool = 0;
#endif

#ifdef __cplusplus
}
#endif

#endif /*_POOLS_H_*/
