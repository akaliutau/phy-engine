

#include "render.h"
#include "camera.h"
#include "tonemapping.h"
#include "error.h"
#include "pools.h"

static float Dither(int i, int j, float shade)
{
  static unsigned char dither[16][16] = {
    {  0,240, 80,160, 48,192, 96,144, 16,224, 64,176, 32,208,112,128},
    {136,120,216, 40,184, 72,232, 24,152,104,200, 56,168, 88,248,  8},
    { 68,180, 20,228,116,132, 36,212, 84,164,  4,244,100,148, 52,196},
    {204, 60,156,108,252, 12,172, 92,220, 44,140,124,236, 28,188, 76},
    { 34,210,114,130, 18,226, 66,178, 50,194, 98,146,  2,242, 82,162},
    {170, 90,250, 10,154,106,202, 58,186, 74,234, 26,138,122,218, 42},
    {102,150, 54,198, 86,166,  6,246,118,134, 38,214, 70,182, 22,230},
    {238, 30,190, 78,222, 46,142,126,254, 14,174, 94,206, 62,158,110},
    { 17,225, 65,177, 33,209,113,129,  1,241, 81,161, 49,193, 97,145},
    {153,105,201, 57,169, 89,249,  9,137,121,217, 41,185, 73,233, 25},
    { 85,165,  5,245,101,149, 53,197, 69,181, 21,229,117,133, 37,213},
    {221, 45,141,125,237, 29,189, 77,205, 61,157,109,253, 13,173, 93},
    { 51,195, 99,147,  3,243, 83,163, 35,211,115,131, 19,227, 67,179},
    {187, 75,235, 27,139,123,219, 43,171, 91,251, 11,155,107,203, 59},
    {119,135, 39,215, 71,183, 23,231,103,151, 55,199, 87,167,  7,247},
    {255, 15,175, 95,207, 63,159,111,239, 31,191, 79,223, 47,143,127}
  };
  return ((shade*256) > dither[i%16][j%16] ? 1. : 0.);
}

static void MakeTestImg1(RGB *img)
{
  int i, j;
  RGB *pix;
  j = 0; pix = img;
  while (j < Camera.vres/6) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, 0., 0., 0.25 + 0.5*(float)i/(float)(Camera.hres-1));
    j++ ;
  }
  while (j < 2*Camera.vres/6) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, 0., 0., Dither(i, j, 0.5));
    j++;
  }
  while (j < 3*Camera.vres/6) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, 0., 0.25 + 0.5 * (float)i/(float)(Camera.hres-1), 0.);
    j++ ;
  }
  while (j < 4*Camera.vres/6) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, 0., Dither(i, j, 0.5), 0.);
    j++;
  }
  while (j < 5*Camera.vres/6) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, 0.25 + 0.5 * (float)i/(float)(Camera.hres-1), 0., 0.);
    j++ ;
  }
  while (j < 6*Camera.vres/6) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, Dither(i, j, 0.5), 0., 0.);
    j++;
  }
  for (j=0; j<Camera.vres; j++) {
    RGB *row = img + j*Camera.hres;
    RGBSET(row[Camera.hres/2], 1., 1., 1.);
  }
}

static void MakeTestImg2(RGB *img)
{
  int i, j;
  RGB *pix;
  j = 0; pix = img;
  while (j < Camera.vres/12) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, 0., 0., (float)i/(float)(Camera.hres-1));
    j++ ;
  }
  while (j < 2*Camera.vres/12) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, 0., 0., Dither(i, j, (float)i/(float)(Camera.hres-1)));
    j++;
  }
  while (j < 3*Camera.vres/12) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, 0., 0., (float)i/(float)(Camera.hres-1));
    j++ ;
  }
  while (j < 4*Camera.vres/12) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, 0., 0., Dither(i, j, (float)i/(float)(Camera.hres-1)));
    j++;
  }
  while (j < 5*Camera.vres/12) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, 0., (float)i/(float)(Camera.hres-1), 0.);
    j++ ;
  }
  while (j < 6*Camera.vres/12) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, 0., Dither(i, j, (float)i/(float)(Camera.hres-1)), 0.);
    j++;
  }
  while (j < 7*Camera.vres/12) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, 0., (float)i/(float)(Camera.hres-1), 0.);
    j++ ;
  }
  while (j < 8*Camera.vres/12) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, 0., Dither(i, j, (float)i/(float)(Camera.hres-1)), 0.);
    j++;
  }
  while (j < 9*Camera.vres/12) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, (float)i/(float)(Camera.hres-1), 0., 0.);
    j++ ;
  }
  while (j < 10*Camera.vres/12) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, Dither(i, j, (float)i/(float)(Camera.hres-1)), 0., 0.);
    j++;
  }
  while (j < 11*Camera.vres/12) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, (float)i/(float)(Camera.hres-1), 0., 0.);
    j++ ;
  }
  while (j < 12*Camera.vres/12) {
    
    for (i=0; i<Camera.hres; i++, pix++)
      RGBSET(*pix, Dither(i, j, (float)i/(float)(Camera.hres-1)), 0., 0.);
    j++;
  }
  for (j=0; j<Camera.vres; j++) {
    RGB *row = img + j*Camera.hres;
    RGBSET(row[Camera.hres*1/10], 1., 1., 1.);
    RGBSET(row[Camera.hres*2/10], 1., 1., 1.);
    RGBSET(row[Camera.hres*3/10], 1., 1., 1.);
    RGBSET(row[Camera.hres*4/10], 1., 1., 1.);
    RGBSET(row[Camera.hres*5/10], 1., 1., 1.);
    RGBSET(row[Camera.hres*6/10], 1., 1., 1.);
    RGBSET(row[Camera.hres*7/10], 1., 1., 1.);
    RGBSET(row[Camera.hres*8/10], 1., 1., 1.);
    RGBSET(row[Camera.hres*9/10], 1., 1., 1.);
    RGBSET(row[Camera.hres*1/20], 0., 0., 0.);
    RGBSET(row[Camera.hres*3/20], 0., 0., 0.);
    RGBSET(row[Camera.hres*5/20], 0., 0., 0.);
    RGBSET(row[Camera.hres*7/20], 0., 0., 0.);
    RGBSET(row[Camera.hres*9/20], 0., 0., 0.);
    RGBSET(row[Camera.hres*11/20], 0., 0., 0.);
    RGBSET(row[Camera.hres*13/20], 0., 0., 0.);
    RGBSET(row[Camera.hres*15/20], 0., 0., 0.);
    RGBSET(row[Camera.hres*17/20], 0., 0., 0.);
    RGBSET(row[Camera.hres*19/20], 0., 0., 0.);
  }
}

static void DrawStripes(RGB *img, int hmin, int hmax, int vmin, int vmax, RGB col)
{
  int i, j;
  for (j=vmin; j<=vmax; j++) {
    RGB *row = img + j*Camera.hres;
    for (i=hmin; i<=hmax; i++) 
      row[i] = (j&1)!=0 ? col : Black;
  }
}

static void DrawRect(RGB *img, int hmin, int hmax, int vmin, int vmax, RGB col)
{
  int i, j;
  for (j=vmin; j<=vmax; j++) {
    RGB *row = img + j*Camera.hres;
    for (i=hmin; i<=hmax; i++) 
      row[i] = col;
  }
}

static void DrawChecker(RGB *img, int hmin, int hmax, int vmin, int vmax, int tilesize, RGB col)
{
  int i, j;
  for (j=vmin; j<=vmax; j++) {
    RGB *row = img + j*Camera.hres;
    for (i=hmin; i<=hmax; i++)
      row[i] = ((((i-hmin)/tilesize) + ((j-vmin)/tilesize))&1)!=0 ? col : Black;
  }
}

static void DrawVLine(RGB *img, int h, int vmin, int vmax, RGB col)
{
  int j;
  for (j=vmin; j<=vmax; j++) {
    RGB *row = img + j*Camera.hres;
    row[h] = col;
  }
}

static void DrawHLine(RGB *img, int hmin, int hmax, int v, RGB col)
{
  int i;
  RGB *row = img + v*Camera.hres;
  for (i=hmin; i<=hmax; i++)
    row[i] = col;
}

static void MakeTestImg3(RGB *img)
{
  RGB
    HalfRed = {0.5, 0., 0.},
    HalfGreen = {0., 0.5, 0.},
    HalfBlue = {0., 0., 0.5};
  DrawStripes(img, 0, Camera.hres/6-1, 0, Camera.vres-1, Red);
  DrawRect(img, Camera.hres/6, 2*Camera.hres/6-1, 0, Camera.vres-1, HalfRed);
  DrawStripes(img, 2*Camera.hres/6, 3*Camera.hres/6-1, 0, Camera.vres-1, Green);
  DrawRect(img, 3*Camera.hres/6, 4*Camera.hres/6-1, 0, Camera.vres-1, HalfGreen);
  DrawStripes(img, 4*Camera.hres/6, 5*Camera.hres/6-1, 0, Camera.vres-1, Blue);
  DrawRect(img, 5*Camera.hres/6, 6*Camera.hres/6-1, 0, Camera.vres-1, HalfBlue);
  DrawVLine(img, 2*Camera.hres/6, 0, Camera.vres-1, Black);
  DrawVLine(img, 4*Camera.hres/6, 0, Camera.vres-1, Black);
}

static void MakeTestImg4(RGB *img)
{
  RGB DarkGray = {0.04, 0.04, 0.04};
  DrawRect(img, Camera.hres/4, 3*Camera.hres/4-1, Camera.vres/4, 3*Camera.vres/4, DarkGray);
}


void RenderGammaTestImage(int which)
{
  RGB *img = (RGB *)Alloc(Camera.hres * Camera.vres * sizeof(RGB));  
  memset((void*)img, 0, sizeof(RGB)*Camera.hres*Camera.vres);
  switch (which) {
  case TESTIMG1: MakeTestImg1(img); break;
  case TESTIMG2: MakeTestImg2(img); break;
  case GAMMA_TEST_IMAGE: MakeTestImg3(img); break;
  case BLACK_LEVEL_IMAGE: MakeTestImg4(img); break;
  default:
    Fatal(-1, "RenderGammaTestImage", "Invalid image index %d", which);
  }
  RenderPixels(0, 0, Camera.hres, Camera.vres, img);
  Free((char *)img, Camera.hres * Camera.vres * sizeof(RGB));
}

