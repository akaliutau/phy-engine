/* rtsoptions.H: Options and global state vars for 
   stochastic raytracing */

#ifndef _RTSOPTIONS_H_
#define _RTSOPTIONS_H_

#include "screenbuffer.H"

/*** Typedefs & enums ***/

typedef enum
{
  STORED_RADIANCE,
  NEXTEVENT_RADIANCE,
  IMPORTANT_LIGHT,
  PHOTONMAP_METHOD
} RTSRADIANCE_OPTION;

typedef enum 
{
  UNIFORMSAMPLING, COSINUSSAMPLING, BRDFSAMPLING, CLASSICALSAMPLING,
  PHOTONMAPSAMPLING
} RTSSAMPLING_MODE;

typedef enum 
{
  POWER_LIGHTS, IMPORTANT_LIGHTS, ALL_LIGHTS
} RTSLIGHT_MODE;

typedef enum 
{
  STORED_NONE, STORED_DIRECT, STORED_INDIRECT, STORED_PHOTONMAP
} RTSRAD_MODE;

typedef enum
{
  BACKGROUND_SAMPLE, BACKGROUND_DIRECT, BACKGROUND_INDIRECT
} RTSBACKGROUND_OPTION;

/*** The global state structure ***/

typedef struct RTStochastic_State {

  /* Pixel sampling */
  int samplesPerPixel;
  int progressiveTracing;

  int doFrameCoherent;
  int doCorrelatedSampling;
  long int baseSeed;

  int doTexFilter;
  int doGrad;
  float gradEps;

  /* Stored radiance handling */
  RTSRAD_MODE radMode;

  /* Direct Light sampling */
  int nextEvent;
  int nextEventSamples;
  RTSLIGHT_MODE lightMode;

  /* Background */
  int backgroundDirect;
  int backgroundIndirect;
  int backgroundSampling;

  /* Scattering */
  int scatterSamples;
  int differentFirstDG;
  int firstDGSamples;
  int separateSpecular;
  RTSSAMPLING_MODE reflectionSampling;

  int minPathDepth;
  int maxPathDepth;

  CScreenBuffer *lastscreen;

} RTStochastic_State;


/*** The global state ***/

extern RTStochastic_State rts;



/*** Function prototypers ***/

void RTStochasticDefaults(void);
void RTStochasticParseOptions(int *argc, char **argv);
void RTStochasticPrintOptions(FILE *fp);

#endif
