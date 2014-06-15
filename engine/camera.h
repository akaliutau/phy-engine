/* camera.h: virtual camera management */

#ifndef _CAMERA_H_
#define _CAMERA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vector.h"
#include "color.h"

typedef struct CAMERA_CLIPPING_PLANE {
  VECTOR norm;
  float d;
} CAMERA_CLIPPING_PLANE;
#define NR_VIEW_PLANES	4

typedef struct CAMERA {
  VECTOR eyep, 		/* virtual camera position in 3D space */
    lookp,		/* focus point of camera */
    updir;		/* direction pointing upward */
  float viewdist;	/* distance from eyepoint to focus point */
  float fov, hfov, vfov;/* field of view, horizontal and vertical,
			 * in degrees. */
  float near, far;	/* near and far clipping plane distance */
  int hres, vres;	/* horizontal and vertical resolution */
  VECTOR X, Y, Z;	/* eye coordinate system: X=right, Y=down, Z=viewing direction */
  RGB background;	/* window background color */
  int changed;		/* True when camera position has been updated */
  float pixh, pixv;       /* Width and height of a pixel */
  float tanhfov, tanvfov; /* Just tanges of hfov and vfov */
  CAMERA_CLIPPING_PLANE viewplane[NR_VIEW_PLANES];
} CAMERA;

extern CAMERA Camera;		/* The one and only virtual camera */
extern CAMERA AlternateCamera;	/* The one and only alternate camera */

extern CAMERA *CameraCreate(void);
extern void CameraDestroy(CAMERA *camera);

extern int CameraCompare(CAMERA *cam1, CAMERA *cam2);

  void CameraPrint(FILE *out, CAMERA *Camera);

/* sets virtual camera position, focus point, up-direction, field of view
 * (in degrees), horizontal and vertical window resolution and window
 * background. Returns (CAMERA *)NULL if eyepoint and focus point coincide or
 * viewing direction is equal to the up-direction. */
extern CAMERA *CameraSet(CAMERA *cam, 
			 VECTOR *eyep, VECTOR *lookp, VECTOR *updir, 
			 float fov, int hres, int vres, RGB *background);

/* Computes camera cordinate system and horizontal and vertical fov depending
 * on filled in fov value and aspect ratio of the view window. Returns
 * NULL if this fails, and a pointer to the camera arg if succes. */
extern CAMERA *CameraComplete(CAMERA *camera);

/* sets only field-of-view, up-direction, focus point, camera position */
extern CAMERA *CameraSetFov(CAMERA *cam, float fov);
extern CAMERA *CameraSetUpdir(CAMERA *cam, float x, float y, float z);
extern CAMERA *CameraSetLookp(CAMERA *cam, float x, float y, float z);
extern CAMERA *CameraSetEyep(CAMERA *cam, float x, float y, float z);

/* moves and rotates the camera, e.g. as reaction to mouse movements on
 * the canvas window */
extern CAMERA *CameraMoveHorizontal(CAMERA *cam, float step);
extern CAMERA *CameraMoveForward(CAMERA *cam, float step);
extern CAMERA *CameraMoveRight(CAMERA *cam, float step);
extern CAMERA *CameraMoveUp(CAMERA *cam, float step);
extern CAMERA *CameraTurnRight(CAMERA *cam, float angle);
extern CAMERA *CameraTurnUp(CAMERA *cam, float angle);
extern CAMERA *CameraTilt(CAMERA *cam, float angle);
extern CAMERA *CameraZoom(CAMERA *cam, float amount);

/* camera postition etc.. can be saved on a stack of size MAXCAMSTACK. */
#define MAXCAMSTACK 20

/* saves/restores the virtual camera on/from the stack */
extern void CameraPush(CAMERA *cam);
extern CAMERA *CameraPop(CAMERA *cam);

/* returns pointer to the next saved camera. If previous==NULL, the first saved
 * camera is returned. In subsequent calls, the previous camera returned
 * by this function should be passed as the parameter. If all saved cameras
 * have been iterated over, NULL is returned. */
extern CAMERA *NextSavedCamera(CAMERA *previous);

extern void ParseCameraOptions(int *argc, char **argv);
extern void PrintCameraOptions(FILE *fp);
extern void CameraDefaults(void);

#ifdef __cplusplus
}
#endif

#endif /*CAMERA_H_*/
