

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "camera.h"
#include "error.h"
#include "vector.h"
#include "Boolean.h"
#include "pools.h"
#include "options.h"
#include "defaults.h"

CAMERA Camera;	
CAMERA AlternateCamera;       

#define NEWCAMERA()	(CAMERA *)Alloc(sizeof(CAMERA))
#define DISPOSECAMERA(ptr) Free((char *)ptr, sizeof(CAMERA))

CAMERA *CameraCreate(void)
{
  CAMERA *cam;
  cam = NEWCAMERA();
  *cam = Camera;

  return cam;
}

void CameraDestroy(CAMERA *cam)
{
  DISPOSECAMERA(cam);
}


static CAMERA CamStack[MAXCAMSTACK], *CamStackPtr=CamStack;

void CameraPrint(FILE *out, CAMERA *Camera)
{
  fprintf(out, "eyepoint: "); VectorPrint(out, Camera->eyep);
  fprintf(out, "\nlooking to: "); VectorPrint(out, Camera->lookp);
  fprintf(out, "\nupdir: "); VectorPrint(out, Camera->updir);
  fprintf(out, "\nfov = %f", (double)(Camera->fov));
  fprintf(out, "\nhres = %d, vres = %d", Camera->hres, Camera->vres);
  fprintf(out, "\npixh = %g, pixv = %g", Camera->pixh, Camera->pixv);
  fprintf(out, "\nbackground color: "); RGBPrint(out, Camera->background);
  fprintf(out, "\n");
}

void CameraDefaults(void)
{
  VECTOR eyep = DEFAULT_EYEP,
         lookp = DEFAULT_LOOKP,
         updir = DEFAULT_UPDIR;
  RGB    backgroundcolor = DEFAULT_BACKGROUND_COLOR;

  CameraSet(&Camera, &eyep, &lookp, &updir, DEFAULT_FOV, 600, 600, 
	    &backgroundcolor);

  AlternateCamera = Camera;
}

static void _setEyepOption(void *val)
{
  VECTOR *v = (VECTOR *)val;
  CameraSetEyep(&Camera, v->x, v->y, v->z);
}

static void _setLookpOption(void *val)
{
  VECTOR *v = (VECTOR *)val;
  CameraSetLookp(&Camera, v->x, v->y, v->z);
}

static void _setUpdirOption(void *val)
{
  VECTOR *v = (VECTOR *)val;
  CameraSetUpdir(&Camera, v->x, v->y, v->z);
}

static void _setFovOption(void *val)
{
  float *v = (float *)val;
  CameraSetFov(&Camera, *v);
}



static void _setAEyepOption(void *val)
{
  VECTOR *v = (VECTOR *)val;
  CameraSetEyep(&AlternateCamera, v->x, v->y, v->z);
}

static void _setALookpOption(void *val)
{
  VECTOR *v = (VECTOR *)val;
  CameraSetLookp(&AlternateCamera, v->x, v->y, v->z);
}

static void _setAUpdirOption(void *val)
{
  VECTOR *v = (VECTOR *)val;
  CameraSetUpdir(&AlternateCamera, v->x, v->y, v->z);
}

static void _setAFovOption(void *val)
{
  float *v = (float *)val;
  CameraSetFov(&AlternateCamera, *v);
}

static CMDLINEOPTDESC cameraOptions[] = {
  {"-eyepoint",	 4, TVECTOR,  &Camera.eyep,           _setEyepOption,
   "-eyepoint  <vector>\t: viewing position"},
  {"-center",    4, TVECTOR,  &Camera.lookp,          _setLookpOption,
   "-center    <vector>\t: point looked at"},
  {"-updir",     3, TVECTOR,  &Camera.updir,          _setUpdirOption,
   "-updir     <vector>\t: direction pointing up"},
  {"-fov",       4, Tfloat,   &Camera.fov,            _setFovOption,
   "-fov       <float> \t: field of view angle"},
  {"-aeyepoint", 4, TVECTOR,  &AlternateCamera.eyep,  _setAEyepOption,
   "-aeyepoint <vector>\t: alternate viewing position"},
  {"-acenter",   4, TVECTOR,  &AlternateCamera.lookp, _setALookpOption,
   "-acenter   <vector>\t: alternate point looked at"},
  {"-aupdir",    3, TVECTOR,  &AlternateCamera.updir, _setAUpdirOption,
   "-aupdir    <vector>\t: alternate direction pointing up"},
  {"-afov",      4, Tfloat,   &AlternateCamera.fov,   _setAFovOption,
   "-afov      <float> \t: alternate field of view angle"},
  {NULL,         0, TYPELESS, NULL,                   DEFAULT_ACTION,
   NULL }
};

void ParseCameraOptions(int *argc, char **argv)
{
  ParseOptions(cameraOptions, argc, argv);
}

void PrintCameraOptions(FILE *fp)
{
  fprintf(fp, "\nCamera options:\n");
  PrintOptions(fp, cameraOptions);
}


CAMERA *CameraSet(CAMERA *Camera, VECTOR *eyep, VECTOR *lookp, VECTOR *updir, 
		  float fov, int hres, int vres, RGB *background)
{
  Camera->eyep = *eyep;
  Camera->lookp = *lookp;
  Camera->updir = *updir;
  Camera->fov = fov;
  Camera->hres = hres;
  Camera->vres = vres;
  Camera->background = *background;
  Camera->changed = TRUE;

  CameraComplete(Camera);

  return Camera;
}

extern int CameraCompare(CAMERA *cam1, CAMERA *cam2)
{
  int result;

  result = VECTOREQUAL(cam1->eyep, cam2->eyep, EPSILON);

  if(result)
  {
    result = VECTOREQUAL(cam1->lookp, cam2->lookp, EPSILON);
    if(result)
    {
      result = VECTOREQUAL(cam1->updir, cam2->updir, EPSILON);
      if(result)
	result = FLOATEQUAL(cam1->fov, cam2->fov, EPSILON);
    }
  }

  return result;
}

void CameraComputeClippingPlanes(CAMERA *Camera)
{
  float x = Camera->tanhfov * Camera->viewdist;	
  float y = Camera->tanvfov * Camera->viewdist;	
  VECTOR vscrn[4];
  int i;

  VECTORCOMB3(Camera->lookp,  x, Camera->X, -y, Camera->Y, vscrn[0]); 
  VECTORCOMB3(Camera->lookp,  x, Camera->X,  y, Camera->Y, vscrn[1]); 
  VECTORCOMB3(Camera->lookp, -x, Camera->X,  y, Camera->Y, vscrn[2]); 
  VECTORCOMB3(Camera->lookp, -x, Camera->X, -y, Camera->Y, vscrn[3]); 

  for (i=0; i<4; i++) {
    VECTORTRIPLECROSSPRODUCT(vscrn[(i+1)%4], Camera->eyep, vscrn[i], Camera->viewplane[i].norm);
    VECTORNORMALIZE(Camera->viewplane[i].norm);
    Camera->viewplane[i].d = -VECTORDOTPRODUCT(Camera->viewplane[i].norm, Camera->eyep);
  }
}

CAMERA *CameraComplete(CAMERA *Camera)
{
  float n;

  
  VECTORSUBTRACT(Camera->lookp, Camera->eyep, Camera->Z);

  
  Camera->viewdist = VECTORNORM(Camera->Z);
  if (Camera->viewdist < EPSILON) {
    Error("SetCamera", "eyepoint and look-point coincide");
    return NULL;
  }
  VECTORSCALEINVERSE(Camera->viewdist, Camera->Z, Camera->Z);

  
  VECTORCROSSPRODUCT(Camera->Z, Camera->updir, Camera->X);
  n = VECTORNORM(Camera->X);
  if (n < EPSILON) {
    Error("SetCamera", "up-direction and viewing direction coincide");
    return NULL;
  }
  VECTORSCALEINVERSE(n, Camera->X, Camera->X);

  
  VECTORCROSSPRODUCT(Camera->Z, Camera->X, Camera->Y);
  VECTORNORMALIZE(Camera->Y);

  
  if (Camera->hres < Camera->vres) {
    Camera->hfov = Camera->fov; 
    Camera->vfov = atan(tan(Camera->fov * M_PI/180.) *
			(float)Camera->vres/(float)Camera->hres) * 180./M_PI;
  } else {
    Camera->vfov = Camera->fov; 
    Camera->hfov = atan(tan(Camera->fov * M_PI/180.) * 
			(float)Camera->hres/(float)Camera->vres) * 180./M_PI;
  }
  
  
  Camera->near = EPSILON;
  Camera->far = 2. * Camera->viewdist;

  
  Camera->tanhfov = tan(Camera->hfov * M_PI / 180.0);
  Camera->tanvfov = tan(Camera->vfov * M_PI / 180.0);

  Camera->pixh = 2.0 * Camera->tanhfov / (float)(Camera->hres);
  Camera->pixv = 2.0 * Camera->tanvfov / (float)(Camera->vres);

  

  CameraComputeClippingPlanes(Camera);

  return Camera;
}


CAMERA *CameraSetEyep(CAMERA *cam, float x, float y, float z)
{
  VECTOR neweyep;
  VECTORSET(neweyep, x, y, z);
  return CameraSet(cam, &neweyep, &cam->lookp, &cam->updir, 
		   cam->fov, cam->hres, cam->vres, &cam->background);	
}

CAMERA *CameraSetLookp(CAMERA *cam, float x, float y, float z)
{
  VECTOR newlookp;
  VECTORSET(newlookp, x, y, z);
  return CameraSet(cam, &cam->eyep, &newlookp, &cam->updir, 
		   cam->fov, cam->hres, cam->vres, &cam->background);	
}

CAMERA *CameraSetUpdir(CAMERA *cam, float x, float y, float z)
{
  VECTOR newupdir;
  VECTORSET(newupdir, x, y, z);
  return CameraSet(cam, &cam->eyep, &cam->lookp, &newupdir,
		   cam->fov, cam->hres, cam->vres, &cam->background);
}

CAMERA *CameraSetFov(CAMERA *cam, float fov)
{
  return CameraSet(cam, &cam->eyep, &cam->lookp, &cam->updir,
		   fov, cam->hres, cam->vres, &cam->background);
}

CAMERA *CameraMoveHorizontal(CAMERA *cam, float step)
{
  VECTOR neweyep, newlookp, dir;
  VECTORORTHOCOMP(cam->Z, cam->updir, dir);
  VECTORNORMALIZE(dir);

  VECTORSUMSCALED(cam->eyep, step, dir, neweyep);
  VECTORSUMSCALED(cam->lookp, step, dir, newlookp);
  return CameraSet(cam, &neweyep, &newlookp, &cam->updir, 
		   cam->fov, cam->hres, cam->vres, &cam->background);
}

CAMERA *CameraMoveForward(CAMERA *cam, float step)
{
  VECTOR neweyep, newlookp, dir;
  dir = cam->Z;

  VECTORSUMSCALED(cam->eyep, step, dir, neweyep);
  VECTORSUMSCALED(cam->lookp, step, dir, newlookp);
  return CameraSet(cam, &neweyep, &newlookp, &cam->updir, 
		   cam->fov, cam->hres, cam->vres, &cam->background);
}

CAMERA *CameraMoveRight(CAMERA *cam, float step)
{
  VECTOR neweyep, newlookp;
  
  VECTORSUMSCALED(cam->eyep, step, cam->X, neweyep);
  VECTORSUMSCALED(cam->lookp, step, cam->X, newlookp);
  return CameraSet(cam, &neweyep, &newlookp, &cam->updir, 
		   cam->fov, cam->hres, cam->vres, &cam->background);
}

CAMERA *CameraMoveUp(CAMERA *cam, float step)
{
  VECTOR neweyep, newlookp;
  
  VECTORSUMSCALED(cam->eyep, step, cam->Y, neweyep);
  VECTORSUMSCALED(cam->lookp, step, cam->Y, newlookp);
  return CameraSet(cam, &neweyep, &newlookp, &cam->updir, 
		   cam->fov, cam->hres, cam->vres, &cam->background);
}

CAMERA *CameraTurnRight(CAMERA *cam, float angle)
{
  VECTOR newlookp;
  float z=cam->viewdist*cos(angle), 
    x=cam->viewdist*sin(angle);
  
  VECTORCOMB3(cam->eyep, z, cam->Z, x, cam->X, newlookp);
  return CameraSet(cam, &cam->eyep, &newlookp, &cam->updir, 
		   cam->fov, cam->hres, cam->vres, &cam->background);
}

CAMERA *CameraTurnUp(CAMERA *cam, float angle)
{
  VECTOR newlookp;
  float z=cam->viewdist*cos(angle), 
    y=-cam->viewdist*sin(angle);
  
  VECTORCOMB3(cam->eyep, z, cam->Z, y, cam->Y, newlookp);
  return CameraSet(cam, &cam->eyep, &newlookp, &cam->updir, 
		   cam->fov, cam->hres, cam->vres, &cam->background);
}

CAMERA *CameraTilt(CAMERA *cam, float angle)
{
  VECTOR newupdir;
  float x, y, z, r;
  
  r=VECTORDOTPRODUCT(cam->updir, cam->Y);
  x=-r*sin(angle);
  y=r*cos(angle); 
  z=VECTORDOTPRODUCT(cam->updir, cam->Z);
  
  VECTORCOORD(x, cam->X, y, cam->Y, z, cam->Z, newupdir);
  return CameraSet(cam, &cam->eyep, &cam->lookp, &newupdir, 
		   cam->fov, cam->hres, cam->vres, &cam->background);
}

CAMERA *CameraZoom(CAMERA *cam, float amount)
{
  cam->fov /= amount;
  cam->hfov /= amount;
  cam->vfov /= amount;

  cam->changed = TRUE;
  return cam;
}


void CameraPush(CAMERA *cam)
{
  if (CamStackPtr - CamStack >= MAXCAMSTACK)
    Error("PushCamera", "Camera Stack depth exceeded");
  else
    *CamStackPtr++ = *cam;
}


CAMERA *CameraPop(CAMERA *cam)
{
  CAMERA poppedcam;

  if (CamStackPtr - CamStack <= 0)
    Error("PopCamera", "Camera Stack empty");
  else {
    poppedcam = *--CamStackPtr;
    cam = CameraSet(cam, 
		    &poppedcam.eyep, 
		    &poppedcam.lookp, 
		    &poppedcam.updir, 
		    poppedcam.fov, 
		    cam->hres, cam->vres, 
		    &poppedcam.background);
  }
  return cam;
}


CAMERA *NextSavedCamera(CAMERA *previous)
{
  CAMERA *cam = previous ? previous : CamStackPtr;
  cam--;
  return (cam < CamStack) ? (CAMERA *)NULL : cam;
}
