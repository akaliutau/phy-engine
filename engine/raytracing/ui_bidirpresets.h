/* Preset definitions */

#include "bidiroptions.H"

#define PRESET_UNCHANGED -1 /* Leave current value unchanged */
#define PRESET_DERIVE -2 /* Derive value from other config values */
#define STRING_UNCHANGED "@"

static BP_BASECONFIG bidirPresets[] =
{
  /* Preset : Startup defaults */
  {
    PRESET_DERIVE,/* maximumEyePathDepth */
    PRESET_DERIVE,/* maximumLightPathDepth */
    7,/* maximumPathDepth */
    2,/* minimumPathDepth */
    1,/* samplesPerPixel */
    0,/* totalSamples */
    TRUE,/* sampleImportantLights */
    TRUE,/* progressiveTracing */
    FALSE,/* eliminateSpikes */
    FALSE,/* useSpars */
    TRUE,/* doLe */
    FALSE,/* doLD */
    FALSE,/* doLI */
    "(LX)(X)*(EX)",/* leRegExp[MAX_REGEXP_SIZE] */
    "(LX)(G|S)(X)*(EX),(LX)(EX)",/* ldRegExp[MAX_REGEXP_SIZE] */
    "(LX)(G|S)(X)*(EX),(LX)(EX)",/* liRegExp[MAX_REGEXP_SIZE] */
    FALSE,/* doWeighted */
    TRUE,/* doWLe */
    TRUE,/* doWLD */
    "(LX)(DR)(X)*(EX)",/* wleRegExp[MAX_REGEXP_SIZE] */
    "(LX)(X)*(EX)",/* wldRegExp[MAX_REGEXP_SIZE] */
  },
  /* Preset : Default BPT */
  {
    PRESET_DERIVE,/* maximumEyePathDepth */
    PRESET_DERIVE,/* maximumLightPathDepth */
    PRESET_UNCHANGED,/* maximumPathDepth */
    PRESET_UNCHANGED,/* minimumPathDepth */
    PRESET_UNCHANGED,/* samplesPerPixel */
    PRESET_UNCHANGED,/* totalSamples */
    PRESET_UNCHANGED,/* sampleImportantLights */
    PRESET_UNCHANGED,/* progressiveTracing */
    PRESET_UNCHANGED,/* eliminateSpikes */
    FALSE,/* useSpars */
    PRESET_UNCHANGED,/* doLe */
    PRESET_UNCHANGED,/* doLD */
    PRESET_UNCHANGED,/* doLI */
    STRING_UNCHANGED,/* leRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* ldRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* liRegExp[MAX_REGEXP_SIZE] */
    PRESET_UNCHANGED,/* doWeighted */
    PRESET_UNCHANGED,/* doWLe */
    PRESET_UNCHANGED,/* doWLD */
    STRING_UNCHANGED,/* wleRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* wldRegExp[MAX_REGEXP_SIZE] */
  },
  /* Preset : Default PT */
  {
    PRESET_DERIVE,/* maximumEyePathDepth */
    1,/* maximumLightPathDepth */
    PRESET_UNCHANGED,/* maximumPathDepth */
    PRESET_UNCHANGED,/* minimumPathDepth */
    PRESET_UNCHANGED,/* samplesPerPixel */
    PRESET_UNCHANGED,/* totalSamples */
    PRESET_UNCHANGED,/* sampleImportantLights */
    PRESET_UNCHANGED,/* progressiveTracing */
    PRESET_UNCHANGED,/* eliminateSpikes */
    FALSE,/* useSpars */
    PRESET_UNCHANGED,/* doLe */
    PRESET_UNCHANGED,/* doLD */
    PRESET_UNCHANGED,/* doLI */
    STRING_UNCHANGED,/* leRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* ldRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* liRegExp[MAX_REGEXP_SIZE] */
    PRESET_UNCHANGED,/* doWeighted */
    PRESET_UNCHANGED,/* doWLe */
    PRESET_UNCHANGED,/* doWLD */
    STRING_UNCHANGED,/* wleRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* wldRegExp[MAX_REGEXP_SIZE] */
  },
  /* Preset : Default LT */
  {
    1,/* maximumEyePathDepth */
    PRESET_DERIVE,/* maximumLightPathDepth */
    PRESET_UNCHANGED,/* maximumPathDepth */
    PRESET_UNCHANGED,/* minimumPathDepth */
    PRESET_UNCHANGED,/* samplesPerPixel */
    PRESET_UNCHANGED,/* totalSamples */
    PRESET_UNCHANGED,/* sampleImportantLights */
    PRESET_UNCHANGED,/* progressiveTracing */
    PRESET_UNCHANGED,/* eliminateSpikes */
    FALSE,/* useSpars */
    PRESET_UNCHANGED,/* doLe */
    PRESET_UNCHANGED,/* doLD */
    PRESET_UNCHANGED,/* doLI */
    STRING_UNCHANGED,/* leRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* ldRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* liRegExp[MAX_REGEXP_SIZE] */
    PRESET_UNCHANGED,/* doWeighted */
    PRESET_UNCHANGED,/* doWLe */
    PRESET_UNCHANGED,/* doWLD */
    STRING_UNCHANGED,/* wleRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* wldRegExp[MAX_REGEXP_SIZE] */
  },
  /* Preset : Set to BPT */
  {
    PRESET_DERIVE,/* maximumEyePathDepth */
    PRESET_DERIVE,/* maximumLightPathDepth */
    PRESET_UNCHANGED,/* maximumPathDepth */
    PRESET_UNCHANGED,/* minimumPathDepth */
    PRESET_UNCHANGED,/* samplesPerPixel */
    PRESET_UNCHANGED,/* totalSamples */
    PRESET_UNCHANGED,/* sampleImportantLights */
    PRESET_UNCHANGED,/* progressiveTracing */
    PRESET_UNCHANGED,/* eliminateSpikes */
    PRESET_UNCHANGED,/* useSpars */
    PRESET_UNCHANGED,/* doLe */
    PRESET_UNCHANGED,/* doLD */
    PRESET_UNCHANGED,/* doLI */
    STRING_UNCHANGED,/* leRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* ldRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* liRegExp[MAX_REGEXP_SIZE] */
    PRESET_UNCHANGED,/* doWeighted */
    PRESET_UNCHANGED,/* doWLe */
    PRESET_UNCHANGED,/* doWLD */
    STRING_UNCHANGED,/* wleRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* wldRegExp[MAX_REGEXP_SIZE] */
  },
  /* Preset : Set to PT */
  {
    PRESET_DERIVE,/* maximumEyePathDepth */
    1,/* maximumLightPathDepth */
    PRESET_UNCHANGED,/* maximumPathDepth */
    PRESET_UNCHANGED,/* minimumPathDepth */
    PRESET_UNCHANGED,/* samplesPerPixel */
    PRESET_UNCHANGED,/* totalSamples */
    PRESET_UNCHANGED,/* sampleImportantLights */
    PRESET_UNCHANGED,/* progressiveTracing */
    PRESET_UNCHANGED,/* eliminateSpikes */
    PRESET_UNCHANGED,/* useSpars */
    PRESET_UNCHANGED,/* doLe */
    PRESET_UNCHANGED,/* doLD */
    PRESET_UNCHANGED,/* doLI */
    STRING_UNCHANGED,/* leRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* ldRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* liRegExp[MAX_REGEXP_SIZE] */
    PRESET_UNCHANGED,/* doWeighted */
    PRESET_UNCHANGED,/* doWLe */
    PRESET_UNCHANGED,/* doWLD */
    STRING_UNCHANGED,/* wleRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* wldRegExp[MAX_REGEXP_SIZE] */
  },
  /* Preset : Set to LT */
  {
    1,/* maximumEyePathDepth */
    PRESET_DERIVE,/* maximumLightPathDepth */
    PRESET_UNCHANGED,/* maximumPathDepth */
    PRESET_UNCHANGED,/* minimumPathDepth */
    PRESET_UNCHANGED,/* samplesPerPixel */
    PRESET_UNCHANGED,/* totalSamples */
    PRESET_UNCHANGED,/* sampleImportantLights */
    PRESET_UNCHANGED,/* progressiveTracing */
    PRESET_UNCHANGED,/* eliminateSpikes */
    PRESET_UNCHANGED,/* useSpars */
    PRESET_UNCHANGED,/* doLe */
    PRESET_UNCHANGED,/* doLD */
    PRESET_UNCHANGED,/* doLI */
    STRING_UNCHANGED,/* leRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* ldRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* liRegExp[MAX_REGEXP_SIZE] */
    PRESET_UNCHANGED,/* doWeighted */
    PRESET_UNCHANGED,/* doWLe */
    PRESET_UNCHANGED,/* doWLD */
    STRING_UNCHANGED,/* wleRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* wldRegExp[MAX_REGEXP_SIZE] */
  },
  /* Preset : Caustics only */
  {
    PRESET_DERIVE,/* maximumEyePathDepth */
    PRESET_DERIVE,/* maximumLightPathDepth */
    PRESET_UNCHANGED,/* maximumPathDepth */
    PRESET_UNCHANGED,/* minimumPathDepth */
    PRESET_UNCHANGED,/* samplesPerPixel */
    PRESET_UNCHANGED,/* totalSamples */
    FALSE,/* sampleImportantLights */
    PRESET_UNCHANGED,/* progressiveTracing */
    PRESET_UNCHANGED,/* eliminateSpikes */
    TRUE,/* useSpars */
    TRUE,/* doLe */
    FALSE,/* doLD */
    FALSE,/* doLI */
    "(LX)(G|S)+(D)(EX)",/* leRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* ldRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* liRegExp[MAX_REGEXP_SIZE] */
    FALSE,/* doWeighted */
    PRESET_UNCHANGED,/* doWLe */
    PRESET_UNCHANGED,/* doWLD */
    STRING_UNCHANGED,/* wleRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* wldRegExp[MAX_REGEXP_SIZE] */
  },
  /* Preset : Diffuse Only */
  {
    PRESET_DERIVE,/* maximumEyePathDepth */
    PRESET_DERIVE,/* maximumLightPathDepth */
    PRESET_UNCHANGED,/* maximumPathDepth */
    PRESET_UNCHANGED,/* minimumPathDepth */
    PRESET_UNCHANGED,/* samplesPerPixel */
    PRESET_UNCHANGED,/* totalSamples */
    TRUE,/* sampleImportantLights */
    PRESET_UNCHANGED,/* progressiveTracing */
    PRESET_UNCHANGED,/* eliminateSpikes */
    TRUE,/* useSpars */
    TRUE,/* doLe */
    FALSE,/* doLD */
    FALSE,/* doLI */
    "(LX)(D)*(EX)",/* leRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* ldRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* liRegExp[MAX_REGEXP_SIZE] */
    FALSE,/* doWeighted */
    PRESET_UNCHANGED,/* doWLe */
    PRESET_UNCHANGED,/* doWLD */
    STRING_UNCHANGED,/* wleRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* wldRegExp[MAX_REGEXP_SIZE] */
  },
  /* Preset : Classic Multipass */
  {
    PRESET_DERIVE,/* maximumEyePathDepth */
    1,/* maximumLightPathDepth */
    PRESET_UNCHANGED,/* maximumPathDepth */
    PRESET_UNCHANGED,/* minimumPathDepth */
    PRESET_UNCHANGED,/* samplesPerPixel */
    PRESET_UNCHANGED,/* totalSamples */
    TRUE,/* sampleImportantLights */
    PRESET_UNCHANGED,/* progressiveTracing */
    PRESET_UNCHANGED,/* eliminateSpikes */
    TRUE,/* useSpars */
    TRUE,/* doLe */
    TRUE,/* doLD */
    TRUE,/* doLI */
    "(LX)(G|S)*(EX)",/* leRegExp[MAX_REGEXP_SIZE] */
    "(LX)(G|S)*(EX)",/* ldRegExp[MAX_REGEXP_SIZE] */
    "(LX)(G|S)*(EX)",/* liRegExp[MAX_REGEXP_SIZE] */
    FALSE,/* doWeighted */
    PRESET_UNCHANGED,/* doWLe */
    PRESET_UNCHANGED,/* doWLD */
    STRING_UNCHANGED,/* wleRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* wldRegExp[MAX_REGEXP_SIZE] */
  },
  /* Preset : Enhanced Multipass PT */
  {
    PRESET_DERIVE,/* maximumEyePathDepth */
    1,/* maximumLightPathDepth */
    PRESET_UNCHANGED,/* maximumPathDepth */
    PRESET_UNCHANGED,/* minimumPathDepth */
    PRESET_UNCHANGED,/* samplesPerPixel */
    PRESET_UNCHANGED,/* totalSamples */
    TRUE,/* sampleImportantLights */
    PRESET_UNCHANGED,/* progressiveTracing */
    PRESET_UNCHANGED,/* eliminateSpikes */
    TRUE,/* useSpars */
    TRUE,/* doLe */
    TRUE,/* doLD */
    TRUE,/* doLI */
    "(LX)(G|S)(X)*(EX),(LX)(EX)",/* leRegExp[MAX_REGEXP_SIZE] */
    "(LX)(G|S)(X)*(EX),(LX)(EX)",/* ldRegExp[MAX_REGEXP_SIZE] */
    "(LX)(G|S)(X)*(EX),(LX)(EX)",/* liRegExp[MAX_REGEXP_SIZE] */
    FALSE,/* doWeighted */
    PRESET_UNCHANGED,/* doWLe */
    PRESET_UNCHANGED,/* doWLD */
    STRING_UNCHANGED,/* wleRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* wldRegExp[MAX_REGEXP_SIZE] */
  },
  /* Preset : BPT Multipass */
  {
    PRESET_DERIVE,/* maximumEyePathDepth */
    PRESET_DERIVE,/* maximumLightPathDepth */
    PRESET_UNCHANGED,/* maximumPathDepth */
    PRESET_UNCHANGED,/* minimumPathDepth */
    PRESET_UNCHANGED,/* samplesPerPixel */
    PRESET_UNCHANGED,/* totalSamples */
    TRUE,/* sampleImportantLights */
    PRESET_UNCHANGED,/* progressiveTracing */
    PRESET_UNCHANGED,/* eliminateSpikes */
    TRUE,/* useSpars */
    TRUE,/* doLe */
    TRUE,/* doLD */
    TRUE,/* doLI */
    "(LX)(G|S)(X)*(EX),(LX)(EX)",/* leRegExp[MAX_REGEXP_SIZE] */
    "(LX)(G|S)(X)*(EX),(LX)(EX)",/* ldRegExp[MAX_REGEXP_SIZE] */
    "(LX)(G|S)(X)*(EX),(LX)(EX)",/* liRegExp[MAX_REGEXP_SIZE] */
    FALSE,/* doWeighted */
    PRESET_UNCHANGED,/* doWLe */
    PRESET_UNCHANGED,/* doWLD */
    STRING_UNCHANGED,/* wleRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* wldRegExp[MAX_REGEXP_SIZE] */
  }
#ifdef WMP_WEIGHTS
  ,
  /* Preset : BPT Multipass with indirect caustic optimisation */
  {
    PRESET_DERIVE,/* maximumEyePathDepth */
    PRESET_DERIVE,/* maximumLightPathDepth */
    PRESET_UNCHANGED,/* maximumPathDepth */
    PRESET_UNCHANGED,/* minimumPathDepth */
    PRESET_UNCHANGED,/* samplesPerPixel */
    PRESET_UNCHANGED,/* totalSamples */
    TRUE,/* sampleImportantLights */
    PRESET_UNCHANGED,/* progressiveTracing */
    PRESET_UNCHANGED,/* eliminateSpikes */
    TRUE,/* useSpars */
    TRUE,/* doLe */
    TRUE,/* doLD */
    TRUE,/* doLI */
    "(LX)(G|S)(X)*(EX),(LX)(EX),(LX)(DR)(S)+(DR)(X)*(EX)",/* leRegExp[MAX_REGEXP_SIZE] */
    "(LX)(G|S)(X)*(EX),(LX)(EX),-(LX)(S)+(DR)(X)*(EX)",/* ldRegExp[MAX_REGEXP_SIZE] */
    "(LX)(G|S)(X)*(EX),(LX)(EX)",/* liRegExp[MAX_REGEXP_SIZE] */
    FALSE,/* doWeighted */
    PRESET_UNCHANGED,/* doWLe */
    PRESET_UNCHANGED,/* doWLD */
    STRING_UNCHANGED,/* wleRegExp[MAX_REGEXP_SIZE] */
    STRING_UNCHANGED,/* wldRegExp[MAX_REGEXP_SIZE] */
  }
#ifdef NEVER
  ,
  /* Preset : Weighted BPT Multipass */
  {
    PRESET_DERIVE,/* maximumEyePathDepth */
    PRESET_DERIVE,/* maximumLightPathDepth */
    PRESET_UNCHANGED,/* maximumPathDepth */
    PRESET_UNCHANGED,/* minimumPathDepth */
    PRESET_UNCHANGED,/* samplesPerPixel */
    PRESET_UNCHANGED,/* totalSamples */
    TRUE,/* sampleImportantLights */
    PRESET_UNCHANGED,/* progressiveTracing */
    PRESET_UNCHANGED,/* eliminateSpikes */
    TRUE,/* useSpars */
    TRUE,/* doLe */
    FALSE,/* doLD */
    TRUE,/* doLI */
    "(LX)(G|S)(X)*(EX),(LX)(EX)",/* leRegExp[MAX_REGEXP_SIZE] */
    "",/* ldRegExp[MAX_REGEXP_SIZE] */
    "(LX)(G|S)(X)*(EX),(LX)(EX)",/* liRegExp[MAX_REGEXP_SIZE] */
    TRUE,/* doWeighted */
    TRUE,/* doWLe */
    TRUE,/* doWLD */
    "(LX)(DR)(G|S)(X)*(EX),(LX)(DR)(EX)",/* wleRegExp[MAX_REGEXP_SIZE] */
    "(LX)(G|S)(X)*(EX),(LX)(EX)",/* wldRegExp[MAX_REGEXP_SIZE] */
  }
#endif
#endif /* WMP_WEIGHTS */
};

#define NUM_BIDIR_PRESETS (sizeof(bidirPresets)/sizeof(BP_BASECONFIG))
