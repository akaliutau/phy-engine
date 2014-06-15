

#include <config.h>
#include <string.h>

#ifdef HAVE_LIBTIFF

#include "tiff.H"

// initializes common TIFF parameters
TiffOutputHandle::TiffOutputHandle()
{
  tif = 0;
  buf = 0;
  row = 0;
}

// common TIFF initialization stuff before specifying sample encoding
// and format
void TiffOutputHandle::init(char *filename, int w, int h, char *drivername)
{
  ImageOutputHandle::init(drivername, w, h);

  tif = TIFFOpen(filename, "w");
  if (!tif) {
    fprintf(stderr, "Can't open TIFF output");
    return;
  }

  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, (uint32)width);
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, (uint32)height);
  TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
}

// common TIFF initialization stuff after specifying sample encoding
// and format
void TiffOutputHandle::allocbuf(void)
{
  buf = (unsigned char *)_TIFFmalloc(TIFFScanlineSize(tif));
  TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP,
	       TIFFDefaultStripSize(tif, (uint32) -1));
}

// LZW-compressed RGB specific initializations
LZWRGBTiffOutputHandle::LZWRGBTiffOutputHandle(char *filename, int w, int h)
{
  init(filename, w, h, "LZW-compressed RGB TIFF");
  if (!tif) return;

#ifdef COMPRESSION_ADOBE_DEFLATE
  
  TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_ADOBE_DEFLATE);
#else
  TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
#endif
  TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, (uint16)3);
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, (uint16)8);

  allocbuf();
}

// common TIFF output termination
TiffOutputHandle::~TiffOutputHandle(void)
{
  if (tif) TIFFClose(tif);
  if (buf) _TIFFfree(buf);
}

// writes scanline of RGB display pixel values
int LZWRGBTiffOutputHandle::WriteDisplayRGB(unsigned char *rgb)
{
  if (!tif || !buf)
    return 0;

  memcpy(buf, rgb, 3*width);
  if (TIFFWriteScanline(tif, buf, row, 0) < 0) {
    fprintf(stderr, "Error writing TIFF output");
    return 0;
  }
  row++;

  return width;
}

#ifdef HAVE_TIFFINITSGILOG
#include "../cie.h"

// SGILOG LogLuv specific initializations
SGILogLuvTiffOutputHandle::SGILogLuvTiffOutputHandle(char *filename, int w, int h, float _stonits)
{
  stonits = _stonits;
  init(filename, w, h, "high dynamic range TIFF");
  if (!tif) return;

  TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_SGILOG);
  TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_LOGLUV);
  TIFFSetField(tif, TIFFTAG_SGILOGDATAFMT, SGILOGDATAFMT_FLOAT);
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, (uint16)3);
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, (uint16)32);
  TIFFSetField(tif, TIFFTAG_STONITS, stonits);

  //  allocbuf(); 		// Why shouldn't we use the default allocbuf() ???
  int i1 = 3*width;
  int i2 = 8192/i1;				// compute good strip size
  if (i2 < 1) i2 = 1;
  TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, (uint32)i2);

  buf = (unsigned char *)_TIFFmalloc((long)(3*width*sizeof(float)));
}

// writes scanline of high-dynamic range radiance data in CIE XZY format
int SGILogLuvTiffOutputHandle::WriteRadianceXYZ(float *xyzrad)
{
  if (!tif || !buf) 
    return 0;

  if (stonits <= 1e-6) stonits = 1.;

  // copy values to buf and divide by stonits
  float *fbuf = (float *)buf;
  for (int i=0; i<3*width; i++, fbuf++, xyzrad++) {
    *fbuf = *xyzrad / stonits;
  }

  if (TIFFWriteScanline(tif, buf, row, 0) < 0) {
    fprintf(stderr, "Error writing TIFF output");
    return 0;
  }
  row++;

  return width;
}

// writes scanline of high-dynamic range radiance data in RGB format
int SGILogLuvTiffOutputHandle::WriteRadianceRGB(float *rgbrad)
{
  // convert RGB to CIE XYZ ...
  float *xyzrad = new float[3*width];
  for (int i=0; i<width; i++) {
    rgb_xyz(&rgbrad[3*i], &xyzrad[3*i]);
  }
  // ... and write XYZ radiance values
  int pixwrit = WriteRadianceXYZ(xyzrad);
  delete[] xyzrad;
  return pixwrit;
}

#endif 
#else 

#endif 

