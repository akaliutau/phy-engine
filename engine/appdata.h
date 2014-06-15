/*
 * appdata.h
 *
 * Describes the application specific data that comes
 * out of resource files. Contains user specific options etc.
 *
 * If fields are added, ui_config.c must be changed too.
 */

#ifndef _APPDATA_H_
#define _APPDATA_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "camera.h"

/* New resource representation type definitions */

#define XmRVector "phyVector"


/* Substructures */

typedef struct
{
  char *spec;
  Widget widget;
  CAMERA cam;
  CAMERA acam;
} RECENTFILE;

/* Application data */

#define NUM_RECENT_FILES 6

typedef struct
{
  char *phyrcFilename;
  RECENTFILE recentFile[NUM_RECENT_FILES];
  float redgamma, greengamma, bluegamma;
} APPDATA;

extern APPDATA appData;

#ifdef __cplusplus
}
#endif
#endif /* _APPDATA_H_ */
