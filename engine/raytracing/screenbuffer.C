
#include <stdlib.h>
#include <math.h>

#include "screenbuffer.H"
#include "render.h"
#include "tonemapping.h"
#include "pools.h"
#include "camera.h"

#include "error.h"
#include "statistics.h"
#include "file.h"

// Implementation

CScreenBuffer::CScreenBuffer(CAMERA *cam)
{
  m_Radiance = NULL;
  m_RGB = NULL;
  Init(cam);
}

CScreenBuffer::~CScreenBuffer(void)
{
  if(m_Radiance != NULL)
  {
    Free((char *)m_Radiance, m_cam.hres * m_cam.vres * sizeof(COLOR));
    m_Radiance = NULL;
  }

  if(m_RGB != NULL)
  {
    Free((char *)m_RGB, m_cam.hres * m_cam.vres * sizeof(RGB));
    m_RGB = NULL;
  }
}


void CScreenBuffer::Init(CAMERA *cam)
{
  int i;

  if(cam == NULL) cam = &Camera; // Use the current camera

  if((m_Radiance != NULL) && 
     ((cam->hres != m_cam.hres) ||
      (cam->vres != m_cam.vres)))
  {
    Free((char *)m_RGB, m_cam.hres * m_cam.vres * sizeof(RGB));
    Free((char *)m_Radiance, m_cam.hres * m_cam.vres * sizeof(COLOR));
    m_RGB = NULL;
    m_Radiance = NULL;
  }

  m_cam = *cam;

  if(m_Radiance == NULL)
  {
    m_Radiance = (COLOR *)Alloc(m_cam.hres * m_cam.vres * 
				sizeof(COLOR));
    m_RGB = (RGB *)Alloc(m_cam.hres * m_cam.vres * 
			 sizeof(RGB));
    // m_PixWidth = m_cam.pixh;
    // m_PixHeight = m_cam.pixv;
  }

  // Clear

  for(i=0; i < m_cam.hres*m_cam.vres; i++)
  {
    COLORSETMONOCHROME(m_Radiance[i], 0.);
    m_RGB[i] = Black;
  }

  m_Factor = 1.0;
  m_AddFactor = 1.0;
  m_Synced = true;
  m_RGBImage = false;
}

// Copy dimensions and contents (m_Radiance only) from source
void CScreenBuffer::Copy(CScreenBuffer *source)
{
  Init(&(source->m_cam));
  m_RGBImage = source->IsRGBImage();

  // Now the resolution is ok.

  memcpy(m_Radiance, source->m_Radiance, m_cam.hres * m_cam.vres * sizeof(COLOR));
  m_Synced = false;
}

// Merge (add) two screenbuffers (m_Radiance only) from src1 and src2
void CScreenBuffer::Merge(CScreenBuffer *src1, CScreenBuffer *src2)
{
  Init(&(src1->m_cam));
  m_RGBImage = src1->IsRGBImage();

  if((GetHRes() != src2->GetHRes()) || (GetVRes() != src2->GetVRes()))
  {
    Error("CScreenBuffer::Merge", "Incompatible screen buffer sources");
    return;
  }

  int N = GetVRes() * GetHRes();

  for(int i = 0; i < N; i++)
  {
    COLORADD(src1->m_Radiance[i], src2->m_Radiance[i], m_Radiance[i]);
  }
}

void CScreenBuffer::Add(int x, int y, COLOR radiance)
{
  int index = x + (m_cam.vres - y - 1) * m_cam.hres;

  COLORADDSCALED(m_Radiance[index], m_AddFactor, radiance, 
		 m_Radiance[index]);
  m_Synced = false;
}

void CScreenBuffer::Set(int x, int y, COLOR radiance)
{
  int index = x + (m_cam.vres - y - 1) * m_cam.hres;
  COLORSCALE(m_AddFactor, radiance, m_Radiance[index]);
  m_Synced = false;
}

void CScreenBuffer::ScaleRadiance(float factor)
{
  for(int i = 0; i < m_cam.hres * m_cam.vres; i++)
  {
    COLORSCALE(factor, m_Radiance[i], m_Radiance[i]);
  }

  m_Synced = false;
}

// average pixel luminance natural logarithm
double CScreenBuffer::AverageLogLuminance(float scale)
{
  double sumloglum = 0.;
  int count = 0;
  for (int i = 0; i < m_cam.hres * m_cam.vres; i++) {
    double brightness = scale * M_PI * ColorLuminance(m_Radiance[i]);
    if (brightness < EPSILON) continue;
    sumloglum += log(brightness);
    count++;
  }
  return sumloglum / (double)count;
}

// compares two double floating point values pointed to by v1 and v2
static int fcmp(const void *v1, const void *v2)
{
  float x = *(float *)v1;
  float y = *(float *)v2;
  return (x > y) ? +1 : (x < y) ? -1 : 0;
}

// computes median pixel luminance 
double CScreenBuffer::MedianLuminance(float scale)
{
  int npix = m_cam.vres * m_cam.hres;
  float *lum = new float[npix], median;
  int i, j;

  for (i=0, j=0; i<npix; i++) { 
    float l = scale * M_PI * ColorLuminance(m_Radiance[i]); 
    if (l > EPSILON)
      lum[j++] = l;
  }
  qsort((void*)lum, j, sizeof(float), fcmp);
  median = lum[j/2];

  delete lum;
  return median;
}

// determines static adaptation level after scaling using the current
// adaptation estimation strategy for tone mapping.
double CScreenBuffer::AdaptationLuminance(float scale)
{
  fprintf(stderr, "Exp(Average Log(luminance)): %g Cd/m^2\n", exp(AverageLogLuminance(scale)));
  fprintf(stderr, "Median luminance          : %g Cd/m^2\n", MedianLuminance(scale));

  switch (tmopts.statadapt) {
  case TMA_NONE:
    return 1.0;
  case TMA_AVERAGE:
    
    return exp(AverageLogLuminance(scale) );
  case TMA_MEDIAN:
    
    return MedianLuminance(scale);
  default:
    Error("CScreenBuffer::StaticAdaptationLuminance", "Invalid adaptation estimation method '%s'\n", tmopts.statadapt);
  }
  return 1.0;
}

COLOR CScreenBuffer::Get(int x, int y)
{
  int index = x + (m_cam.vres - y - 1) * m_cam.hres;

  return m_Radiance[index];
}


COLOR CScreenBuffer::GetBilinear(float x, float y)
{
  int nx0, nx1, ny0, ny1;
  VEC2D center;
  COLOR col;

  GetPixel(x,y, &nx0, &ny0);
  center = GetPixelCenter(nx0, ny0);

  x = (x - center.u) / GetPixXSize();
  y = (y - center.v) / GetPixYSize();

  if(x < 0)
  {
    // Point on left side of pixel center
    x = -x; nx1 = MAX(nx0 - 1, 0);
  }
  else
  {
    nx1 = MIN(GetHRes(), nx0 + 1);
  }

  if(y < 0)
  {
    y = -y; ny1 = MAX(ny0 - 1, 0);
  }
  else
  {
    ny1 = MIN(GetVRes(), ny0 + 1);
  }

  // u = 0 for nx0 and u = 1 for nx1, x inbetween. Not that
  // nx0 and nx1 may be the same (at border of image). Same for ny

  COLOR c0, c1, c2, c3; // Separate vars, since interpolation is a macro...

  c0 = Get(nx0,ny0);
  c1 = Get(nx1,ny0); // u = 1
  c2 = Get(nx1,ny1); // u = 1, v = 1
  c3 = Get(nx0,ny1); // v = 1

  COLORINTERPOLATEBILINEAR(c0,c1,c2,c3, x, y, col);

  return col;
}


void CScreenBuffer::SetFactor(float factor)
{
  m_Factor = factor;
}

void CScreenBuffer::SetAddScaleFactor(float factor)
{
  m_AddFactor = factor;
}

void CScreenBuffer::Render(void)
{
  if(!m_Synced)
    Sync();

  RenderPixels(0, 0, m_cam.hres, m_cam.vres, m_RGB);
}

void CScreenBuffer::RenderZoomed(int nx0, int ny0, int nx1, int ny1)
{
  float dx, dy, x, y;
  VEC2D p0, p1;
  COLOR tmpRad;

  m_Synced = true; // We're going to mess up m_RGB;

  p0 = GetPixelCenter(nx0, ny0);
  p1 = GetPixelCenter(nx1, ny1);

  dx = (p1.u - p0.u) / m_cam.hres;
  dy = (p1.v - p0.v) / m_cam.vres;

  x = p0.u;
  y = p1.v;

  int index = 0;

  for(int i = 0; i < m_cam.vres; i++)
  {
    for(int j = 0; j < m_cam.hres; j++)
    {
      tmpRad = GetBilinear(x,y);
      COLORSCALE(m_Factor, tmpRad, tmpRad);
      RadianceToRGB(tmpRad, &m_RGB[index]);

      index += 1;
      x += dx;
    }

    x = p0.u;
    y -= dy;
  }

  RenderPixels(0, 0, m_cam.hres, m_cam.vres, m_RGB);
}

void CScreenBuffer::WriteFile(ImageOutputHandle *ip)
{
  if (!ip) return;

  if(!m_Synced)
    Sync();

  fprintf(stderr, "Writing %s file ... ", ip->drivername);

  ip->gamma[0] = tmopts.gamma.r;	// for default radiance -> display RGB
  ip->gamma[1] = tmopts.gamma.g;
  ip->gamma[2] = tmopts.gamma.b;
  for (int i=m_cam.vres-1; i>=0; i--)	// write scanlines
  {
#ifdef RGBCOLORS
    if(!IsRGBImage())
      ip->WriteRadianceRGB((float *)&m_Radiance[i * m_cam.hres]);
    else
      ip->WriteDisplayRGB((float *)&m_Radiance[i * m_cam.hres]);
#else
    ip->WriteRadianceXYZ((float *)&m_Radiance[i * m_cam.hres]);
#endif
  }

  fprintf(stderr, "done.\n");
}

void CScreenBuffer::WriteFile(char *filename)
{
  int ispipe;
  FILE *fp = OpenFile(filename, "w", &ispipe);
  if (!fp) return;

  ImageOutputHandle *ip = 
    CreateRadianceImageOutputHandle(filename, fp, ispipe,
				    m_cam.hres, m_cam.vres,
				    reference_luminance/179.);

  WriteFile(ip);

  // DeleteImageOutputHandle(ip);
  CloseFile(fp, ispipe);
}

void CScreenBuffer::RenderScanline(int i)
{
  i = m_cam.vres - i - 1;

  if(!m_Synced)
    SyncLine(i);

  RenderPixels(0, i, m_cam.hres, 1, &m_RGB[i*m_cam.hres]);
}

void CScreenBuffer::Sync(void)
{
  int i;
  COLOR tmpRad;

  for(i = 0; i < m_cam.hres * m_cam.vres; i++)
  {
    COLORSCALE(m_Factor, m_Radiance[i], tmpRad);
    if(!IsRGBImage())
      RadianceToRGB(tmpRad, &m_RGB[i]);
    else
    {
      ColorToRGB(tmpRad, &m_RGB[i]);
    }
  }

  m_Synced = true;
}


void CScreenBuffer::SyncLine(int lineNr)
{
  int i;
  COLOR tmpRad;

  for(i = 0; i < m_cam.hres; i++)
  {
    COLORSCALE(m_Factor, m_Radiance[lineNr * m_cam.hres + i], tmpRad);
    if(!IsRGBImage())
      RadianceToRGB(tmpRad, &m_RGB[lineNr * m_cam.hres + i]);
    else
    {
      ColorToRGB(tmpRad, &m_RGB[lineNr * m_cam.hres + i]);
    }
  }
}



double ComputeFluxToRadFactor(int pix_x, int pix_y)
{
  double x,y, xsample, ysample;
  VECTOR dir;
  double distPixel2, distPixel, factor;
  double h = Camera.pixh;
  double v = Camera.pixv;

  x = -h * Camera.hres / 2.0 + pix_x * h;
  y = -v * Camera.vres / 2.0 + pix_y * v;

  xsample = x + h * 0.5;  // pix_x, Pix_y indicate upper left
  ysample = y + v * 0.5;

  VECTORCOMB3(Camera.Z, xsample, Camera.X, ysample, Camera.Y, dir);
  distPixel2 = VECTORNORM2(dir);
  distPixel = sqrt(distPixel2);
  VECTORSCALEINVERSE(distPixel, dir, dir);

  factor = 1.0 / (h * v);

  factor *= distPixel2; // r(eye->pixel)^2
  factor /= pow(VECTORDOTPRODUCT(dir, Camera.Z), 2);  // cos^2

  return factor;
}
