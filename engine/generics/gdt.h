/* generics: the Generic Data Types library */

#ifndef _GDT_H_
#define _GDT_H_

#ifdef __cplusplus
extern "C" {
#endif

/* callback routines for communicating informational, warning, error and
 * fatal error messages. */
typedef void (*GDT_MSG_CALLBACK_FUNC)(char *message);

/* No error message from the library is longer than this */
#define GDT_MAX_MESSAGE_LENGTH 200

/* The SetCallback functions return the previously set callback function */
extern GDT_MSG_CALLBACK_FUNC GdtSetInfoCallback(GDT_MSG_CALLBACK_FUNC f);
extern GDT_MSG_CALLBACK_FUNC GdtSetWarningCallback(GDT_MSG_CALLBACK_FUNC f);
extern GDT_MSG_CALLBACK_FUNC GdtSetErrorCallback(GDT_MSG_CALLBACK_FUNC f);
extern GDT_MSG_CALLBACK_FUNC GdtSetFatalCallback(GDT_MSG_CALLBACK_FUNC f);

#ifdef __cplusplus
}
#endif

#endif /* _GDT_H_ */
