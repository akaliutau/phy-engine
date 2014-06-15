/* raytrace.h: raytracing routines */

#ifndef _RAYTRACE_H_
#define _RAYTRACE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "image.h"

typedef struct RAYTRACINGMETHOD {
  /* short name for the raytracing method, for use as argument of
   * -raytracing command line optoin. */
  char *shortName;

  /* how short can the short name be abbreviated? */
  int nameAbbrev;

  /* full name of the raytracing method */
  char *fullName;

  /* name for the button for activating the method in
   * the raytracing method menu */
  char *buttonName;

  /* function for setting default values etc... */
  void (*Defaults)(void);

  /* funciton for creating user interface panel for interactively
   * setting method specific parameters  parent widget is
   * passed as a void *. The type is however 'Widget', but we don't
   * want to include the Xt header files in interface-independent files. */
  void (*CreateControlPanel)(void *parent_widget);

  /* function for parsing method specific command line options */
  void (*ParseOptions)(int *argc, char **argv);

  /* function for printing method specific command line options */
  void (*PrintOptions)(FILE *fp);

  /* Initializes the current scene for raytracing computations. 
   *  Called when a new scene is loaded or when selecting a particular 
   * raytracing algorithm. */
  void	(*Initialize)(void);

  /* Raytrace the current scene as seen with the current camera. If 'ip'
   * is not a NULL pointer, write the raytraced image using the image output
   * handle pointerd to by 'ip' . */
  void (*Raytrace)(ImageOutputHandle *ip);

  /* determines static adaptation luminance after scaling with the given
   * factor. Uses the current adaptation estimation strategy for tone
   * mapping. */
  double (*AdaptationLuminance)(float scalefactor);

  /* recomputes display colors based on stored pixel radiance values. Called
   * after changing the tone mapping function. */
  void (*RecomputeDisplayColors)(void);

  /* Redisplays last raytraced image. Returns FALSE if there is no
   * previous raytraced image and TRUE there is. */
  int (*Redisplay)(void);

  /* Reprojects the raytraced image after a viewing change (current camera
   * is taken for the new camera). Returns TRUE or FALSE like Redisplay(). */
  int (*Reproject)(void);

  /* Saves last raytraced image in the file describe dby the image output
   * handle. */
  int (*SaveImage)(ImageOutputHandle *ip);

  /* Interrupts raytracing */
  void (*InterruptRayTracing)(void);

  /* raytracing method specific user interface routines */
  /* manages the control panel */
  void (*ShowControlPanel)(void);

  /* unmanages the control panel */
  void (*HideControlPanel)(void);

  /* terminate raytracing computations */
  void (*Terminate)(void);
} RAYTRACINGMETHOD;

/* table of available raytracing methods, NULL-terminated */
extern RAYTRACINGMETHOD *RayTracingMethods[];

/* iterator over all available raytracing methods */
#define ForAllRayTracingMethods(methodp) {{ 		\
  RAYTRACINGMETHOD **methodpp;				\
  for (methodpp=RayTracingMethods; *methodpp; methodpp++) { \
    RAYTRACINGMETHOD *methodp = *methodpp;
#ifndef EndForAll
#define EndForAll }}}
#endif

/* current raytracing method */
extern RAYTRACINGMETHOD *RayTracing;

/* does raytracing */
extern void RayTrace(char *fname, FILE *fp, int ispipe);

/* initializes and makes current a new raytracing method */
extern void SetRayTracing(RAYTRACINGMETHOD *method);

/* raytracing statistics --- remains of prehistoric raytracing interface --- 
 * printing raytracing statistics will one day become a method similar
 * as for world-space radiance algorithms. */
extern double rt_total_time;	/* raytracing time */
extern long rt_raycount, rt_pixcount; /* number of rays traced and pixels drawn */

/* command line option processing */
extern void ParseRayTracingOptions(int *argc, char **argv);
extern void PrintRayTracingOptions(FILE *fp);

/* initializations, called from Init() in main.c */
extern void RayTracingDefaults(void);

#ifdef __cplusplus
}
#endif

#endif /*_RAYTRACE_H_*/
