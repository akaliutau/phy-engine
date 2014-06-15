

#include <stdlib.h>
#include <unistd.h>


#include "trwf.h"
#include "statistics.h"
#include "error.h"
#include "rgb.h"
#include "color.h"
#include "cie.h"
#include "camera.h"






static RGB   f_sf = {0.062, 0.608, 0.330};
static float f_msf, f_pm_comp, f_pm_disp, f_sm_comp, f_sm_disp;
static float invcmax, lrwm_comp, lrwm_disp, lrwexponent;
static float m_comp, m_disp;
static float g, r_comp, r_disp;



static float _lwa;
static float _ldaTumb, _ldaWard;


static float _stevensGamma(float lum)
{
  if (lum > 100.0) return 2.655;
  else             return 1.855 + 0.4 * log10(lum + 2.3e-5);
}


static float _photopicOperator(float logLa)
{
    float r;
    if      (logLa <= -2.6) r = -0.72;
    else if (logLa >=  1.9) r = logLa - 1.255;
    else                    r = pow(0.249*logLa+0.65, 2.7) - 0.72;

    return pow(10.0, r);
}

static float _scotopicOperator(float logLa)
{
    float r;
    if      (logLa <= -3.94) r = -2.86;
    else if (logLa >= -1.44) r = logLa - 0.395;
    else                     r = pow(0.405*logLa+1.6, 2.18) - 2.86;

    return pow(10.0, r);
}

static float _mesopicScaleFactor(float logLwa)
{
    if      (logLwa < -2.5) return 1.0;
    else if (logLwa >  0.8) return 0.0;
    else                    return (0.8-logLwa)/3.3;
}

static void Defaults(void)
{
}

static void Init(void)
{
  float lwa = _lwa = tmopts.lwa;
  float ldmax = tmopts.ldm;
  float cmax = tmopts.cmax;

  float alpharw,betarw,alphad,betad;
  float gwd;

  

  _ldaTumb = ldmax/sqrt(cmax);
#ifdef VERBOSE
  printf("         lda   = %f (TMO_TMBLRUSH)\n",_ldaTumb);
#endif

  {
    float l10 = log10(TMO_CANDELA_LAMBERT(lwa));
    alpharw =                  0.4  *l10 + 2.92;
    betarw  = -0.4*(l10*l10) - 2.584*l10 + 2.0208;
  }
  
  {
    float l10 = log10(TMO_CANDELA_LAMBERT(_ldaTumb));
    alphad  =                  0.4  *l10 + 2.92;
    betad   = -0.4*(l10*l10) - 2.584*l10 + 2.0208;
  }
    
  lrwexponent = alpharw/alphad;
  lrwm_comp   = pow(10.0, (betarw-betad)/alphad);
  lrwm_disp   = lrwm_comp/(TMO_CANDELA_LAMBERT(ldmax));
  invcmax     = 1.0/cmax;

#ifdef VERBOSE
  printf("         (Brw  = %f)\n",
	 pow(10.0, betarw) * pow(TMO_CANDELA_LAMBERT(lwa), alpharw));
  printf("         (Bd   = %f)\n",
	 pow(10.0, betad) * pow(TMO_CANDELA_LAMBERT(_ldaTumb), alphad));
#endif

  

  _ldaWard = ldmax/2.0;
  {
    double p1 = pow(_ldaWard,0.4);
    double p2 = pow(lwa,0.4);
    double p3 = (1.219+p1)/(1.219+p2);
    
    m_comp = pow(p3, 2.5);
  }

  m_disp = m_comp/ldmax;

#ifdef VERBOSE
  printf("         lda   = %f (TMO_WARD)\n",_ldaWard);
  printf("         m_d   = %g (TMO_WARD)\n",m_disp);
  printf("         lwa   = %f\n         ldmax = %f\n         cmax  = %f\n",
	 lwa,ldmax,cmax);
  printf("         m_comp = %g, m_disp = %g (Ward)\n", m_comp, m_disp);
#endif

  

  f_msf     = _mesopicScaleFactor(log10(lwa)); 
  f_sm_comp = _scotopicOperator(log10(_ldaWard)) /
              _scotopicOperator(log10(lwa));
  f_pm_comp = _photopicOperator(log10(_ldaWard)) / 
              _photopicOperator(log10(lwa));
  f_sm_disp = f_sm_comp/ldmax;
  f_pm_disp = f_pm_comp/ldmax;

  

  g      = _stevensGamma(lwa) / _stevensGamma(_ldaTumb);
  gwd    = _stevensGamma(lwa) / (1.855 + 0.4*log(_ldaTumb));
  r_comp = pow(sqrt(cmax), gwd-1) * _ldaTumb;
  r_disp = r_comp/ldmax;
}

static void Terminate(void)
{
}

static COLOR TR_ScaleForComputations(COLOR radiance)
{
  float rwl, scale;

  rwl = ColorLuminance(radiance);

  if (rwl > 0.0) {
    float m = TMO_LAMBERT_CANDELA(
               (pow(TMO_CANDELA_LAMBERT(rwl),lrwexponent)*lrwm_comp));
    scale = m > 0.0 ? m/rwl : 0.0;
  }
  else
    scale = 0.0;

  COLORSCALE(scale, radiance, radiance);
  return radiance;
}

static COLOR TR_ScaleForDisplay(COLOR radiance)
{
  float rwl, scale, eff;

  rwl = M_PI * ColorLuminance(radiance);

  GetLuminousEfficacy(&eff);
  COLORSCALE(eff * M_PI, radiance, radiance);

  if (rwl > 0.0) {
    float m = (pow(TMO_CANDELA_LAMBERT(rwl),lrwexponent)*lrwm_disp-invcmax);
    scale = m > 0.0 ? m/rwl : 0.0;
  }
  else
    scale = 0.0;

  COLORSCALE(scale, radiance, radiance);
  return radiance;
}

static float TR_ReverseScaleForComputations(float dl)
{
  if (dl > 0.0) return pow(dl*3.14e-4/lrwm_comp,1.0/lrwexponent) / 
		  (3.14e-4 * dl);
  else          return 0.0;
}

static float TR_ReverseScaleForDisplay(float dl)
{
  Fatal(-1, "TR_ReverseScaleForDisplay", "Not yet implemented");
  return -1.0;
}

TONEMAP TM_TumblinRushmeier = {
  "Tumblin/Rushmeier's Mapping", "TumblinRushmeier", "tmoTmblRushButton", 3,
  Defaults,
  (void (*)(int *, char **))NULL,
  (void (*)(FILE *))NULL,
  Init,
  Terminate,
  TR_ScaleForComputations,
  TR_ScaleForDisplay,
  TR_ReverseScaleForComputations,
  (void (*)(void *))NULL,
  (void (*)(void *))NULL,
  (void (*)(void))NULL,
  (void (*)(void))NULL
};

static COLOR Ward_ScaleForComputations(COLOR radiance)
{
  COLORSCALE(m_comp, radiance, radiance);
  return radiance;
}

static COLOR Ward_ScaleForDisplay(COLOR radiance)
{
  float eff;

  GetLuminousEfficacy(&eff);

  COLORSCALE(eff * m_disp, radiance, radiance);
  return radiance;
}

static float Ward_ReverseScaleForComputations(float dl)
{
  return 1.0/m_comp;
}

static float Ward_ReverseScaleForDisplay(float dl)
{
  Fatal(-1, "Ward_ReverseScaleForDisplay", "Not yet implemented");
  return -1.0;
}

TONEMAP TM_Ward = {
  "Ward's Mapping", "Ward", "tmoWardButton", 3,
  Defaults,
  (void (*)(int *, char **))NULL,
  (void (*)(FILE *))NULL,
  Init,
  Terminate,
  Ward_ScaleForComputations,
  Ward_ScaleForDisplay,
  Ward_ReverseScaleForComputations,
  (void (*)(void *))NULL,
  (void (*)(void *))NULL,
  (void (*)(void))NULL,
  (void (*)(void))NULL
};

static COLOR RevisedTR_ScaleForComputations(COLOR radiance)
{
  float rwl, scale;

  rwl = ColorLuminance(radiance);

  if (rwl > 0.0) scale = r_comp * pow(rwl/_lwa, g) / rwl;
  else           scale = 0.0;

  COLORSCALE(scale, radiance, radiance);
  return radiance;
}

static COLOR RevisedTR_ScaleForDisplay(COLOR radiance)
{
  float rwl, scale, eff;

  rwl = M_PI * ColorLuminance(radiance);

  GetLuminousEfficacy(&eff);
  COLORSCALE(eff * M_PI, radiance, radiance);

  if (rwl > 0.0) scale = r_disp * pow(rwl/_lwa, g) / rwl;
  else           scale = 0.0;

  COLORSCALE(scale, radiance, radiance);
  return radiance;
}

static float RevisedTR_ReverseScaleForComputations(float dl)
{
  if (dl > 0.0) return _lwa * pow(dl/r_comp, 1.0/g) / dl;
  else          return 0.0;
}

static float RevisedTR_ReverseScaleForDisplay(float dl)
{
  Fatal(-1, "RevisedTR_ReverseScaleForDisplay", "Not yet implemented");
  return -1.0;
}

TONEMAP TM_RevisedTumblinRushmeier = {
  "Revised Tumblin/Rushmeier's Mapping", "RevisedTR", "tmoRevisedTmblRushButton", 3,
  Defaults,
  (void (*)(int *, char **))NULL,
  (void (*)(FILE *))NULL,
  Init,
  Terminate,
  RevisedTR_ScaleForComputations,
  RevisedTR_ScaleForDisplay,
  RevisedTR_ReverseScaleForComputations,
  (void (*)(void *))NULL,
  (void (*)(void *))NULL,
  (void (*)(void))NULL,
  (void (*)(void))NULL
};

static COLOR Ferwerda_ScaleForComputations(COLOR radiance)
{
  RGB p;
  float sl, eff;

  
  GetLuminousEfficacy(&eff);
  COLORSCALE(eff, radiance, radiance);

  
  ColorToRGB(radiance,&p);
  sl = f_sm_comp * f_msf * (p.r*f_sf.r + p.g*f_sf.g + p.b*f_sf.b);

  
  COLORSCALE(f_pm_comp, radiance, radiance);
  
  
  if (sl > 0.0) COLORADDCONSTANT(radiance, sl, radiance);
  
  return radiance;
}

static COLOR Ferwerda_ScaleForDisplay(COLOR radiance)
{
  RGB p;
  float sl, eff;

  
  GetLuminousEfficacy(&eff);
  COLORSCALE(eff, radiance, radiance);

  
  ColorToRGB(radiance,&p);
  sl = f_sm_disp * f_msf * (p.r*f_sf.r + p.g*f_sf.g + p.b*f_sf.b);

  
  COLORSCALE(f_pm_disp, radiance, radiance);

  
  if (sl > 0.0) COLORADDCONSTANT(radiance, sl, radiance);

  return radiance;
}

static float Ferwerda_ReverseScaleForComputations(float dl)
{
  Fatal(-1, "Ferwerda_ReverseScaleForComputations", "Not yet implemented");
  return -1.0;
}

static float Ferwerda_ReverseScaleForDisplay(float dl)
{
  Fatal(-1, "Ferwerda_ReverseScaleForDisplay", "Not yet implemented");
  return -1.0;
}

TONEMAP TM_Ferwerda = {
  "Partial Ferwerda's Mapping", "Ferwerda", "tmoFerwerdaButton", 3,
  Defaults,
  (void (*)(int *, char **))NULL,
  (void (*)(FILE *))NULL,
  Init,
  Terminate,
  Ferwerda_ScaleForComputations,
  Ferwerda_ScaleForDisplay,
  Ferwerda_ReverseScaleForComputations,
  (void (*)(void *))NULL,
  (void (*)(void *))NULL,
  (void (*)(void))NULL,
  (void (*)(void))NULL
};
