#ifndef _PHY_PIC_H_
#define _PHY_PIC_H_

#include "image.h"

/**
 *	(Simple) High dynamic range PIC output handle.
 *
 *	Olaf Appeltants, March 2000
 */
class PicOutputHandle : public ImageOutputHandle
{
protected:
  FILE *pic;
  void writeHeader ();

public:
  PicOutputHandle (char *filename, int w, int h);
  ~PicOutputHandle ();

  int WriteRadianceRGB (float *rgbrad);
};

#endif /* _PHY_PIC_H_ */
