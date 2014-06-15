/* cie.h: converts MGF color specification into our representation of
 * colors. */

#ifndef _CIE_H_
#define _CIE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* for XYZ<->LUV conversions */

#ifdef USE_D50
#  define CIE_WHITE_X           100.0
#  define CIE_WHITE_Y           100.0
#  define CIE_WHITE_Z           100.0
#else
#  define CIE_WHITE_X           100.0
#  define CIE_WHITE_Y           100.0
#  define CIE_WHITE_Z           100.0
#endif

#define CIE_DISPLAY_WHITE_Y	1.0

/* ---------------------------------------------------------------------------
  `MAX_EFFICACY'
  `WHITE_EFFICACY'

  Luminous efficacies [lm/W] for conversion between radiometric and
  photometric units (in our case, for conversion between radiance and
  luminance). Normaly, the spectral value of a radiometric quantity would
  be scaled by the photopic luminous efficeincy function and integrated
  over the visible spectrum; the result multiplied by "MAX_EFFICACY would
  give the appropriate photometric value. Without knowing the spectral
  represnetation, it is generally impossible to perform correct
  radometric->photometric conversion. The "WHITE_EFFICACY" factor is the
  ratio between the luminour powes of the uniform equal-energy white
  spectrum of 1W and its radiant power (which is, surprisingly, 1W). The
  corrected CIE 1988 standard observer curve has been used in this case -
  using the older CIE 1931 curves gives the value of 179 (see the Radiance
  rendering system). 
  ------------------------------------------------------------------------- */
#define MAX_EFFICACY		683.002  /* photopic maximum for 555 nm */
#define WHITE_EFFICACY		183.07   /* uniform white light */

extern void xyz_rgb(float *xyz, float *rgb);
extern void rgb_xyz(float *rgb, float *xyz);

extern void xyz_luv(float *xyz, float *luv, float ymax);
extern void luv_xyz(float *luv, float *xyz, float ymax);

extern void xyz_xyy(float *xyz, float *xyy);
extern void xyy_xyz(float *xyy, float *xyz);

/* returns TRUE if the color was desaturated during clipping against the 
 * monitor gamut */
extern int clipgamut(float *rgb);

/* computes RGB <-> XYZ color transforms based on the 
 * given monitor primary colors and whitepoint */
extern void ComputeColorConversionTransforms(float xr, float yr,
					     float xg, float yg,
					     float xb, float yb,
					     float xw, float yw);

/* ---------------------------------------------------------------------------
  `GetLuminousEfficacy'
  `SetLuminousEfficacy'

  Set/return the value usef for tristimulus white efficacy.
  ------------------------------------------------------------------------- */
extern void GetLuminousEfficacy(float *e);
extern void SetLuminousEfficacy(float e);


#ifdef __cplusplus
}
#endif

#endif /*_CIE_H_*/
