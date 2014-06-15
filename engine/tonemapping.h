/* tonemapping.h: tone mapping */

#ifndef _PHY_TONE_MAPPING_
#define _PHY_TONE_MAPPING_

#include <stdio.h>
#include "color.h"
#include "adaptation.h"

/* struct describing a tone map. Most of the functions have similar meaning
 * as for a radiance or ray-tracing method */
typedef struct TONEMAP {
  char *name, 		/* full name */
    *shortName, 	/* short name useable as option argument */
    *buttonName;	/* UI button name, determines button resources */
  int abbrev;		/* minimal abbreviation of short name in option arg. */

  void (*Defaults)(void);				/* sets defaults */
  void (*ParseOptions)(int *argc, char **argv);		/* optional */
  void (*PrintOptions)(FILE *fp);			/* optional */

  void (*Init)(void);					/* initialises */
  void (*Terminate)(void);				/* terminates */

/* ---------------------------------------------------------------------------
  `TonemapReverseScaleForComputations'

  Knowing the display luminance "dl" this function determines the
  correct scaling value that transforms display luminance back into
  the real world luminance.
  ------------------------------------------------------------------------- */
  COLOR (*ScaleForComputations)(COLOR radiance);

/* ---------------------------------------------------------------------------
  `TonemapScaleForDisplay'

  Full tonemapping to display values. Transforms real world luminance of
  colour specified by "radiance" into corresponding display input
  values. The result has to be clipped to <0,1> afterwards.
  ------------------------------------------------------------------------- */
  COLOR (*ScaleForDisplay)(COLOR radiance);

/* ---------------------------------------------------------------------------
  `TonemapReverseScaleForComputations'

  Knowing the display luminance "dl" this function determines the
  correct scaling value that transforms display luminance back into
  the real world luminance.
  ------------------------------------------------------------------------- */
  float	(*ReverseScaleForComputations)(float dl);

  /* the GUI routines below are currently unused */
  void (*CreateControlPanel)(void *parent_widget);	/* optional */
  void (*UpdateControlPanel)(void *parent_widget);	/* optional */
  void (*ShowControlPanel)(void);			/* optional */
  void (*HideControlPanel)(void);			/* optional */
} TONEMAP;

/* available tone mapping operators (NULL terminated array) */
extern TONEMAP *AvailableToneMaps[];

/* iterates over all available tone maps */
#define ForAllAvailableToneMaps(map)  {{ 	\
  TONEMAP **mapp;				\
  for (mapp=AvailableToneMaps; *mapp; mapp++) { \
    TONEMAP *map = *mapp;

#ifndef EndForAll
#define EndForAll }}}
#endif

/* makes 'map' the current tone map and initialises. */
extern void SetToneMap(TONEMAP *map);

/* gamma correction table */
#define GAMMATAB_BITS  12  /* 12-bit gamma table */
#define GAMMATAB_SIZE  ((1<<GAMMATAB_BITS)+1)

/* convert RGB color value between 0 and 1 to entry index in gamma table */
#define GAMMATAB_ENTRY(x)  (int)((x)*(float)(1<<GAMMATAB_BITS))

/* recomputes gamma tables for the given gamma values for 
 * red, green and blue */
extern void RecomputeGammaTables(RGB gamma);

/* Displays a test image for testing/determining gamma correction factors */
extern void RenderGammaTestImage(int which);

/* which can take the following values: */
#define TESTIMG1	1		/* useless */
#define TESTIMG2	2		/* useless */
#define GAMMA_TEST_IMAGE	3	/* RGB stripes versus half intensity */
#define BLACK_LEVEL_IMAGE	4	/* black level test image */

/* tone mapping global variables */
typedef struct TONEMAPPINGCONTEXT {
  /* fixed radiance rescaling before tone mapping */
  float 	brightness_adjust, 	/* brightness adjustment factor */
                pow_bright_adjust; 	/* pow(2, brightness_adjust) */

  /* variable/non-linear radiance rescaling */
  TONEMAP *	ToneMap;		/* current tone mapping operator */
  TMA_METHOD	statadapt;		/* static adaptation method */
  float 	lwa,                 	/* jp: real world adaption luminance */
                ldm,                 	/* jp: maximum display luminance */
                cmax;                	/* jp: maximum display contrast */

  /* conversion from radiance (COLOR type) to display RGB */
  float	    	xr, yr, xg, yg, xb, yb,	/* monitor primary colors */
                xw, yw;			/* monitor white point */

  /* display RGB mapping (corrects display non-linear response) */
  RGB   	gamma;                  /* gamma factors for red, green, blue */
  float         gammatab[3][GAMMATAB_SIZE]; /* gamma correction tables for red, green and blue */
  int           display_test_image;     /* gamma correction test image */
  int           testimg;                /* which test image (index). See gamma.c */
} TONEMAPPINGCONTEXT;
extern TONEMAPPINGCONTEXT tmopts;

/* defaults and option handling */
extern void ToneMapDefaults(void);
extern void ParseToneMapOptions(int *argc, char **argv);
extern void PrintToneMapOptions(FILE *fp);

/* initialises tone mapping for a new scene e.g. */
extern void InitToneMapping(void);

/* gamma correction */
#define RGBGAMMACORRECT(rgb) {\
  (rgb).r = tmopts.gammatab[0][GAMMATAB_ENTRY((rgb).r)]; \
  (rgb).g = tmopts.gammatab[1][GAMMATAB_ENTRY((rgb).g)]; \
  (rgb).b = tmopts.gammatab[2][GAMMATAB_ENTRY((rgb).b)]; \
}

/* shortcuts */
#define TonemapScaleForComputations(radiance)  (tmopts.ToneMap->ScaleForComputations(radiance))
#define TonemapScaleForDisplay(radiance)  (tmopts.ToneMap->ScaleForDisplay(radiance))
#define TonemapReverseScaleForComputations(dl)  (tmopts.ToneMap->ReverseScaleForComputations(dl))

/* ---------------------------------------------------------------------------
  `ContrastSensitivity'

  Returns the normalised sensitivity to contrast changes along the line
  from "p1" to "p2" in case that the points are viewed from the eye point
  for the current camera.
  ------------------------------------------------------------------------- */
extern float ContrastSensitivity(POINT *p1, POINT *p2);

/* ---------------------------------------------------------------------------
  `ContrastSensitivityEye'

  Eyepoint for contrast sensitivity computations.
  ------------------------------------------------------------------------- */
extern void ContrastSensitivityEye(POINT *p);


/* ---------------------------------------------------------------------------
  `TMO_COLOR_COMP'

  Scales the given radiance "_c" so that the luminance of the result
  corresponds to the luminance that would be emitted by the display
  device displaying this color.
  ------------------------------------------------------------------------- */
#define TMO_COLOR_COMP(_c) \
	_c = TonemapScaleForComputations(_c)
/* ---------------------------------------------------------------------------
  `TMO_COLOR_DISP'

  Scales the given radiance "_c" so that the result lays in [0,1] range.
  ------------------------------------------------------------------------- */
#define TMO_COLOR_DISP(_c) \
	_c = TonemapScaleForDisplay(_c)

/* ---------------------------------------------------------------------------
  `TMO_COLOR_COMP_CLIPPED'

  This is something that I am again not sure about: We need clipped
  luminance value in cd/m^2 of display output - this has to be clipped
  by "ldmax" as the display device can not produce higher luminance. I
  am clipping the Y value and scaling the X and Z so that the
  correspoding x,y stays the same. But:

  * shall we not simply set it to maximum white? 
  * what about gamut clipping in this case?
  * is gamma correction necessary? (probably no, as it is just a means to
    account for nonlinear response of the given display device; luminances
    reproduced shall be OK.

  Moreover, the effectivity of this piece of code is not very high. Blah.
  ------------------------------------------------------------------------- */
#define TMO_COLOR_COMP_CLIPPED(_c, _gamma) \
	do { \
          float _b, _ldmax; \
          TMO_COLOR_COMP(_c); /* result is in cd/m2 */ \
	  _b = ColorGray(_c); /* achromatic luminance */ \
	  _ldmax = TonemapLdmax(); \
          if (_b > _ldmax) { COLORSCALE(_ldmax/_b, _c, _c); } \
	} while(0)

/* ---------------------------------------------------------------------------
  `TMO_CANDELA_LAMBERT'

  Transforms luminance from cd/m^2 to lamberts. Luminance in lamberts
  is needed for example by algorithms that are based on experiments of
  Stevens and Stevens (original Tumblin-Rushmeier tone operator). The
  transformation rule comes from Glassner's book, table 13.3, seems to
  be OK.
  ------------------------------------------------------------------------- */
#define TMO_CANDELA_LAMBERT(_a) ((_a) * M_PI * 1e-4)

/* ---------------------------------------------------------------------------
  `TMO_LAMBERT_CANDELA'

  Transforms luminance from lamberts to cd/m^2 to lamberts.
  ------------------------------------------------------------------------- */
#define TMO_LAMBERT_CANDELA(_a) ((_a) / (M_PI * 1e-4))

#endif /*_PHY_TONE_MAPPING_*/
