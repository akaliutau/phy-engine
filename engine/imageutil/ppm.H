/* ppm.H: image output in PPM format */

#ifndef _PPM_H_
#define _PPM_H_

#include "image.h"

class PPMOutputHandle: public ImageOutputHandle {
protected:
  FILE *fp;		// image file pointer

public:
  PPMOutputHandle(FILE *_fp, int _width, int _height);
  int WriteDisplayRGB(unsigned char *rgb);
};

#endif /*_PPM_H_*/
