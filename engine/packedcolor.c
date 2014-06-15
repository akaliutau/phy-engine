



#include <math.h>

#include "Float.h"
#include "packedcolor.h"

PACKEDCOLOR PackColor(COLOR col)
{
  PACKEDCOLOR pcol;
  int e;
  float v;
#if SPECTRUM_CHANNELS != 3
  Error("packedcolor.c", "not ready for more than 3 spectrum channels");
#endif

  v = COLORMAXCOMPONENT(col);

  if(v < EPSILON)
  {
    pcol.r = 0;
    pcol.g = 0;
    pcol.b = 0;
    pcol.exp = 0;
  }
  else
  {
    v = frexp(v,&e) * 256. / v;
    pcol.r = (unsigned char)(int)(col.spec[0] * v);
    pcol.g = (unsigned char)(int)(col.spec[1] * v);
    pcol.b = (unsigned char)(int)(col.spec[2] * v);

    pcol.exp = (unsigned char)(e + 128);
  }

  return pcol;
}

COLOR UnpackColor(PACKEDCOLOR pcol)
{
  COLOR col;
  float v;

  if(pcol.exp == 0)
  {
    col.spec[0] = 0.0;
    col.spec[1] = 0.0;
    col.spec[2] = 0.0;
  }
  else
  {
    v = ldexp(1./256., (int)pcol.exp - 128);
    col.spec[0] = (pcol.r + .5) * v;
    col.spec[1] = (pcol.g + .5) * v;
    col.spec[2] = (pcol.b + .5) * v;
  }

  return col;
}
