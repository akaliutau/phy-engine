/****************************************************************************\

  Copyright 1995 The University of North Carolina at Chapel Hill.
  All Rights Reserved.

  Permission to use, copy, modify and distribute this software and its
  documentation for educational, research and non-profit purposes,
  without fee, and without a written agreement is hereby granted,
  provided that the above copyright notice and the following three
  paragraphs appear in all copies.

  IN NO EVENT SHALL THE UNIVERSITY OF NORTH CAROLINA AT CHAPEL HILL BE
  LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
  CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE
  USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY
  OF NORTH CAROLINA HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH
  DAMAGES.


  Permission to use, copy, modify and distribute this software and its
  documentation for educational, research and non-profit purposes,
  without fee, and without a written agreement is hereby granted,
  provided that the above copyright notice and the following three
  paragraphs appear in all copies.

  THE UNIVERSITY OF NORTH CAROLINA SPECIFICALLY DISCLAIM ANY
  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
  PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND THE UNIVERSITY OF
  NORTH CAROLINA HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,
  UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

  The authors may be contacted via:

  US Mail:  Jonathan Cohen                      Amitabh Varshney
            Department of Computer Science      Department of Computer Science 
            Sitterson Hall, CB #3175            State University of New York
            University of N. Carolina           Stony Brook, NY 11794-4400, USA 
            Chapel Hill, NC 27599-3175
	    
  Phone:    (919)962-1749                       Phone: (516)632-8446 
	    
  EMail:    cohenj@cs.unc.edu                   varshney@cs.sunysb.edu

\****************************************************************************/


/* Protection from multiple includes. */
#ifndef INCLUDED_PLYSIMPLIFY_SIMPLIFY_H
#define INCLUDED_PLYSIMPLIFY_SIMPLIFY_H


/*------------------ Includes Needed for Definitions Below ------------------*/

#include <malloc.h>
#include <string.h>
#include <geometry.h>

/*-------------------------------- Constants --------------------------------*/

#if 0
#define VERTEX_EPSILONS
#endif


#if 0
#define REMOVE_BORDER_VERTS
#endif

#if 1
#define USE_EXHAUSTIVE_FILL_HOLE
#endif

/* #define BORDER_TEST to TUBES_TEST, ANGLE_TEST, or XY_ANGLE_TEST */
#define TUBES_TEST 1
#define ANGLE_TEST 2
#define XY_ANGLE_TEST 3

#define BORDER_TEST TUBES_TEST


/*--------------------------------- Macros ----------------------------------*/

#ifndef ALLOCN
#define REALLOCN(PTR,TYPE,OLD_N,NEW_N)\
{										\
	    if ((OLD_N) == 0)                                           		\
	    {   ALLOCN((PTR),TYPE,(NEW_N));}                            		\
	    else									\
	    {								    		\
	       (PTR) = (TYPE *)realloc((PTR),(NEW_N)*sizeof(TYPE));			\
	       if (((PTR) == NULL) && ((NEW_N) != 0))					\
	       {									\
		   fprintf(stderr, "Memory reallocation failed on line %d in %s\n", 	\
		           __LINE__, __FILE__);                             		\
		   fprintf(stderr, "  tried to reallocate %d->%d\n",       		\
			   (OLD_N), (NEW_N));                              		\
		   exit(-1);								\
	       }									\
	       if ((NEW_N)>(OLD_N))							\
		   memset((char *)(PTR)+(OLD_N)*sizeof(TYPE), 0,			\
		          ((NEW_N)-(OLD_N))*sizeof(TYPE));				\
	    }										\
	}

#define  ALLOCN(PTR,TYPE,N) 					         \
	{ (PTR) = (TYPE *) calloc(((unsigned)(N)),sizeof(TYPE));         \
	  if (((PTR) == NULL) && ((N) != 0))                             \
	  {    				                                 \
	  fprintf(stderr, "Memory allocation failed on line %d in %s\n", \
		 __LINE__, __FILE__);                                    \
          fprintf(stderr, "\ttried to allocate %d bytes\n",              \
                  (N)*sizeof(TYPE));                                     \
	  exit(-1);                                                      \
	  }							         \
	}


#define FREE(PTR)  { free((PTR)); (PTR) = NULL; }
#endif

/*---------------------------------- Types ----------------------------------*/


/*---------------------------- Function Prototypes --------------------------*/

void simplify(Surface *orig_model, Surface **simp_model, double epsilon);

/*---------------------------Globals (externed)------------------------------*/



/* Protection from multiple includes. */
#endif /*INCLUDED_PLYSIMPLIFY_SIMPLIFY_H*/



