

#include "ipc.h"
#include <sys/msg.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "options.h"
#include "Boolean.h"
#include "camera.h"
#include "render.h"

IPC Ipc;

static void DoQuit(void *value)
{
   fprintf(stderr, "Quiting on demand\n");
   exit(0);
}

extern int DoRadianceOneStep(void);

static void Do1Iteration(void *value)
{
   DoRadianceOneStep();
}

static void EnableIpc(void *value)
{
   Ipc.have_ipc = TRUE;
}

void IpcDefaults()
{
   Ipc.have_ipc = FALSE;
   Ipc.mtypeoffset = 0;
   Ipc.msqid = 0;
}

static CMDLINEOPTDESC ipcOptions[] = {
  {"-ipc-mtypeoffset",	1,	Tint,		&Ipc.mtypeoffset,  EnableIpc,
   "-ipc-mtypeoffset  <int>\t: mtypeoffsets, <ing>+1 is receiving mtype, +2 sending"},
  {"-ipc-qid",	1,	Tint,		&Ipc.msqid, 	EnableIpc,
   "-ipc-qid  <int>\t: the msg-queue id"},
  {"-ipc-quit",	0,	TYPELESS,	NULL,		DoQuit,
   "-ipc-quit\t: quit. (only usefull through IPC)"},
  {"-ipc-radiance-iterate",	0,	TYPELESS,	NULL, Do1Iteration,
   "-ipc-radiance-iterate\t: iterate once more. (only usefull through IPC)"},
  {NULL	, 	0,	TYPELESS, 	NULL, 		DEFAULT_ACTION,
   NULL }
};

void ParseIpcOptions(int *argc, char **argv)
{
   ParseOptions(ipcOptions, argc, argv);
}

void PrintIpcOptions(FILE *fp)
{
   fprintf(fp, "\nIPC options:\n");
   PrintOptions(fp, ipcOptions);
}

#define MAXMSGSIZE 1024

typedef struct my_msgbuf {
      long mtype; 
      char command[MAXMSGSIZE];
} my_msgbuf;

static my_msgbuf rcvbuf;

static void MakeArgcAndArgv(int *argc, char **argv, char *commandline)
{
   char *c = commandline;
   *argc = 0;

   do {
      
      while (*c != 0 && *c == ' ') ++c;
      if (*c != 0) { 
	 char *k= strchr(c, ' ');
	 if (k) {
	    argv[(*argc)] = malloc(k - c + 1);
	    argv[(*argc)][k - c] = 0;
	    strncpy(argv[(*argc)++], c, k - c);
	    c = k;
	 } else {
	    argv[(*argc)++] = strdup(c);
	    return;
	 }
      }
   } while (*c != 0);
}
     
void IpcCheckForMessages(void)
{
   

   int setneedrerender = FALSE;
   int retval;

   do {
      retval = msgrcv(Ipc.msqid, (struct msgbuf *)&rcvbuf, MAXMSGSIZE, Ipc.mtypeoffset+1,
			  MSG_NOERROR | IPC_NOWAIT);
      if (retval == -1) {
	 if (errno != ENOMSG)
	    fprintf(stderr, "Oopsing errno = %d\n", errno);
      } else
	 if (retval > 0) {
	    char *argv[50];
	    int argc;

	    MakeArgcAndArgv(&argc, argv, rcvbuf.command);

	    ParseCameraOptions(&argc, argv);
	    ParseIpcOptions(&argc, argv);

	    CameraComplete(&Camera);
	    setneedrerender = TRUE;
	 } else
	    fprintf(stderr, "Empty msg ?\n");
   } while (retval > 0);

   if (setneedrerender) {
      RenderScene();
   }
}
