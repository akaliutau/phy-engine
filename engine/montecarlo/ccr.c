

#include "scene.h"

#include "mcradP.h"
#include "ccr.h"

static COLOR *(*get_radiance)(ELEMENT *);
static COLOR (*get_scaling)(ELEMENT *);


static void InitialControlRadiosity(COLOR *minRad, COLOR *maxRad, COLOR *fmin, COLOR *fmax)
{
  COLOR totalflux, maxrad; 
  double area=0.;
  COLORCLEAR(totalflux);
  COLORCLEAR(maxrad);

  
  ForAllPatches(P, Patches) {
    REC_ForAllSurfaceLeafs(elem, TOPLEVEL_ELEMENT(P)) {
      COLOR rad = get_radiance(elem)[0];
      float warea = elem->area;	
#ifdef IDMCR
      if (mcr.importance_driven && mcr.method!=RWR) {
	warea *= (elem->imp - elem->source_imp); 
      }
#endif
      
      COLORADDSCALED(totalflux,  warea, rad, totalflux);
      area += warea;
      COLORMAX(maxrad, rad, maxrad);
    } REC_EndForAllSurfaceLeafs;
  } EndForAll;

  COLORCLEAR(*minRad);
  *fmin = totalflux;

  *maxRad = maxrad;
  COLORSCALE(area, maxrad, *fmax);
  COLORSUBTRACT(*fmax, totalflux, *fmax);
}

#define NRINTERVALS 10
static void RefineComponent(float *minRad, float *maxRad, float *fmin, float *fmax,
			    float *f, float *rad)
{
  int i, imin;

  
  *fmax = f[0]; *fmin = f[0], imin=0;
  for (i=1; i<=NRINTERVALS; i++) {
    if (f[i] < *fmin) { *fmin = f[i]; imin = i; }
    if (f[i] > *fmax) { *fmax = f[i]; }
  }
  
  if (imin == 0) {			
    *minRad = rad[0];
    *maxRad = rad[1];
  } else if (imin == NRINTERVALS) { 	
    *minRad = rad[NRINTERVALS-1];
    *maxRad = rad[NRINTERVALS];
  } else {				
    if (f[imin-1] < f[imin+1]) {	
      *minRad = rad[imin-1];
      *maxRad = rad[imin];
    } else {				
      *minRad = rad[imin];
      *maxRad = rad[imin+1];
    }
  }
}


static void RefineControlRadiosity(COLOR *minRad, COLOR *maxRad, COLOR *fmin, COLOR *fmax)
{
  COLOR color_one;
  COLOR f[NRINTERVALS+1], rad[NRINTERVALS+1], d;
  int i, s;

  COLORSETMONOCHROME(color_one, 1.);

  
  COLORSUBTRACT(*maxRad, *minRad, d);
  for (i=0; i<=NRINTERVALS; i++) {
    COLORCLEAR(f[i]);
    COLORADDSCALED(*minRad, (double)i/(double)NRINTERVALS, d, rad[i]);
  }

  
  ForAllPatches(P, Patches) {
    REC_ForAllSurfaceLeafs(elem, TOPLEVEL_ELEMENT(P)) {
      COLOR B = get_radiance(elem)[0];
      COLOR s = get_scaling ? get_scaling(elem) : color_one;
      float warea = elem->area;	
#ifdef IDMCR
      if (mcr.importance_driven && mcr.method!=RWR) {
	warea *= (elem->imp - elem->source_imp); 
      }
#endif
      for (i=0; i<=NRINTERVALS; i++) {
	COLOR t;
	COLORPROD(s, rad[i], t);
	COLORSUBTRACT(B, t, t);
	COLORABS(t, t);
	COLORADDSCALED(f[i], warea, t, f[i]);
      }
    } REC_EndForAllSurfaceLeafs;
  } EndForAll;

  
  for (s=0; s<SPECTRUM_CHANNELS; s++) {
    float fc[NRINTERVALS+1], radc[NRINTERVALS+1];
    for (i=0; i<=NRINTERVALS; i++) {	
      fc[i] = f[i].spec[s];
      radc[i] = rad[i].spec[s];
    }
    RefineComponent(&(minRad->spec[s]), &(maxRad->spec[s]),
		    &(fmin->spec[s]), &(fmax->spec[s]), fc, radc);
  }
}
#undef NRINTERVALS

COLOR DetermineControlRadiosity(COLOR *(*GetRadiance)(ELEMENT *),
				COLOR (*GetScaling)(ELEMENT *))
{
  COLOR minRad, maxRad, fmin, fmax, beta, delta, f_orig;
  float eps = 0.001;
  int sweep = 0;

  get_radiance = GetRadiance;
  get_scaling = GetScaling;
  COLORCLEAR(beta);
  if (!get_radiance) return beta;

  fprintf(stderr, "Determining optimal control radiosity value ... ");
  InitialControlRadiosity(&minRad, &maxRad, &fmin, &fmax);
  f_orig = fmin;	

  COLORSUBTRACT(fmax, fmin, delta);
  COLORADDSCALED(delta, (-eps), fmin, delta);
  while ((COLORMAXCOMPONENT(delta) > 0.) || sweep<4) {
    sweep++;
    
    RefineControlRadiosity(&minRad, &maxRad, &fmin, &fmax);
    
    COLORSUBTRACT(fmax, fmin, delta);
    COLORADDSCALED(delta, (-eps), fmin, delta);
  }

  COLORADD(minRad, maxRad, beta); COLORSCALE(0.5, beta, beta);
  
  ColorPrint(stderr, beta);
  fprintf(stderr, " (%g lux)", M_PI * ColorLuminance(beta));
  
  fprintf(stderr, "\n");
  
  return beta;
}
