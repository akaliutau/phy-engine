#ifndef _PHY_TIFF_H_
#define _PHY_TIFF_H_

#ifdef HAVE_LIBTIFF

#include "image.h"
#include "tiffio.h"

// TIFF output handle base class
class TiffOutputHandle : public ImageOutputHandle {
protected:
  TIFF *tif;		// TIFF file handle
  unsigned char *buf;	// scan line data
  int row;		// row number

  void init(char *filename, int w, int h, char *drivername);
  void allocbuf(void);

public:
  TiffOutputHandle();
  ~TiffOutputHandle();
};

// LZW-compressed RGB TIFF output handle
class LZWRGBTiffOutputHandle: public TiffOutputHandle {
public:
  LZWRGBTiffOutputHandle(char *filename, int w, int h);
  int WriteDisplayRGB(unsigned char *rgb);
};

#ifdef HAVE_TIFFINITSGILOG

// High dynamic range (compressed LogLuv) TIFF output handle
class SGILogLuvTiffOutputHandle: public TiffOutputHandle {
protected:
  double stonits;	// sample to Nits conversion value

public:
  SGILogLuvTiffOutputHandle(char *filename, int w, int h, float _stonits = 1.);
  int WriteRadianceRGB(float *rgbrad);
  int WriteRadianceXYZ(float *xyzrad);
};

#endif /* HAVE_TIFFINITSGILOG */
#else /* HAVE_LIBTIFF not defined */
/* this file is empty when libtiff is not available */
#endif /* HAVE_LIBTIFF */

#endif /* _PHY_TIFF_H_ */
