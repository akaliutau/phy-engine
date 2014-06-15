/* private.h: private data structures and routines for the generics library */

#ifndef _GDT_PRIVATE_H_
#define _GDT_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* prints an informational, warning, error, fatal error  message */
extern void GdtInfo(char *routine, char *text, ...);
extern void GdtWarning(char *routine, char *text, ...);
extern void GdtError(char *routine, char *text, ...);
extern void GdtFatal(char *routine, char *text, ...);

#ifdef __cplusplus
}
#endif

#endif /* _GDT_PRIVATE_H_ */
