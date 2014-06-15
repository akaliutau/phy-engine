/* monitor.h: debugging aid: maintains a list of patches to be monitored. */

#ifndef _MONITOR_H_
#define _MONITOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "patch_type.h"

/* Initializes the list of patches to be monitored. */
extern void MonitorInit(void);

/* returns TRUE if the given patch is being monitored and FALSE if not. */
extern int Monitored(PATCH *patch);

/* adds the patch to the list of patches to be monitored. */
extern void MonitorAdd(PATCH *patch);

/* removes the patch from the list of patches being monitored. */
extern void MonitorRemove(PATCH *patch);

#ifdef __cplusplus
}
#endif

#endif /*_MONITOR_H_*/
