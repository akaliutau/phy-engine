

#include <string.h>

#include "tests.h"
#include "scene.h"
#include "statistics.h"
#include "color.h"
#include "mcradP.h"
#include "error.h"
#include "file.h"
#include "element.h"







                           
                           


                           

#ifdef WHITETEST
static FILE *test = NULL;
#endif

#ifdef VARTEST
static FILE *vartestfp;
#endif

void InitTests(void)
{
  fprintf(stderr, "Testing options: %s%s%s%s%s%s%s\n",
#ifdef STATS
	  "STATS ",
#else
	  "",
#endif 
#ifdef WHITETEST
	  "WHITETEST ",
#else
	  "",
#endif
#ifdef VARTEST
	  "VARTEST ",
#else
	  "",
#endif
#ifdef FORMFACTORS
	  "FORMFACTORS ",
#else
	  "",
#endif
#ifdef INFOSTUFF
	  "INFOSTUFF ",
#else
	  "",
#endif
#ifdef NO_REJECTION_SAMPLING
	  "NO_REJECTION_SAMPLING ",
#else
	  "",
#endif
	  "");

#ifdef WHITETEST
  test = (FILE *)NULL;
#endif

#ifdef VARTEST
  vartestfp = (FILE *)NULL;
#endif
}

#ifdef VAR_ESTIMATES
COLOR ElementRMSErrorEstimate(ELEMENT *elem)
{
  COLOR rmse, mse = ElementVariance(elem);
  int i;
  for (i=0; i<SPECTRUM_CHANNELS; i++) {
    rmse.spec[i] = sqrt(mse.spec[i]);
  }
  return rmse;
}
#endif

#ifdef WHITETEST
static void do_whitetest(void)
{
  double maxerr, sumerr, rmserr;

  maxerr = sumerr = rmserr = 0.;
  ForAllPatches(p, Patches) {
    ELEMENT *elem = TOPLEVEL_ELEMENT(p);
    
    double err = fabs(ColorLuminance(elem->rad[0])*M_PI - 1.);
    if (mcr.importance_driven) {
      
      err *= elem->source_imp;
      rmse *= elem->source_imp;
      if (elem->source_imp > 0.)
	nrelems++;
    } else
      nrelems++;

    if (err > maxerr) maxerr = err;
    sumerr += p->area * err;
    rmserr += p->area * err * err;
    
  } EndForAll;
  sumerr /= total_area;
  rmserr /= total_area; rmserr = sqrt(rmserr);

  fprintf(test, "%ld %g %g %g %g\n", mcr.traced_rays, sumerr, rmserr, maxerr, mcr.cpu_secs);
  fflush(test);
  fprintf(stderr, "whitetest: %g secs, %ld samples: %g av error, %g rms error, %g max error\n",
	  mcr.cpu_secs, mcr.traced_rays, sumerr, rmserr, maxerr);
}
#endif

#ifdef STATS
#include "raycasting.h"

#define COLOR_SQUARE_ROOT(c, d)	{\
  (d).spec[0] = sqrt((c).spec[0]); \
  (d).spec[1] = sqrt((c).spec[1]); \
  (d).spec[2] = sqrt((c).spec[2]); \
}

static void SaveElementData(FILE *fp, ELEMENT *elem)
{
  double L;
  COLOR rad, mse, msediff, rmse;
  SHOW_WEIGHTING save_show_weighted = mcr.show_weighted;

  fprintf(fp, "%ld  ", (long)elem->id);
  

  mcr.show_weighted = SHOW_NON_WEIGHTED;
  rad = ElementDisplayRadiance(elem);
  ColorPrint(fp, rad);

  fprintf(fp, " ");
  mse = ElementVariance(elem);
  ColorPrint(fp, mse);

  fprintf(fp, "   ");
  mcr.show_weighted = SHOW_WEIGHTED;
  rad = ElementDisplayRadiance(elem);
  ColorPrint(fp, rad);

  msediff = ElementWeightingMSEDiff(elem);
  COLORADD(mse, msediff, mse);
  fprintf(fp, " ");
  ColorPrint(fp, mse);

  fprintf(fp, "\n");

  mcr.show_weighted = save_show_weighted;
}

static void SaveData(char *filename)
{
  int ispipe;
  FILE *fp = OpenFile(filename, "w", &ispipe);
  if (!fp) {
    Error(NULL, "Can't open file '%s' for writing", filename);
    return;
  }
  
  ForAllPatches(P, Patches) {
    SaveElementData(fp, TOPLEVEL_ELEMENT(P));
  } EndForAll;

  CloseFile(fp, ispipe);
}

static void SaveImage(char *filename)
{
  int o_no_smoothing = mcr.no_smoothing;
  int ispipe;
  FILE *fp = OpenFile(filename, "w", &ispipe);
  if (!fp) {
    Error(NULL, "Can't open file '%s' for writing", filename);
    return;
  }

  mcr.no_smoothing = TRUE;
  RayCast(filename, fp, ispipe);
  mcr.no_smoothing = o_no_smoothing;

  CloseFile(fp, ispipe);
}
#endif

#ifdef VARTEST
static void do_vartest(void)
{
  fprintf(vartestfp, "%g %ld %ld   ",
	  mcr.cpu_secs,
	  mcr.traced_rays, mcr.traced_paths);

  ForAllPatches(p, Patches) {
    double err = fabs(ColorLuminance(RAD(p)[0])*M_PI - 1.);
    fprintf(vartestfp, " %g", p->area * err * err);
  } EndForAll;

  fprintf(vartestfp, "\n"); fflush(vartestfp);
}
#endif 

static char *estimator_kind_name(void)
{
  static char buf[100];
  switch (mcr.rw_estimator_kind) {
  case RW_COLLISION: 	sprintf(buf, "collision"); break;
  case RW_ABSORPTION: 	sprintf(buf, "absorption"); break;
  case RW_SURVIVAL: 	sprintf(buf, "survival"); break;
  case RW_LAST_BUT_NTH: sprintf(buf, "last_but_%d", mcr.rw_numlast); break;
  case RW_NLAST: 	sprintf(buf, "%d_last", mcr.rw_numlast); break;
  default: return "UNKNOWN";
  }
  return buf;
}

static char *estimator_type_name(void)
{
  switch (mcr.rw_estimator_type) {
  case RW_SHOOTING: return "shooting";
  case RW_GATHERING: return "gathering";
  default: return "UNKNOWN";
  }
}

static char *phy_basename(void)
{
  static char _phy_basename[100];
  char *seqname = SEQ4D_NAME(mcr.sequence);
  char *approx = approxdesc[mcr.approx_type].name;

  char method[100]; int n;
  sprintf(method, "%s%n",
	  METHOD_SHORT_NAME(mcr.method), &n);
  if (mcr.method == RWR) {
    sprintf(method+n, "-%s-%s-%s",
	    mcr.continuous_random_walk ? "cont" : "discrete",
	    estimator_kind_name(),
	    estimator_type_name());	    
  }
  sprintf(_phy_basename, "%s-%s-%s%s%s%s",
	  method, approx, seqname,
	  mcr.indirect_only ? "-indirect" : "",
	  mcr.bidirectional_transfers ? "-bi" : "",
	  mcr.constant_control_variate ? "-ccr" : ""
	  );		
  return _phy_basename;
}

void do_tests(void)
{
  float saved_cpu_secs = mcr.cpu_secs;		

#ifdef WHITETEST
  if (!test) {
    char fname[100];
    sprintf(fname, "%s.wt", phy_basename());
    test = fopen(fname, "w");
    fprintf(stderr, "%saving whitetest stats to '%s'\n",
	    test ? "S" : "NOT s",
	    fname);
  }
  if (test) do_whitetest();
#endif

#ifdef STATS
{
  static int lastlogit = -1;
  if ((int)(floor(log10((double)mcr.iteration_nr)*10.)) != lastlogit) {
    char fname[100]; 
    
    sprintf(fname, "%s-%d.dump.gz", phy_basename(), mcr.iteration_nr);
    fprintf(stderr, "Dumping to file '%s' ...\n", fname);
    SaveData(fname);
  }
  lastlogit = (int)(floor(log10((double)mcr.iteration_nr) * 10.));
}
#endif 

#ifdef VARTEST
  if (!vartestfp) {
    char fname[100];
    sprintf(fname, "%s.wterrs", phy_basename());
    vartestfp = fopen(fname, "w");
    fprintf(stderr, "%saving measured square errors to '%s'\n",
	    vartestfp ? "S" : "NOT s",
	    fname);
  }
  if (vartestfp) do_vartest();
#endif

  mcr.cpu_secs = saved_cpu_secs;
}

#ifdef PLOTBETA

void PlotBeta(void)
{
  COLOR f[1001], rad[1001], d, totflux, maxrad, bestf, maxRad, minRad;
  double totarea;
  int i;
  FILE *fp = fopen("beta.dat", "w");

  totarea = 0.; COLORCLEAR(totflux); COLORCLEAR(maxrad);
  ForAllPatches(P, Patches) {
    COLOR B = RAD(P)[0];
    totarea += P->area;
    COLORADDSCALED(totflux,  P->area, B, totflux);
    COLORMAX(maxrad, B, maxrad);
  } EndForAll;

  fprintf(stderr, "totarea = %g, totflux = ", totarea);
  ColorPrint(stderr, totflux);
  fprintf(stderr, ", maxrad = ");
  ColorPrint(stderr, maxrad);
  fprintf(stderr, "\n");

  COLORCLEAR(minRad);
  COLORSCALE((2. / totarea), totflux, maxRad);
  COLORSUBTRACT(maxRad, minRad, d);
  for (i=0; i<=1000; i++) {
    COLORCLEAR(f[i]);
    COLORADDSCALED(minRad, (double)i/(double)1000, d, rad[i]);
  }

  ForAllPatches(P, Patches) {
    COLOR B = RAD(P)[0];
    for (i=0; i<=1000; i++) {
      COLOR t;
      COLORSUBTRACT(B, rad[i], t);
      COLORABS(t, t);
      COLORADDSCALED(f[i], P->area, t, f[i]);
    }
  } EndForAll;

  bestf = f[0];
  for (i=0; i<=1000; i++) {
    int s;
    for (s=0; s<SPECTRUM_CHANNELS; s++) {
      float rads = rad[i].spec[s];
      float fs = f[i].spec[s];
      float totfluxs = totflux.spec[s];
      float maxrads = maxrad.spec[s];
      fprintf(fp, "%g %g %g %g    ", rads, fs, 
	      totfluxs - totarea * rads,
	      totarea * maxrads - totfluxs + totarea * (rads - maxrads));
    }
    fprintf(fp, "\n");
    COLORMIN(bestf, f[i], bestf);
  }

  fclose(fp);

  fprintf(stderr, "best F = "); ColorPrint(stderr, bestf); fprintf(stderr, "\n");
}
#endif 
