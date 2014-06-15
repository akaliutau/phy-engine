
#include <stdio.h>
#include <math.h>
#include "vector.h"
#include "color.h"
#include "cie.h"
#include "extmath.h"
#include "tonemapping.h"
#include "render.h"

RGB 	NullRGB   = {0.0, 0.0, 0.0},
        Black     = {0.0, 0.0, 0.0},
        Red       = {1.0, 0.0, 0.0},
        Yellow    = {1.0, 1.0, 0.0},
        Green     = {0.0, 1.0, 0.0},
        Turquoise = {0.0, 1.0, 1.0},
        Blue      = {0.0, 0.0, 1.0},
        Magenta   = {1.0, 0.0, 1.0},
        White     = {1.0, 1.0, 1.0};


COLOR *RescaleRadiance(COLOR in, COLOR *out)
{
  COLORSCALE(tmopts.pow_bright_adjust, in, in);
  *out = TonemapScaleForDisplay(in);
  return out;
}

RGB *ColorToRGB(COLOR col, RGB *rgb)
{
#ifdef RGBCOLORS
  RGBSET(*rgb, col.spec[0], col.spec[1], col.spec[2]);
#else
  xyz_rgb(col.spec, (float *)rgb);
#endif
  return rgb;
}

COLOR *RGBToColor(RGB rgb, COLOR *col)
{
#ifdef RGBCOLORS
  COLORSET(*col, rgb.r, rgb.g, rgb.b);
#else
  rgb_xyz((float *)&rgb, col->spec);
#endif
  return col;
}

LUV *ColorToLUV(COLOR col, LUV *luv)
{
#ifdef RGBCOLORS
  {
    float xyz[3];
    rgb_xyz(col.spec, xyz);
    xyz_luv(xyz, (float *)luv, CIE_WHITE_Y);	
  }
#else
  xyz_luv(col.spec, (float *)luv, CIE_WHITE_Y);
#endif
  return luv;
}

COLOR *LUVToColor(LUV luv, COLOR *col)
{
#ifdef RGBCOLORS
  {
    float xyz[3];
    luv_xyz((float *)&luv, xyz, CIE_WHITE_Y);	
    xyz_rgb(xyz, col->spec);
  }
#else
  luv_xyz((float *)&luv, col->spec, CIE_WHITE_Y);
#endif
  return col;
}

LUV *ColorToDisplayLUV(COLOR col, LUV *luv)
{

  RGB rgb;
  float xyz[3];

  RadianceToRGB(col, &rgb);
  
  rgb_xyz((float*)&rgb, xyz);
  xyz_luv(xyz, (float*)luv, CIE_DISPLAY_WHITE_Y);

  return luv;
}




static void RGBClipProp(RGB *rgb)
{
  float s = RGBMAXCOMPONENT(*rgb);
  if (s > 1.) {
    rgb->r /= s;
    rgb->g /= s;
    rgb->b /= s;
  }
}

RGB *RadianceToRGB(COLOR color, RGB *rgb)
{
  RescaleRadiance(color, &color);
  ColorToRGB(color, rgb);
  RGBCLIP(*rgb);
  
  return rgb;
}

