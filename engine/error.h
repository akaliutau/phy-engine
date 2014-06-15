/* error.h: for printing warning/error/fatal error messages */

#ifndef _PHY_ERROR_H_
#define _PHY_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
#ifndef NULL
#define NULL (char *)0
#endif /*NULL*/
#endif

#ifndef FALSE
#define FALSE 0
#endif /*FALSE*/

#ifndef TRUE
#define TRUE 1
#endif /*TRUE*/


#ifdef NAN_CHECKS

  /* A number of interesting macro's when you get in NaN or Inf
     trouble... */

#ifndef LINUX   /* thus SGI, what about other systems ?? */
#include "ieeefp.h"
#define ISINF(d) ((fpclass((double)d) == FP_NINF)||(fpclass((double)d) == FP_PINF))
#else
#define ISINF(d) (isinf(d))
#endif

#define ISNAN(d) (isnan((double)d))
#define ISNANORINF(d) ((ISNAN(d))||(ISINF(d)))


#ifndef KILLFPE  /* From nan.h (SGI include files) */
#include "signal.h"
#include "unistd.h"
#define KILLFPE() (void) kill(getpid(), 8)
#endif


  /* PNAN prints een error message if the (double) argument is NaN */
#define PNAN(d) {if(ISNAN(d)) printf("NaN %s, file %s, line %i, val %f\n", #d, __FILE__, __LINE__, (d));}

#define PINF(d) {if(ISINF(d)){printf("Inf %s, file %s, line %i, val %f\n", #d, __FILE__, __LINE__, (d));}}

#define PNANORINF(d) {PNAN(d); PINF(d);}

/* Some macro's to generate a core dump if a double is NaN of Inf */
#define KILLNAN(d) {if(ISNAN(d)) {KILLFPE();}}
#define KILLINF(d) {if(ISINF(d)) {KILLFPE();}}
#define KILLNANORINF(d) {KILLNAN(d); KILLINF(d);}


#else /* ! NAN_CHECKS */

/* No NaN or Inf checking. Tests return false, prints/kills do nothing */

#define ISINF(d) (FALSE)
#define ISNAN(d) (FALSE)
#define ISNANORINF(d) (FALSE)

#define PNAN(d)
#define PINF(d)
#define PNANORINF(d)
#define KILLNAN(d) 
#define KILLINF(d) 
#define KILLNANORINF(d) 
#endif


/* prints an error message. Behaves much like printf. The first argument is the
 * name of the routine in which the error occurs (optional - can be NULL) */
extern void Error(char *routine, char *text, ...);

  /* same, but for warning messages */
extern void Warning(char *routine, char *text, ...);

/* prints an informational message. Informational message are NOT printed
 * unless error.c is compiled with -DINFO. */
extern void Info(char *routine, char *text, ...);

/* same, but for fatal error messages (also aborts the program).
 * First argument is a return code. We use negative return codes for
 * "internal" error messages. */
extern void Fatal(int errcode, char *routine, char *text, ...);

/* returns FALSE if no errors have been reported since the last call to this
 * routine. */
extern int ErrorOccurred(void);

/* set state to "no errors occured" */
extern void ErrorReset(void);

/* Assert
 * With assert, tests can be included in the program that will lead to
 * a core dump (debug) or fatal error message (non debug) if they
 * are not fulfilled.
 */

#ifdef DEBUGASSERTS
#define Assert(test, message) \
if(!(test)) { Error(NULL, \
              message " in file : " __FILE__ " line : %i\n",__LINE__); \
              abort(); \
          }
#else
#define Assert(test, message) \
if(!(test)) Fatal(-1, NULL, \
                  message " in file :" __FILE__ " line : %i\n",__LINE__);
#endif


#ifdef __cplusplus
}
#endif

#endif /*_PHY_ERROR_H_*/
