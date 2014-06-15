#ifndef _PHY_LUV_H_
#define _PHY_LUV_H_

/* CIE L*u*v* colour triplet. */
typedef struct LUV {
  float L, u, v;
} LUV;

/* Macro to print a LUV triplet; it's a macro, not a function,
 * therefore I am using capitals */
#define LUV_PRINT(_f, _c) \
	fprintf(_f, "Luv(%8g,%8g,%8g)", (_c).L, (_c).u, (_c).v)

/* CIE color difference formula: Eucledian distance of two LUV colors */
#define LUV_DIFF(_c1, _c2) \
	sqrt(((_c1).L-(_c2).L)*((_c1).L-(_c2).L) + \
	     ((_c1).u-(_c2).u)*((_c1).u-(_c2).u) + \
	     ((_c1).v-(_c2).v)*((_c1).v-(_c2).v))

/* Visibility threshold ... some people claim it's 2% on the luminance
 * scale, therefore it shall be 2 for LUV colour where L goes from 0
 * to 100. I am not sure that this is the correct approach though. */
#define LUV_VISIBILITY_THRESHOLD 2.0

#endif /*_PHY_LUV_H_*/
