/* ipc.h: interprocess communication through external interfaces */

#ifndef IPC_H_
#define IPC_H_

#include <sys/types.h>
#include <sys/ipc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IPC {
      int have_ipc;    /* boolean */	 
      int mtypeoffset; /* mtypeoffset + 1 is for receiving, mtypeoffset + 2
			  is for sending */
      int msqid;       /* the id of the msg queue */
} IPC;

extern IPC Ipc;		/* The IPC setup */

#include <stdio.h>
extern void ParseIpcOptions(int *argc, char **argv);
extern void PrintIpcOptions(FILE *fp);
extern void IpcDefaults(void);

extern void IpcCheckForMessages(void);
   
#ifdef __cplusplus
}
#endif

#endif /*IPC_H*/
