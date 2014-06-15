/*
 * screenbuffer.H
 */

#ifndef _SCREENBUFFER_H_
#define _SCREENBUFFER_H_

#ifdef __cplusplus

#include <stdio.h>

#include "image.h"
#include "color.h"
#include "camera.h"

/*************************************************************************/
// Class for storing pixel radiances/fluxes
// and an associated RGB framebuffer

// 12/10/99 : The screen buffer has now a Camera variable that states
//            from where the image is made. Several functions
//            are provided to handle pixel float coordinates,
//            pixel numbers and screen boundaries. These are
//            derived from the camera variable

class CScreenBuffer
{
protected:
  COLOR *m_Radiance;
  RGB *m_RGB;
  CAMERA m_cam; // Camera used (copied, no pointer!)

  // Parameters
  //  int m_Width, m_Height;
  //  double m_PixWidth, m_PixHeight;
  bool m_Synced;
  float m_Factor;
  float m_AddFactor;
  bool m_RGBImage; // Indicates an RGB image (=no radiance conversion!)

public:
  // Constructor : make an screen buffer from a camera definition. 
  //    If (cam == NULL) the current camera (Camera) is taken
  CScreenBuffer(CAMERA *cam = NULL); // Also calls Init()
  ~CScreenBuffer(void);

  // Inits instance according to current window size
  // Everything set to black
  void Init(CAMERA *cam = NULL);

  void SetRGBImage(bool isRGB)
    { m_RGBImage = isRGB; }
  bool IsRGBImage()
    { return m_RGBImage; }

  // Copy dimensions and contents (m_Radiance only) from source
  void Copy(CScreenBuffer *source);
  
  // Merge (add) two screenbuffers into *this. this overwritten.

  void Merge(CScreenBuffer *src1, CScreenBuffer *src2);

  // Data Access routines. Some could be also defined for a CAMERA class...

  // Screen coordinates <-> pixel number conversions etc.

  inline float GetScreenXMin(void) { return -m_cam.pixh * m_cam.hres / 2.0;}
  inline float GetScreenYMin(void) { return -m_cam.pixv * m_cam.vres / 2.0;}
  inline float GetScreenXMax(void) { return m_cam.pixh * m_cam.hres / 2.0;}
  inline float GetScreenYMax(void) { return m_cam.pixv * m_cam.vres / 2.0;}

  inline float GetPixXSize(void) { return m_cam.pixh; }
  inline float GetPixYSize(void) { return m_cam.pixv; }

  inline VEC2D GetPixelPoint(int nx, int ny, float xoff=0.5, float yoff=0.5) 
  { return VEC2D(GetScreenXMin() + ((float)nx+xoff) * GetPixXSize(), 
		 GetScreenYMin() + ((float)ny+yoff) * GetPixYSize()); }

  inline VEC2D GetPixelCenter(int nx, int ny)
  { return GetPixelPoint(nx, ny, 0.5, 0.5); }

  inline VEC2D GetScreenTopLeft(void)
  { return VEC2D(GetScreenXMin(), GetScreenYMin()); }
  inline VEC2D GetScreenBottomRight(void)
  { return VEC2D(GetScreenXMax(), GetScreenYMax()); }
  
  inline int GetNx(float x)
  { return (int)floor((x - GetScreenXMin())/GetPixXSize()); }
  inline int GetNy(float y)
  { return (int)floor((y - GetScreenYMin())/GetPixYSize()); }

  inline void GetPixel(float x, float y, int *nx, int *ny)
  { *nx = GetNx(x); *ny = GetNy(y); }
  inline void GetPixel(VEC2D pix, int *nx, int *ny)
  { GetPixel(pix.u, pix.v, nx, ny); }

  // (unnormalised) vector pointing from the eye point to the 
  // point with given fractional pixel coordinates.
  inline VECTOR GetPixelVector(int nx, int ny, float xoff =0.5, float yoff =0.5)
  {
    VEC2D pix = GetPixelPoint(nx,ny,xoff,yoff);
    VECTOR dir;
    VECTORCOMB3(m_cam.Z, pix.u, m_cam.X, pix.v, m_cam.Y, dir);
    return dir;
  }

  // Computes screen coordinates of intersection of direction D (pointing
  // from the point of view towards the screen) with the virtual screen.
  // Returns false if the direction points backwards. nx and ny will contain
  // rubbish in that case.
  inline bool GetDirectionScreen(VECTOR D, float *nx, float *ny)
  {
    double DZ = VECTORDOTPRODUCT(D, m_cam.Z);
    if (DZ >= 0.) {
      VECTOR S;
      VECTORADDSCALED(D, -DZ, m_cam.Z, S);
      VECTORSCALEINVERSE(DZ, S, S);
      *nx = VECTORDOTPRODUCT(S, m_cam.X);
      *ny = VECTORDOTPRODUCT(S, m_cam.Y);
      return true;
    }
    return false;
  }


  // tests whether pixel coordinates (x,y) are out of screen
  inline bool OutOfScreen(int x, int y)
  {
    return (x < 0 || x >= m_cam.hres || y < 0 || y >= m_cam.vres);
  }

  // same, but returns pixel coordinates. In addition, this routine also 
  // tests whether the obtained pixel coordinates are within the screen.
  inline bool GetDirectionPixel(VECTOR D, int *x, int *y)
  {
    float nx, ny;
    bool valid = GetDirectionScreen(D, &nx, &ny);
    GetPixel(nx, ny, x, y);
    return (valid && !OutOfScreen(*x,*y));
  }

  // Screen resolution
  inline int GetHRes(void)
  { return m_cam.hres; }
  inline int GetVRes(void)
  { return m_cam.vres; }

  // Get/set radiance of a specified pixel
  COLOR Get(int x, int y);
  void Set(int x, int y, COLOR radiance);

  // Get radiance somewhere on screen using bilinear interpolation
  // of the pixel values
  COLOR GetBilinear(float x, float y);

  // Renders the RGB screen to the window
  void Render(void);
  void RenderScanline(int line);

  // Render a protion of the buffer to the full screen
  void RenderZoomed(int nx0, int ny0, int nx1, int ny1);

  // Write the buffer to an image file
  void WriteFile(ImageOutputHandle *ip);

  // Write the buffer to an image file given a filename
  void WriteFile(char *filename);

  // Adds radiance to a specified pixel
  void Add(int x, int y, COLOR radiance);

  // The factor is just multiplied with the radiance when
  // converting to RGB. This allows for storing flux.

  void SetFactor(float factor);
  void SetAddScaleFactor(float factor);
  void ScaleRadiance(float factor);

  // Synchronize m_Radiance with m_RGB 
  void Sync(void);

  // average pixel luminance natural logarithm after scaling
  double AverageLogLuminance(float scalefactor =1.0);

  // computes median pixel luminance after scaling
  double MedianLuminance(float scalefactor =1.0);

  // determines static adaptation level after scaling using the current
  // adaptation estimation strategy for tone mapping.
  double AdaptationLuminance(float scalefactor =1.0);

  // reprojects the screenbuffer for a new view (determined by the current Camera) */
  // (implemented in raycasting.C)
  void Reproject(void);

protected:
  void SyncLine(int linenr);
};

/************ Utility routines ************/
// Some auxiliary functions to return the start float coord
// of a pixel on screen and to get the float size of a pixel
// Should move to Camera (in C++) or somewhere ????

// Float size of a pixel
/*
inline float PixXSize(void)
{
  return Camera.pixh;
}

inline float PixYSize(void)
{
  return Camera.pixv;
}


// Left side of image plane (not center of pixel !!)
inline float PixXMin(void)
{
  -Camera.pixh * Camera.hres / 2.0;
}

// Upper side 
inline float PixYMin(void)
{
  -Camera.pixv * Camera.vres / 2.0;
}

// Right side
inline float PixXMax(void)
{
  -Camera.pixh * Camera.hres / 2.0;
}

inline float PixYMax(void)
{
  -Camera.pixv * Camera.vres / 2.0;
}

inline float PixXToFloat(int nx)
{
  return -Camera.pixh * Camera.hres / 2.0 + nx * Camera.pixh;
}
inline float PixYToFloat(int ny)
{
  return -Camera.pixv * Camera.vres / 2.0 + ny * Camera.pixv;
}

// Get nearest pixel number (x coord)
inline int FloatToPixX(float x)
{
  return x / PixXSize 
  return -Camera.pixh * Camera.hres / 2.0 + nx * Camera.pixh;
}

// Get nearest pixel number (y coord)
inline int FloatToPixY(float y)
{
  return -Camera.pixv * Camera.vres / 2.0 + ny * Camera.pixv;
}
*/

extern double ComputeFluxToRadFactor(int pix_x, int pix_y);

#else /*__cplusplus*/

typedef void CScreenBuffer;	/* opaque object */

#endif /*__cplusplus*/

#endif
