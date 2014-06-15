/* error.h; pools library error reporting routine */

#ifndef _POOLS_ERROR_H_
#define _POOLS_ERROR_H_

/* debug output will be generated if this callback function has been set */
extern POOLS_MSG_CALLBACK_FUNC pools_info_callback_func;
extern void PoolsInfo(char *routine, char *text, ...);
extern void PoolsWarning(char *routine, char *text, ...);
extern void PoolsError(char *routine, char *text, ...);
extern void PoolsFatal(char *routine, char *text, ...);

#endif /*_POOLS_ERROR_H_*/
