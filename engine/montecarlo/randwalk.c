

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "error.h"
#include "scene.h"
#include "statistics.h"

#include "mcrad.h"
#include "mcradP.h"
#include "tracepath.h"
#include "stochjacobi.h"
#include "ccr.h"

static void RwrPrintPatchData(FILE *out, PATCH *patch)
{
  PrintElement(out, TOPLEVEL_ELEMENT(patch));
}

static void RwrInit(void)
{
  mcr.method = RWR;
  McrInit();
}

static void PrintStats(void)
{
  fprintf(stderr, "%g secs., total radiance rays = %ld",
	  mcr.cpu_secs, mcr.traced_rays);
  fprintf(stderr, ", total flux = ");
  ColorPrint(stderr, mcr.total_flux);
  if (mcr.importance_driven)
    fprintf(stderr, "\ntotal importance rays = %ld, total importance = %g, total_area = %g", 
	    mcr.imp_traced_rays, mcr.total_ymp, total_area);
  fprintf(stderr, "\n");
}


static double PatchArea(PATCH *P)
{
  return P->area;
}


static double ScalarSourcePower(PATCH *P)
{
  COLOR radiance = SOURCE_RAD(P);
  return  P->area * COLORSUMABSCOMPONENTS(radiance);
}


static double ScalarReflectance(PATCH *P)
{
  return McrScalarReflectance(P);
}

static COLOR *GetSelfEmittedRadiance(ELEMENT *elem)
{
  static COLOR Ed[MAX_BASIS_SIZE];
  CLEARCOEFFICIENTS(Ed, elem->basis);
  Ed[0] = EMITTANCE(elem->pog.patch);
  return Ed;
}

static COLOR *GetSourceRadiance(ELEMENT *elem)
{
  static COLOR Ed[MAX_BASIS_SIZE];
  CLEARCOEFFICIENTS(Ed, elem->basis);
  Ed[0] = elem->source_rad;
  return Ed;
}


static void ReduceSource(void)
{
  ForAllPatches(P, Patches) {
    COLOR newsrcrad, rho;

    COLORSETMONOCHROME(newsrcrad, 1.);
    rho = REFLECTANCE(P);
    COLORSUBTRACT(newsrcrad, rho, newsrcrad);	
    COLORPROD(newsrcrad, mcr.control_radiance, newsrcrad);  
    COLORSUBTRACT(SOURCE_RAD(P), newsrcrad, newsrcrad);	
    SOURCE_RAD(P) = newsrcrad;
  } EndForAll;
}

static double ScoreWeight(PATH *path, int n)
{
  double w = 0.;
  int t = path->nrnodes - ((mcr.rw_numlast>0) ? mcr.rw_numlast : 1);

  switch (mcr.rw_estimator_kind) {
  case RW_COLLISION:
    w = 1.; 
    break;
  case RW_ABSORPTION:
    if (n == path->nrnodes-1) 	
      w = 1./(1. - path->nodes[n].probability);
    break;
  case RW_SURVIVAL:
    if (n < path->nrnodes-1)	
      w = 1./path->nodes[n].probability;
    break;
  case RW_LAST_BUT_NTH:
    if (n == t-1) {
      int i = path->nrnodes-1;
      PATHNODE *node = &path->nodes[i];
      w = 1./(1. - node->probability);	
      for (i--, node--; i>=n; i--, node--)
	w /= node->probability;	   
    }
    break;
  case RW_NLAST:
    if (n == t) {
      
      w = 1./(1. - path->nodes[path->nrnodes-1].probability);      
    } else if (n > t) {
      w = 1.;
    }
    break;
  default:
    Fatal(-1, "ScoreWeight", "Unknown random walk estimator kind %d", mcr.rw_estimator_kind);
  }
  return w;
}

static void ShootingScore(PATH *path, long nr_paths, double (*birth_prob)(PATCH *))
{
  COLOR accum_pow;
  int n;
  PATHNODE *node = &path->nodes[0];

  
  COLORSCALE((node->patch->area/node->probability), SOURCE_RAD(node->patch), accum_pow);
  for (n=1, node++; n<path->nrnodes; n++, node++) {
    double uin=0., vin=0., uout=0., vout=0., r=1., w;
    int i;
    PATCH *P = node->patch;
    COLOR Rd = REFLECTANCE(P);
    COLORPROD(accum_pow, Rd, accum_pow);

#ifdef HOMCR
    PatchUniformUV(P, &node->inpoint, &uin, &vin);
    if (!mcr.continuous_random_walk) {
      r=0.;
      if (n<path->nrnodes-1) {
	
	PatchUniformUV(P, &node->outpoint, &uout, &vout);
      }
    }
#endif 

    w = ScoreWeight(path, n);

    for (i=0; i<BAS(P)->size; i++) {
      double dual = BAS(P)->dualfunction[i](uin,vin) / P->area;
      COLORADDSCALED(RECEIVED_RAD(P)[i], (w*dual/(double)nr_paths), accum_pow, RECEIVED_RAD(P)[i]);

#ifdef HOMCR
      if (!mcr.continuous_random_walk) {
	double basf = BAS(P)->function[i](uout, vout);
	r += dual * P->area * basf;
      }
#endif 
    }

    COLORSCALE((r/node->probability), accum_pow, accum_pow);
  }
}

static void ShootingUpdate(PATCH *P, double w)
{
  double k, old_quality;
  old_quality = QUALITY(P);
  QUALITY(P) += w;
  if (QUALITY(P) < EPSILON)
    return;
  k = old_quality / QUALITY(P);

  
  COLORSUBTRACT(RAD(P)[0], SOURCE_RAD(P), RAD(P)[0]);

  
  SCALECOEFFICIENTS(k, RAD(P), BAS(P));
  SCALECOEFFICIENTS((1.-k), RECEIVED_RAD(P), BAS(P));
  ADDCOEFFICIENTS(RAD(P), RECEIVED_RAD(P), BAS(P));

  
  COLORADD(RAD(P)[0], SOURCE_RAD(P), RAD(P)[0]);

  
  CLEARCOEFFICIENTS(UNSHOT_RAD(P), BAS(P));
  CLEARCOEFFICIENTS(RECEIVED_RAD(P), BAS(P));
}

static void DoShootingIteration(void)
{
  long nr_walks; 

  nr_walks = mcr.initial_nr_rays;
#ifdef HOMCR
  if (mcr.continuous_random_walk)
    nr_walks *= approxdesc[mcr.approx_type].basis_size;
  else
    nr_walks *= pow(approxdesc[mcr.approx_type].basis_size, 1./(1. - COLORMAXCOMPONENT(average_reflectivity)));
#endif

  fprintf(stderr, "Shooting iteration %d (%ld paths, approximately %ld rays)\n",
	  mcr.iteration_nr,
	  nr_walks, (long)floor((double)nr_walks / (1. - COLORMAXCOMPONENT(average_reflectivity))));

  TracePaths(nr_walks,
	     ScalarSourcePower, ScalarReflectance,
	     ShootingScore,
	     ShootingUpdate);
}


static COLOR DetermineGatheringControlRadiosity(void)
{
  COLOR c1, c2, cr;
  COLORCLEAR(c1); COLORCLEAR(c2);
  ForAllPatches(P, Patches) {
    COLOR absorb, rho, Ed, num, denom;

    COLORSETMONOCHROME(absorb, 1.);
    rho = REFLECTANCE(P);
    COLORSUBTRACT(absorb, rho, absorb);	

    Ed = SOURCE_RAD(P);
    COLORPROD(absorb, Ed, num);
    COLORADDSCALED(c1, P->area, num, c1);	

    COLORPROD(absorb, absorb, denom);
    COLORADDSCALED(c2, P->area, denom, c2);	
  } EndForAll;

  COLORDIV(c1, c2, cr);
  fprintf(stderr, "Control radiosity value = "); ColorPrint(stderr, cr); fprintf(stderr, ", luminosity = %g\n", ColorLuminance(cr));

  return cr;
}

static void CollisionGatheringScore(PATH *path, long nr_paths, double (*birth_prob)(PATCH *))
{
  COLOR accum_rad;
  int n;
  PATHNODE *node = &path->nodes[path->nrnodes-1];
  accum_rad = SOURCE_RAD(node->patch);
  for (n=path->nrnodes-2, node--; n>=0; n--, node--) {
    double uin=0., vin=0., uout=0., vout=0., r=1.;
    int i;
    PATCH *P = node->patch;
    COLOR Rd = REFLECTANCE(P);
    COLORPROD(Rd, accum_rad, accum_rad);

#ifdef HOMCR
    PatchUniformUV(P, &node->outpoint, &uout, &vout);
    if (!mcr.continuous_random_walk) {
      r=0.;
      if (n>0) {
	
	PatchUniformUV(P, &node->inpoint, &uin, &vin);
      }
    }
#endif 

    for (i=0; i<BAS(P)->size; i++) {
      double dual = BAS(P)->dualfunction[i](uout,vout);	
      COLORADDSCALED(RECEIVED_RAD(P)[i], dual, accum_rad, RECEIVED_RAD(P)[i]);

#ifdef HOMCR
      if (!mcr.continuous_random_walk) {
	double basf = BAS(P)->function[i](uin, vin);
	r += basf * dual;
      }
#endif 
    }
    NG(P) ++;

    COLORSCALE((r/node->probability), accum_rad, accum_rad);
    COLORADD(accum_rad, SOURCE_RAD(P), accum_rad);
  }
}

static void GatheringUpdate(PATCH *P, double w)
{
  
  ADDCOEFFICIENTS(UNSHOT_RAD(P), RECEIVED_RAD(P), BAS(P));
  COPYCOEFFICIENTS(RAD(P), UNSHOT_RAD(P), BAS(P));

  
  if (NG(P) > 0) SCALECOEFFICIENTS((1./(double)NG(P)), RAD(P), BAS(P));

  
  COLORADD(RAD(P)[0], SOURCE_RAD(P), RAD(P)[0]);

  if (mcr.constant_control_variate) {
    
    COLOR cr = mcr.control_radiance;
    if (mcr.indirect_only) {
      COLOR Rd = REFLECTANCE(P);
      COLORPROD(Rd, mcr.control_radiance, cr);
    }
    COLORADD(RAD(P)[0], cr, RAD(P)[0]);
  }

  CLEARCOEFFICIENTS(RECEIVED_RAD(P), BAS(P));
}

static void DoGatheringIteration(void)
{
  long nr_walks = mcr.initial_nr_rays;
#ifdef HOMCR
  if (mcr.continuous_random_walk)
    nr_walks *= approxdesc[mcr.approx_type].basis_size;
  else
    nr_walks *= pow(approxdesc[mcr.approx_type].basis_size, 1./(1. - COLORMAXCOMPONENT(average_reflectivity)));
#endif

  if (mcr.constant_control_variate && mcr.iteration_nr == 1) { 
    
    mcr.control_radiance = DetermineGatheringControlRadiosity();
    ReduceSource();	
  }

  fprintf(stderr, "Collision gathering iteration %d (%ld paths, approximately %ld rays)\n",
	  mcr.iteration_nr,
	  nr_walks, (long)floor((double)nr_walks / (1. - COLORMAXCOMPONENT(average_reflectivity))));

  TracePaths(nr_walks,
	     PatchArea, ScalarReflectance,
	     CollisionGatheringScore,
	     GatheringUpdate);
}

static COLOR OneMinusRho(ELEMENT *elem)
{
  COLOR rho = REFLECTANCE(elem->pog.patch);
  COLOR oneminusrho;
  COLORSETMONOCHROME(oneminusrho, 1.);
  COLORSUBTRACT(oneminusrho, rho, oneminusrho);
  return oneminusrho;
}

static COLOR DetermineShootingControlRadiosity(void)
{
  return DetermineControlRadiosity(GetSourceRadiance, OneMinusRho);
}

static void UpdateSourceIllum(ELEMENT *elem, double w)
{
  COPYCOEFFICIENTS(elem->rad, elem->received_rad, elem->basis);
  elem->source_rad = elem->received_rad[0];
  CLEARCOEFFICIENTS(elem->unshot_rad, elem->basis);
  CLEARCOEFFICIENTS(elem->received_rad, elem->basis);
}

static void DoFirstShot(void)
{
  long nr_rays = mcr.initial_nr_rays * approxdesc[mcr.approx_type].basis_size;
  fprintf(stderr, "First shot (%ld rays):\n", nr_rays);
  DoStochasticJacobiIteration(nr_rays, GetSelfEmittedRadiance, NULL, UpdateSourceIllum);
  PrintStats();
}

static int RwrDoStep(void)
{
  McrPreStep();

  if (mcr.iteration_nr == 1) {
    if (mcr.indirect_only)
      DoFirstShot();
  }

  switch (mcr.rw_estimator_type) {
  case RW_SHOOTING: 	DoShootingIteration(); 	break;
  case RW_GATHERING: 	DoGatheringIteration(); break;
  default:
    Fatal(-1, "RwrDoStep", "Unknown random walk estimator type %d", mcr.rw_estimator_type);
  }

  PatchListIterate(Patches, McrPatchComputeNewColor);
  McrPostStep();
  return FALSE; 
}

static void RwrTerminate(void)
{
  
  McrTerminate();
}

static char *RwrGetStats(void)
{
  static char stats[2000];
  char *p;
  int n;

  p = stats;
  sprintf(p, "Random Walk Radiosity\nStatistics\n\n%n", &n); p += n;
  sprintf(p, "Iteration nr: %d\n%n", mcr.iteration_nr, &n); p += n;
  sprintf(p, "CPU time: %g secs\n%n", mcr.cpu_secs, &n); p += n;
  sprintf(p, "Memory usage: %ld KBytes.\n%n", GetMemoryUsage()/1024, &n); p += n;
  sprintf(p, "Radiance rays: %ld\n%n", mcr.traced_rays, &n); p += n;
  sprintf(p, "Importance rays: %ld\n%n", mcr.imp_traced_rays, &n); p += n;

  return stats;
}

RADIANCEMETHOD RandomWalkRadiosity = {
  "RandomWalk", 3,
  "Random Walk Radiosity",
  "randwalkButton",
  McrDefaults,
  RwrParseOptions,
  RwrPrintOptions,
  RwrInit,
  RwrDoStep,
  RwrTerminate,
  McrGetRadiance,
  McrCreatePatchData,
  RwrPrintPatchData,
  McrDestroyPatchData,
  RwrCreateControlPanel,
  RwrUpdateControlPanel,
  RwrShowControlPanel,
  RwrHideControlPanel,
  RwrGetStats,
  (void (*)(void))NULL,		
  McrRecomputeDisplayColors,
  McrUpdateMaterial,
  (void (*)(FILE *))NULL
};
