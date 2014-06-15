/* pnm_private.h: PNM library private globale variabes and functions */

#ifndef _PNM_PRIVATE_
#define _PNM_PRIVATE_

/* prints an informational message */
extern void PnmInfo(char *routine, char *text, ...);

/* prints a warning message */
extern void PnmWarning(char *routine, char *text, ...);

/* prints an error message */
extern void PnmError(char *routine, char *text, ...);

/* prints a fatal error message */
extern void PnmFatal(char *routine, char *text, ...);

#endif /*_PNM_RPIVATE_*/
