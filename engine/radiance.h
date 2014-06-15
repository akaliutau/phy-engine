/* radiance.h: declarations common to all radiance algorithms */

#ifndef _RADIANCE_H_
#define _RADIANCE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "patch_type.h"
#include "geom_type.h"
#include "material.h"

extern int interrupt_radiance; /* defined in ui_radiance.h */
  
  typedef COLOR(*GETRADIANCE_FT)(PATCH *patch, double u, double v, VECTOR dir);

/* routines to be implemented for each radiance algorithm */
typedef struct RADIANCEMETHOD {
  /* A one-word name for the method: among others used to select it using 
   * the radiance method with the -radiance command line option. */
  char *shortName;

  /* to how many characters can be abbreiviated the short name */
  int nameAbbrev;

  /* a longer name for the method */
  char *fullName;

  /* name for the Radiance methods menu button to activate the method */
  char *buttonName;

  /* a funciton to set default values for the method */
  void (*Defaults)(void);

  /* a function to parse command line arguments for the method. */
  void (*ParseOptions)(int *argc, char **argv);

  /* a function to print command line arguments for the method, invoked e.g.
   * using 'phy -help' */
  void (*PrintOptions)(FILE *fp);

  /* Initializes the current scene for radiance computations. Called when a new
   * scene is loaded or when selecting a particular radiance algorithm. */
  void	(*Initialize)(void);

  /* Does one step or iteration of the radiance computation, typically a unit
   * of computations after which the scene is to be redrawn. Returns TRUE when 
   * done. */
  int	(*DoStep)(void);

  /* Terminates radiance computations on the current scene */
  void (*Terminate)(void);

  /* Returns the radiance being emitted from the specified patch, at
   * the point with given (u,v) parameters and into the given direction. */
  /* COLOR (*GetRadiance)(PATCH *patch, double u, double v, VECTOR dir); */
  GETRADIANCE_FT GetRadiance;

  /* Allocates memory for the radiance data for the given patch. Fills in
  * the pointer in patch->radiance_data. */
  void *(*CreatePatchData)(PATCH *patch);

  /* Print radiance data for the patch to file out */
  void (*PrintPatchData)(FILE *out, PATCH *patch);

  /* destroys the radiance data for the patch. Clears the patch->radiance_data
   * pointer. */
  void (*DestroyPatchData)(PATCH *patch);

  /* A function to create a control panel for itneractively modifying
   * options. The parent widget is passed as a void *. It is of
   * type Widget, but using that type would require to include the
   * Xt include files, and we don't want to do that here (they are 
   * large). */
  void (*CreateControlPanel)(void *parent_widget);

  /* radiance method specific user interface routines */
  /* updates the control panel to correctly reflect radiance method
   * state variables. Can be NULL. */
  void (*UpdateControlPanel)(void *parent_widget);

  /* radiance method specific user interface routines */
  /* manages the control panel */
  void (*ShowControlPanel)(void);

  /* unmanages the control panel */
  void (*HideControlPanel)(void);

  /* returns a string with statistics information about the current run so 
   * far */
  char *(*GetStats)(void);

  /* Renders the scene using the specific data. This routine can be
   * a NULL pointer. In that case, the default hardware assisted rendering
   * method (in render.c) is used: render all the patches with the RGB color
   * triplet they were assigned. */
  void (*RenderScene)(void);

  /* routine for recomputing display colors if default 
   * RenderScene method is being used. */
  void (*RecomputeDisplayColors)(void);

  /* called when a material has been updated. The radiance method recomputes
   * the color of the affected surfaces and also of the other
   * surfaces, whose colors change because of interreflections. */
  void (*UpdateMaterial)(MATERIAL *oldmaterial, MATERIAL *newmaterial);

} RADIANCEMETHOD;

/* able of available radiance methods, terminated with a NULL pointer. */
extern RADIANCEMETHOD *RadianceMethods[];

/* iterator over all available radiance methods */
#define ForAllRadianceMethods(methodp) {{ 		\
  RADIANCEMETHOD **methodpp;				\
  for (methodpp=RadianceMethods; *methodpp; methodpp++) { \
    RADIANCEMETHOD *methodp = *methodpp;
#ifndef EndForAll
#define EndForAll }}}
#endif

/* pointer to current radiance method handle */
extern RADIANCEMETHOD *Radiance;

/* This routine sets the current radiance method to be used + initializes */
extern void SetRadianceMethod(RADIANCEMETHOD *newmethod);

/* Initializes. Called from Init() in main.c. */
extern void RadianceDefaults(void);

/* Parses (and consumes) command line options for radiance
 * computation. */
extern void ParseRadianceOptions(int *argc, char **argv);

/* Prints world-space radiance computation command line option usage */
extern void PrintRadianceOptions(FILE *fp);

#ifdef __cplusplus
}
#endif

#endif /*_RADIANCE_H_*/
