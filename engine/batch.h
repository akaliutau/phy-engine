/* batch.h: batch rendering */

#ifndef _BATCH_H_
#define _BATCH_H_


#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

extern void ParseBatchOptions(int *argc, char **argv);
extern void PrintBatchOptions(FILE *fp);
extern void Batch(void);

#ifdef __cplusplus
}
#endif
#endif /*_BATCH_H_*/
