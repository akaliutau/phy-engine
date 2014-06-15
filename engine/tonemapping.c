

#include <string.h>

#include "tonemapping.h"
#include "options.h"
#include "error.h"
#include "defaults.h"
#include "cie.h"
#include "extmath.h"

#include "tonemapping/dummy.h"
#include "tonemapping/lightness.h"
#include "tonemapping/trwf.h"


TONEMAP *AvailableToneMaps[] = {
  &TM_Lightness,
  &TM_TumblinRushmeier,
  &TM_Ward,
  &TM_RevisedTumblinRushmeier,
  &TM_Ferwerda,
  (TONEMAP *)NULL		
};


TONEMAPPINGCONTEXT tmopts;


static char tonemapping_methods_string[1000];
static void make_tonemapping_methods_string(void)
{
  char *str = tonemapping_methods_string;
  int n, first = TRUE;
  sprintf(str, "\
-tonemapping <method>: Set tone mapping method\n%n",
	  &n); str += n;
  sprintf(str,   "\tmethods: %n", &n); str += n;
  
  ForAllAvailableToneMaps(method) {
    if (!first) {
      sprintf(str, "\t         %n", &n); str += n;
    }
    first = FALSE;
    sprintf(str, "%-20.20s %s%s\n%n",
	    method->shortName, method->name,
	    tmopts.ToneMap==method ? " (default)" : "", &n); str += n;
  } EndForAll;
  *(str-1) = '\0';	
}

static void ToneMappingMethodOption(void *value)
{
  char *name = *(char **)value;

  ForAllAvailableToneMaps(method) {
    if (strncasecmp(name, method->shortName, method->abbrev) == 0) {
      SetToneMap(method);
      return;
    }
  } EndForAll;

  Error(NULL, "Invalid tone mapping method name '%s'", name);
}


static void BrightnessAdjustOption(void *val)
{
  tmopts.pow_bright_adjust = pow(2., tmopts.brightness_adjust);  
}

static float rxy[2], gxy[2], bxy[2], wxy[2];

static void ChromaOption(void *value)
{
  float *chroma = (float *)value;
  if (chroma == rxy) {
    tmopts.xr = chroma[0]; tmopts.yr = chroma[1];
  } else if (chroma == gxy) {
    tmopts.xg = chroma[0]; tmopts.yg = chroma[1];
  } else if (chroma == bxy) {
    tmopts.xb = chroma[0]; tmopts.yb = chroma[1];
  } else if (chroma == wxy) {
    tmopts.xw = chroma[0]; tmopts.yw = chroma[1];
  } else {
    Fatal(-1, "ChromaOption", "invalid value pointer");
  }

  ComputeColorConversionTransforms(tmopts.xr, tmopts.yr,
				   tmopts.xg, tmopts.yg,
				   tmopts.xb, tmopts.yb,
				   tmopts.xw, tmopts.yw);
}

static void _tmAdaptMethodOption(void *value)
{
  char *name = *(char **)value;

  if (strncasecmp(name, "average", 2) == 0)
    tmopts.statadapt = TMA_AVERAGE;
  else if (strncasecmp(name, "median", 2) == 0)
    tmopts.statadapt = TMA_MEDIAN;
  else
  {
    Error(NULL,
	  "Invalid adaptation estimate method '%s'",
	  name);
  }
}

static void GammaOption(void *value)
{
  float gam = *(float *)value;
  RGBSET(tmopts.gamma, gam, gam, gam);
}

static CMDLINEOPTDESC tmOptions[] = {
  {"-tonemapping", 		4, 	Tstring, 	
   NULL,			ToneMappingMethodOption,
   tonemapping_methods_string },
  {"-brightness-adjust", 	4,  	Tfloat,	
   &tmopts.brightness_adjust,	BrightnessAdjustOption,
   "-brightness-adjust <float> : brightness adjustment factor"},
  {"-adapt",			5,	Tstring,
   NULL,			_tmAdaptMethodOption,
   "-adapt <method>  \t: adaptation estimation method\n"
   "\tmethods: \"average\", \"median\""},
  {"-lwa",			3,	Tfloat,
   &tmopts.lwa,			DEFAULT_ACTION,
   "-lwa <float>\t\t: real world adaptation luminance"},
  {"-ldmax",			5,	Tfloat,
   &tmopts.ldm,			DEFAULT_ACTION,
   "-ldmax <float>\t\t: maximum diaply luminance"},
  {"-cmax",			4,	Tfloat,
   &tmopts.cmax,		DEFAULT_ACTION,
   "-cmax <float>\t\t: maximum displayable contrast"},
  {"-gamma",			4,	 Tfloat,	
   NULL,		        GammaOption,
   "-gamma <float>       \t: gamma correction factor (same for red, green. blue)"},
  {"-rgbgamma",			4,	 TRGB,	
   &tmopts.gamma,		DEFAULT_ACTION,
   "-rgbgamma <r> <g> <b>\t: gamma correction factor (separate for red, green, blue)"},
  {"-red",			4,	Txy,		
   rxy,				ChromaOption,
   "-red <xy>            \t: CIE xy chromaticity of monitor red"},
  {"-green",			4,	Txy,		
   gxy,				ChromaOption,
   "-green <xy>          \t: CIE xy chromaticity of monitor green"},
  {"-blue",			4,	Txy,		
   bxy,				ChromaOption,
   "-blue <xy>           \t: CIE xy chromaticity of monitor blue"},
  {"-white",			4,	Txy,		
   wxy,				ChromaOption,
   "-white <xy>          \t: CIE xy chromaticity of monitor white"},
  {NULL,			0,	 TYPELESS,	
   NULL,			DEFAULT_ACTION,
   NULL}
};


void ToneMapDefaults(void)
{
  ForAllAvailableToneMaps(map) {
    map->Defaults();
  } EndForAll;

  tmopts.brightness_adjust = 0.;
  tmopts.pow_bright_adjust = pow(2., tmopts.brightness_adjust);

  tmopts.statadapt = TMA_MEDIAN;
  tmopts.lwa       = DEFAULT_TM_LWA;
  tmopts.ldm       = DEFAULT_TM_LDMAX;
  tmopts.cmax      = DEFAULT_TM_CMAX;

  rxy[0] = tmopts.xr = 0.640; rxy[1] = tmopts.yr = 0.330;
  gxy[0] = tmopts.xg = 0.290; gxy[1] = tmopts.yg = 0.600;
  bxy[0] = tmopts.xb = 0.150; bxy[1] = tmopts.yb = 0.060;
  wxy[0] = tmopts.xw = 0.333333333333; wxy[1] = tmopts.yw = 0.333333333333;
  ComputeColorConversionTransforms(tmopts.xr, tmopts.yr,
				   tmopts.xg, tmopts.yg,
				   tmopts.xb, tmopts.yb,
				   tmopts.xw, tmopts.yw);

  RGBSET(tmopts.gamma, DEFAULT_GAMMA, DEFAULT_GAMMA, DEFAULT_GAMMA);
  RecomputeGammaTables(tmopts.gamma);
  tmopts.display_test_image = FALSE;
  tmopts.testimg = 3;

  tmopts.ToneMap = &TM_Lightness;
  tmopts.ToneMap->Init();

  make_tonemapping_methods_string();
}

void ParseToneMapOptions(int *argc, char **argv)
{
  ParseOptions(tmOptions, argc, argv);
  RecomputeGammaTables(tmopts.gamma);

  ForAllAvailableToneMaps(map) {
    if (map->ParseOptions) 
      map->ParseOptions(argc, argv);
  } EndForAll;
}

void PrintToneMapOptions(FILE *fp)
{
  fprintf(fp, "\nTone mapping options:\n");
  PrintOptions(fp, tmOptions);

  ForAllAvailableToneMaps(map) {
    if (map->PrintOptions)
      map->PrintOptions(fp);
  } EndForAll;
}


void SetToneMap(TONEMAP *map)
{
  tmopts.ToneMap->Terminate();
  tmopts.ToneMap = map ? map : &TM_Dummy;
  tmopts.ToneMap->Init();
}


void InitToneMapping(void)
{
  InitSceneAdaptation();
  SetToneMap(tmopts.ToneMap);
}

void RecomputeGammaTable(int index, double gamma)
{
  int i;
  if (gamma <= EPSILON) gamma = 1.;
  for (i=0; i<=(1<<GAMMATAB_BITS); i++) {
    tmopts.gammatab[index][i] = pow((double)i / (double)(1<<GAMMATAB_BITS), 1./gamma);
  }
}


void RecomputeGammaTables(RGB gamma)
{
  RecomputeGammaTable(0, gamma.r);
  RecomputeGammaTable(1, gamma.g);
  RecomputeGammaTable(2, gamma.b);
}


static POINT _cs_eye = {0.0, 0.0, 0.0};

static float _daly_csf(float spf)
{
    if (spf > 0.0)
    {
        float tmp = 0.3*spf;
        return pow(1.0+0.008/(spf*spf*spf),-0.2) * 1.42*spf*exp(-tmp) * 
               sqrt(1.0+0.6*exp(tmp));
    }
    
    return 0.0;
}

static float _ms_csf(float spf)
{
    if (spf > 0.0)
    {
        float tmp = 0.114*spf;
        return 2.6*(0.0192+tmp)*exp(-pow(tmp,1.1));
    }

    return 0.0;
}

float ContrastSensitivity(POINT *p1, POINT *p2)
{
  VECTOR a1, a2;
  float sf;

  VECTORSUBTRACT(*p1, _cs_eye, a1);
  VECTORSUBTRACT(*p2, _cs_eye, a2);

  VECTORNORMALIZE(a1);
  VECTORNORMALIZE(a2);
  
  sf = M_PI/(360.0 * acos(VECTORDOTPRODUCT(a1,a2)));
  return _ms_csf(sf);
}

void ContrastSensitivityEye(POINT *p)
{
  _cs_eye = *p;
}
