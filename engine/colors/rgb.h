/* rgb.h: representation of RGB color triples that can be 
 * displayed on the screen */

#ifndef _PHY_RGB_H_
#define _PHY_RGB_H_

typedef struct RGB {
	float r, g, b;
} RGB;

/* some RGB colors for e.g. debugging purposes */
extern RGB Black, White, Green, Yellow, Red, Magenta, Blue, Turquoise;

#ifdef __cplusplus
extern "C++" {
inline int operator==(RGB rgb1, RGB rgb2)
  {
    return(FLOATEQUAL(rgb1.r, rgb2.r, EPSILON) &&
	   FLOATEQUAL(rgb1.g, rgb2.g, EPSILON) &&
	   FLOATEQUAL(rgb1.b, rgb2.b, EPSILON));
  }
}
#endif

/* macro to print an RGB triplet */
#define RGBPrint(fp, color) 	fprintf(fp, "%g %g %g", (color).r, (color).g, (color).b);

/* macro to fill in an RGB structure with given red,green and blue components */
#define RGBSET(color, R, G, B)  ((color).r = (R), (color).g = (G), (color).b = (B))

/* clips RGB intensities to the interval [0,1] */
#define RGBCLIP(color)		(\
			(color).r = ((color).r < 0. ? 0. : ((color).r > 1. ? 1. : (color).r)), \
			(color).g = ((color).g < 0. ? 0. : ((color).g > 1. ? 1. : (color).g)), \
			(color).b = ((color).b < 0. ? 0. : ((color).b > 1. ? 1. : (color).b)))

#ifdef NEVER
/* depreciated - now uses table lookup + moved to tonemapping.h */
/* gamma correction */
#define RGBGAMMACORRECT(color, gamma)  (\
			(color).r = (gamma==1. ? (color).r : pow((color).r, 1./gamma)), \
			(color).g = (gamma==1. ? (color).g : pow((color).g, 1./gamma)), \
			(color).b = (gamma==1. ? (color).b : pow((color).b, 1./gamma)))
#endif

/* maximum component */
#define RGBMAXCOMPONENT(_r) \
	(_r).r > (_r).g ? MAX((_r).r,(_r).b) : MAX((_r).g,(_r).b)

#endif /*_PHY_RGB_H_*/

