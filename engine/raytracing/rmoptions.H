/* rmoptions.H: Options and global state vars for raymatting */

#ifndef _RMOPTIONS_H_
#define _RMOPTIONS_H_

#include "screenbuffer.H"

/*** Typedefs & enums ***/

typedef enum
{
  RM_BOX_FILTER,
  RM_TENT_FILTER,
  RM_GAUSS_FILTER,
  RM_GAUSS2_FILTER
} RM_FILTER_OPTION;

/*** The global state structure ***/

typedef struct RM_State {

  /* Pixel sampling */
  int samplesPerPixel;

  /* pixel filter */
  RM_FILTER_OPTION filter;

} RM_State;


/*** The global state ***/

extern RM_State rms;



/*** Function prototypers ***/

void RM_Defaults(void);
void RM_ParseOptions(int *argc, char **argv);
void RM_PrintOptions(FILE *fp);

#endif
