/* color.h */

#ifndef _PHY_COLOR_H_
#define _PHY_COLOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "colors/spectrum.h"

/* a struct because you can't assign arrays nor return them 
 * in functions */
typedef struct COLOR {
  SPECTRUM spec;
} COLOR;

#define ColorPrint(fp, c) 		PrintSpectrum(fp, (c).spec)
#define COLORCLEAR(c)			ClearSpectrum((c).spec)
#define COLORSET(c, v1, v2, v3)		SetSpectrum((c).spec, v1, v2, v3)
#define COLORSETMONOCHROME(c, v) 	SetSpectrumMonochrome((c).spec, v)
#define COLORNULL(c)			IsBlackSpectrum((c).spec)
#define COLORSCALE(a, s, r)		ScaleSpectrum((a), (s).spec, (r).spec)
#define COLORPROD(s, t, r)		MultSpectrum((s).spec, (t).spec, (r).spec)
#define COLORPRODSCALED(s, a, t, r)  	MultScaledSpectrum((s).spec, (a), (t).spec, (r).spec)
#define COLORSCALARPRODUCT(s, t)	ScalarSpectrumProduct((s).spec, (t).spec)
#define COLORADD(s, t, r)		AddSpectrum((s).spec, (t).spec, (r).spec)
#define COLORADDSCALED(s, a, t, r)	AddScaledSpectrum((s).spec, (a), (t).spec, (r).spec)
#define COLORADDCONSTANT(s, a, r)	AddConstantSpectrum((s).spec, (a), (r).spec)
#define COLORSUBTRACT(s, t, r)		SubtractSpectrum((s).spec, (t).spec, (r).spec)
#define COLORSUBTRACTSCALED(s, a, t, r)	SubtractScaledSpectrum((s).spec, (a), (t).spec, (r).spec)
#define COLORDIV(s, t, r)		DivideSpectrum((s).spec, (t).spec, (r).spec)
#define COLORSCALEINVERSE(a, s, r)	InverseScaleSpectrum((a), (s).spec, (r).spec)
#define COLORMAXCOMPONENT(s)		MaxSpectrumComponent((s).spec)
#define COLORMINCOMPONENT(s)		MinSpectrumComponent((s).spec)
#define COLORSUMABSCOMPONENTS(s)	SumAbsSpectrumComponents((s).spec)
#define COLORABS(s, r)			AbsSpectrum((s).spec, (r).spec)
#define COLORMAX(s, t, r)		MaxSpectrum((s).spec, (t).spec, (r).spec)
#define COLORMIN(s, t, r)		MinSpectrum((s).spec, (t).spec, (r).spec)
#define COLORCLIPPOSITIVE(s,r) 		ClipSpectrumPositive((s).spec, (r).spec)
#define COLORAVERAGE(s)			SpectrumAverage((s).spec)

#define ColorGray(s)			SpectrumGray((s).spec)
#define ColorLuminance(s)		SpectrumLuminance((s).spec)

#define COLORGAMMACORRECT(_c, _g)	SpectrumGammaCorrect((_c).spec, _g)

#define COLORINTERPOLATEBARYCENTRIC(c0, c1, c2, u, v, c) \
  SpectrumInterpolateBarycentric(c0.spec, c1.spec, c2.spec, u, v, (c).spec)
#define COLORINTERPOLATEBILINEAR(c0, c1, c2, c3, u, v, c) \
  SpectrumInterpolateBilinear(c0.spec, c1.spec, c2.spec, c3.spec, u, v, (c).spec)

#include "colors/rgb.h"
#include "colors/luv.h"

/* Converts from color representation used for radiance etc...
 * to display RGB colors */
extern RGB *ColorToRGB(COLOR col, RGB *rgb);
extern COLOR *RGBToColor(RGB rgb, COLOR *col);

/* Perceptually uniform color space CIE L*u*v* */
extern LUV *ColorToLUV(COLOR col, LUV *luv);
extern LUV *ColorToDisplayLUV(COLOR col, LUV *luv);

/* Does most to convert radiance to display RGB color
 * 1) radiance compression: from the high dynamic range in reality to
 *    the limited range of the computer screen.
 * 2) colormodel conversion from the color model used for the computations to
 *    an RGB triplet for display on the screen
 * 3) clipping of RGB values to the range [0,1].
 * Gamma correction is performed in render.c. */
extern RGB *RadianceToRGB(COLOR radiance, RGB *rgb);

#ifdef __cplusplus
}
#endif

#endif /*_PHY_COLOR_H_*/



