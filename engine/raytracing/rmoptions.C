

#include "options.h"
#include "rmoptions.H"
#include "Boolean.h"

void RM_Defaults(void)
{
  rms.filter = RM_TENT_FILTER;
  rms.samplesPerPixel = 8;
}




static ENUMDESC rmPixelFilters[] = {
  { RM_BOX_FILTER, "box", 2 },
  { RM_TENT_FILTER, "tent", 2 },
  { RM_GAUSS_FILTER, "gaussian 1/sqrt2", 2 },
  { RM_GAUSS2_FILTER, "gaussian 1/2", 2 },
  { 0, NULL, 0 }
};
MakeEnumOptTypeStruct(rmPixelFilterTypeStruct, rmPixelFilters);
#define TrmPixelFilter (&rmPixelFilterTypeStruct)

static CMDLINEOPTDESC rmOptions[] = 
{
  {"-rm-samples-per-pixel", 6,	Tint,	&rms.samplesPerPixel,	DEFAULT_ACTION,
   "-rm-samples-per-pixel <number>\t: eye-rays per pixel"},
  {"-rm-pixel-filter", 7,	TrmPixelFilter, &rms.filter, 	DEFAULT_ACTION,
   "-rm-pixel-filter <type>\t: Select filter - \"box\", \"tent\", \"gaussian 1/sqrt2\", \"gaussian 1/2\""},
  {NULL	, 		0,	TYPELESS, NULL, 		DEFAULT_ACTION,
   NULL}
};

void RM_ParseOptions(int *argc, char **argv)
{
  ParseOptions(rmOptions, argc, argv);
}

void RM_PrintOptions(FILE *fp)
{
  fprintf(fp, "\nRaymatting options:\n");
  PrintOptions(fp, rmOptions);
}
