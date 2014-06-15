

#include <config.h>
#include <string.h>
//#include "mystrings.h"

#include "image.h"
#include "color.h"
#include "error.h"



#define NEW_TIFF_RGB_HANDLE(_fn, _w, _h) \
	new LZWRGBTiffOutputHandle(_fn, _w, _h)

#ifdef HAVE_LIBTIFF



#ifdef HAVE_TIFFINITSGILOG



#define IS_TIFF_EXT(_ext) \
	IS_TIFF_RGB_EXT(_ext) || IS_TIFF_LOGLUV_EXT(_ext)
#define PRE_TIFF_LOGLUV_HANDLE(_func) 
#define NEW_TIFF_LOGLUV_HANDLE(_fn, _w, _h, _lum) \
	new SGILogLuvTiffOutputHandle(_fn, _w, _h, _lum)
#define NEW_TIFF_LOGLUV_HANDLE_HDR(_fn, _w, _h, _hdr, _lum) \
	_hdr ? \
	(ImageOutputHandle *) NEW_TIFF_LOGLUV_HANDLE(_fn, _w, _h, _lum) :



#else



#define IS_TIFF_EXT(_ext) \
	IS_TIFF_RGB_EXT(_ext)
#define PRE_TIFF_LOGLUV_HANDLE(_func) \
	Error(_func, \
	      "Your libtiff does not support high dynamic range images.")
#define NEW_TIFF_LOGLUV_HANDLE(_fn, _w, _h, _lum) \
	(ImageOutputHandle *)NULL
#define NEW_TIFF_LOGLUV_HANDLE_HDR(_fn, _w, _h, _hdr, _lum)



#endif



#define PRE_TIFF_GENERAL_HANDLE(_func)
#define NEW_TIFF_GENERAL_HANDLE(_fn, _w, _h, _hdr, _lum) \
	NEW_TIFF_LOGLUV_HANDLE_HDR(_fn, _w, _h, _hdr, _lum) \
	NEW_TIFF_RGB_HANDLE(_fn, _w, _h)



#else


 
#define PRE_TIFF_GENERAL_HANDLE(_func) \
	Error(_func, "TIFF support has not been compiled in")
#define NEW_TIFF_GENERAL_HANDLE(_fn, _w, _h, _hdr, _lum) \
	(ImageOutputHandle *)NULL



#endif


int ImageOutputHandle::WriteDisplayRGB(unsigned char *)
{
  static bool wgiv = false;
  if (!wgiv) {
    fprintf(stderr, "%s does not support display RGB output.\n", drivername);
    wgiv = true;
  }
  return 0;
}

#define GAMMACORRECT(rgb, gamma) { \
  (rgb).r = (gamma)[0] == 1. ? (rgb).r : pow((rgb).r, 1./(gamma)[0]); \
  (rgb).g = (gamma)[1] == 1. ? (rgb).g : pow((rgb).g, 1./(gamma)[1]); \
  (rgb).b = (gamma)[2] == 1. ? (rgb).b : pow((rgb).b, 1./(gamma)[2]); \
}

int ImageOutputHandle::WriteDisplayRGB(float *rgbflt)
{
#ifdef RGBCOLORS
  unsigned char *rgb = new unsigned char[3*width];
  for (int i=0; i<width; i++) {
    // convert RGB radiance to display RGB
    RGB disprgb = *(RGB *)(&rgbflt[3*i]);
    // RadianceToRGB(*(COLOR *)&rgbflt[3*i], &disprgb);	
    // apply gamma correction    
    GAMMACORRECT(disprgb, gamma);
    // convert float to byte representation
    rgb[3*i  ] = (unsigned char)(disprgb.r * 255.);
    rgb[3*i+1] = (unsigned char)(disprgb.g * 255.);
    rgb[3*i+2] = (unsigned char)(disprgb.b * 255.);
  }

  // output display RGB values
  int pixwrit = WriteDisplayRGB(rgb);

  delete[] rgb;
  return pixwrit;
#else
  static bool wgiv = false;
  if (!wgiv) {
    fprintf(stderr, "%s does not support RGB radiance output.\n", drivername);
    wgiv = true;
  }
  return 0;
#endif
}

int ImageOutputHandle::WriteRadianceRGB(float *rgbrad)
{
#ifdef RGBCOLORS
  unsigned char *rgb = new unsigned char[3*width];
  for (int i=0; i<width; i++) {
    // convert RGB radiance to display RGB
    RGB disprgb;
    RadianceToRGB(*(COLOR *)&rgbrad[3*i], &disprgb);	
    // apply gamma correction
    GAMMACORRECT(disprgb, gamma);
    // convert float to byte representation
    rgb[3*i  ] = (unsigned char)(disprgb.r * 255.);
    rgb[3*i+1] = (unsigned char)(disprgb.g * 255.);
    rgb[3*i+2] = (unsigned char)(disprgb.b * 255.);
  }

  // output display RGB values
  int pixwrit = WriteDisplayRGB(rgb);

  delete[] rgb;
  return pixwrit;
#else
  static bool wgiv = false;
  if (!wgiv) {
    fprintf(stderr, "%s does not support RGB radiance output.\n", drivername);
    wgiv = true;
  }
  return 0;
#endif
}

int ImageOutputHandle::WriteRadianceXYZ(float *xyzrad)
{
#ifndef RGBCOLORS
  unsigned char *rgb = new unsigned char[3*width];
  for (int i=0; i<width; i++) {
    // convert RGB radiance to display RGB
    RGB disprgb;
    RadianceToRGB(*(COLOR *)&xyzrad[3*i], &disprgb);	
    // apply gamma correction
    GAMMACORRECT(disprgb, gamma);
    // convert float to byte representation
    rgb[3*i  ] = (unsigned char)(disprgb.r * 255.);
    rgb[3*i+1] = (unsigned char)(disprgb.g * 255.);
    rgb[3*i+2] = (unsigned char)(disprgb.b * 255.);
  }

  // output display RGB values
  int pixwrit = WriteDisplayRGB(rgb);

  delete[] rgb;
  return pixwrit;
#else
  static bool wgiv = false;
  if (!wgiv) {
    Error("WriteRadianceXYZ",
	  "%s does not support CIE XYZ radiance output.\n", drivername);
    wgiv = true;
  }
  return 0;
#endif
}



#include "image.h"
#include "ppm.H"
#include "tiff.H"
#include "pic.H"


char *
ImageFileExtension(char *fname)
{
  char *ext = fname + strlen(fname)-1;	

  while (ext>=fname && *ext!='.')
    ext--;

  if ((!strcmp(ext, ".Z"))   || 
      (!strcmp(ext, ".gz"))  ||
      (!strcmp(ext, ".bz"))  ||
      (!strcmp(ext, ".bz2")))
  { 
    ext--;				
    while (ext>=fname && *ext!='.')
      ext--;				
  }

  return ext+1;				
}


ImageOutputHandle *
CreateRadianceImageOutputHandle(char *fname, FILE *fp,
				int ispipe,
				int width, int height,
				float reference_luminance)
{
  if (fp)
  {
    char *ext = ispipe ? (char *)"ppm" : ImageFileExtension(fname);
    					// assume PPM format if pipe

    if (strncasecmp(ext, "ppm", 3) == 0) {
      return new PPMOutputHandle(fp, width, height);
    }
#ifdef HAVE_LIBTIFF
    else if (IS_TIFF_EXT(ext))
    {
      if (ispipe)
      {
	Error("CreateRadianceImageOutputHandle",
	      "Can't write TIFF output to a pipe.\n");
	return (ImageOutputHandle *)0;
      }
      freopen("/dev/null", "w", fp);	
      if (IS_TIFF_LOGLUV_EXT(ext))
      {
	PRE_TIFF_LOGLUV_HANDLE("CreateRadianceImageOutputHandle");
	return
	  NEW_TIFF_LOGLUV_HANDLE(fname, width, height, reference_luminance);
      }
      else
      {
	return
	  NEW_TIFF_RGB_HANDLE(fname, width, height);
      }
    }
#endif
    
    else if(strncasecmp(ext, "pic", 3) == 0)
    {
	if (ispipe)
	{
	    Error("CreateRadianceImageOutputHandle",
	          "Can't write PIC output to a pipe.\n");
	    return (ImageOutputHandle *)0;
	}

	freopen("/dev/null", "w", fp);

	return new PicOutputHandle(fname, width, height);
    }
    else
    {
      PRE_TIFF_GENERAL_HANDLE("CreateRadianceImageOutputHandle");
      Error("CreateRadianceImageOutputHandle",
	    "Can't save high dynamic range images to a '%s' file.", ext);
      return (ImageOutputHandle *)0;
    }
  }
  return (ImageOutputHandle *)0;
}


ImageOutputHandle *
CreateImageOutputHandle(char *fname, FILE *fp,
			int ispipe, int width, int height)
{
  if (fp) {
    char *ext = ispipe ? (char *)"ppm" : ImageFileExtension(fname);

    if (strncasecmp(ext, "ppm", 3) == 0)
    {
      return new PPMOutputHandle(fp, width, height);
    }
#ifdef HAVE_LIBTIFF
    else if (IS_TIFF_RGB_EXT(ext))
    {
      if (ispipe)
      {
	Error("CreateImageOutputHandle",
	      "Can't write TIFF output to a pipe.\n");
	return (ImageOutputHandle *)0;
      }
      freopen("/dev/null", "w", fp);	

      return new LZWRGBTiffOutputHandle(fname, width, height);
    }
#endif
    else
    {
      PRE_TIFF_GENERAL_HANDLE("CreateImageOutputHandle");
      Error("CreateImageOutputHandle",
	    "Can't save display-RGB images to a '%s' file.\n", ext);
      return (ImageOutputHandle *)0;
    }
  }
  return (ImageOutputHandle *)0;
}

ImageOutputHandle *
CreatePPMOutputHandle(FILE *fp, int width, int height)
{
  return (ImageOutputHandle *)new PPMOutputHandle(fp, width, height);
}

ImageOutputHandle *
CreateTiffOutputHandle(char *filename, int width, int height, 
		       int high_dynamic_range, float stonits)
{
  PRE_TIFF_GENERAL_HANDLE("CreateTiffOutputHandle");
  return
    NEW_TIFF_GENERAL_HANDLE(filename, width, height, 
			    high_dynamic_range, stonits <= EPSILON ? 1. : stonits);
}

int WriteDisplayRGB(ImageOutputHandle *img, unsigned char *data)
{
  return img->WriteDisplayRGB(data);
}

int WriteRadianceRGB(ImageOutputHandle *img, float *data)
{
  return img->WriteRadianceRGB(data);
}

int WriteRadianceXYZ(ImageOutputHandle *img, float *data)
{
  return img->WriteRadianceXYZ(data);
}

void DeleteImageOutputHandle(ImageOutputHandle *img)
{
  delete img;
}
