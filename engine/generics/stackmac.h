/* stackmac.h: simple stack macros */

#ifndef _STACKMAC_H_
#define _STACKMAC_H_

#include "error.h"

#define STACK_DECL(TYPE, stack, maxdepth, stackptr) 	\
TYPE stack[maxdepth+1]; TYPE *stackptr = &stack[0]

#define STACK_SAVE(obj, stack, maxdepth, stackptr) { 	\
  if (stackptr-stack < maxdepth) 			\
    *stackptr++ = obj; 					\
  else 							\
    Fatal(-1, "STACK_SAVE", "Stack overflow in %s line %d", __FILE__, __LINE__); \
}

#define STACK_RESTORE(obj, stack, stackptr) {		\
  if (stackptr-stack > 0) 				\
     obj = *--stackptr;					\
  else							\
    Fatal(-1, "STACK_RESTORE", "Stack underflow in %s line %d", __FILE__, __LINE__); \
}

#define STACK_RESTORE_NOCHECK(obj, stack, stackptr) {	\
  obj = *--stackptr;					\
}

#endif /*_STACKMAC_H_*/
