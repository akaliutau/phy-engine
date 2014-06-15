

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "error.h"
#include "scene.h"
#include "statistics.h"
#include "render.h"
#include "pools.h"

#include "mcrad.h"
#include "mcradP.h"
#include "hierarchy.h"
#include "stochjacobi.h"

static void SrrInit(void)
{
  mcr.method = SRR;
  McrInit();
}

static char *SrrGetStats(void)
{
  static char stats[2000];
  char *p;
  int n;

  p = stats;
  sprintf(p, "Stochastic Relaxation Radiosity\nStatistics\n\n%n", &n); p += n;
  sprintf(p, "Iteration nr: %d\n%n", mcr.iteration_nr, &n); p += n;
  sprintf(p, "CPU time: %g secs\n%n", mcr.cpu_secs, &n); p += n;
  sprintf(p, "Memory usage: %ld KBytes.\n%n", GetMemoryUsage()/1024, &n); p += n;
  sprintf(p, "%ld elements (%ld clusters, %ld surfaces)\n%n",
	  hierarchy.nr_elements, hierarchy.nr_clusters, hierarchy.nr_elements - hierarchy.nr_clusters, &n); p += n;
  sprintf(p, "Radiance rays: %ld\n%n", mcr.traced_rays, &n); p += n;
  sprintf(p, "Importance rays: %ld\n%n", mcr.imp_traced_rays, &n); p += n;

  return stats;
}


static long RandomRound(float x)
{
  long l = (long)floor(x);
  if (drand48() < (x - (float)l)) l++;
  return l;
}

static void SrrRecomputeDisplayColors(void)
{
  if (hierarchy.topcluster) {
    ForAllLeafElements(hierarchy.topcluster, ElementComputeNewVertexColors);
    ForAllLeafElements(hierarchy.topcluster, ElementAdjustTVertexColors);
  } else {
    PatchListIterate(Patches, McrPatchComputeNewColor);
  }
}


static double QualityFactor(ELEMENT *elem, double w)
{
#ifdef IDMCR
  if (mcr.importance_driven) {
    return w * elem->imp;
  }
#endif 
  return w / ElementScalarReflectance(elem);
}

static COLOR *ElementUnshotRadiance(ELEMENT *elem)
{
  return elem->unshot_rad;
}

static void ElementIncrementRadiance(ELEMENT *elem, double w)
{
  
  if (mcr.discard_incremental) {
    elem->quality = 0.0;
    {
      static int wgiv = FALSE;
      if (!wgiv) Warning("ElementIncrementRadiance", "Solution of incremental Jacobi steps receives zero quality");
      wgiv = TRUE;
    }
  } else
    elem->quality = QualityFactor(elem, w);

  ADDCOEFFICIENTS(elem->rad, elem->received_rad, elem->basis);
  COPYCOEFFICIENTS(elem->unshot_rad, elem->received_rad, elem->basis);
  if (mcr.set_source) {
    
    elem->rad[0] = elem->source_rad = elem->received_rad[0];
  }
  CLEARCOEFFICIENTS(elem->received_rad, elem->basis);
}

static void PrintIncrementalRadianceStats(void)
{
  fprintf(stderr, "%g secs., radiance rays = %ld (%ld not to background), unshot flux = ", 
	  mcr.cpu_secs, mcr.traced_rays, mcr.traced_rays-mcr.nrmisses);
  ColorPrint(stderr, mcr.unshot_flux);
  fprintf(stderr, ", total flux = ");
  ColorPrint(stderr, mcr.total_flux);
#ifdef IDMCR
  fprintf(stderr, ", indirect importance weighted unshot flux = ");
  ColorPrint(stderr, mcr.imp_unshot_flux);
#endif
  fprintf(stderr, "\n");
}

static void DoIncrementalRadianceIterations(void)
{
  double ref_unshot;
  long step_nr=0;

  int weighted_sampling = mcr.weighted_sampling;
  int importance_driven = mcr.importance_driven;
  if (!mcr.incremental_uses_importance)
    mcr.importance_driven = FALSE; 
  mcr.weighted_sampling = FALSE;

  PrintIncrementalRadianceStats();
  ref_unshot = COLORSUMABSCOMPONENTS(mcr.unshot_flux);
#ifdef IDMCR
  if (mcr.incremental_uses_importance) {
    ref_unshot = COLORSUMABSCOMPONENTS(mcr.imp_unshot_flux);
  }
#endif
  while (1) {
    
    double unshot_fraction;
    long nr_rays;
    unshot_fraction = COLORSUMABSCOMPONENTS(mcr.unshot_flux) / ref_unshot;
#ifdef IDMCR
    if (mcr.incremental_uses_importance) {
      unshot_fraction = COLORSUMABSCOMPONENTS(mcr.imp_unshot_flux) / ref_unshot;
    }
#endif
    if (unshot_fraction < 0.01) break;	
    nr_rays = RandomRound(unshot_fraction * (double)mcr.initial_nr_rays * approxdesc[mcr.approx_type].basis_size);

    step_nr ++;
    fprintf(stderr, "Incremental radiance propagation step %ld: %.3f%% unshot power left.\n",
	    step_nr, 100.*unshot_fraction);

    DoStochasticJacobiIteration(nr_rays, ElementUnshotRadiance, NULL, ElementIncrementRadiance);
    mcr.set_source = FALSE;	

    McrUpdateCpuSecs();
    PrintIncrementalRadianceStats();
    if (unshot_fraction > 0.3) {
      SrrRecomputeDisplayColors();
      RenderNewDisplayList();
      RenderScene();
    }
  }

  mcr.importance_driven = importance_driven;	
  mcr.weighted_sampling = weighted_sampling;
}

#ifdef IDMCR
static float ElementUnshotImportance(ELEMENT *elem)
{
  return elem->unshot_imp;
}

static void ElementIncrementImportance(ELEMENT *elem, double w)
{
  elem->imp += elem->received_imp;
  elem->unshot_imp = elem->received_imp;
  elem->received_imp = 0.;
}

static void PrintIncrementalImportanceStats(void)
{
  fprintf(stderr, "%g secs., importance rays = %ld, unshot importance = %g, total importance = %g, total area = %g\n",
	  mcr.cpu_secs, mcr.imp_traced_rays, mcr.unshot_ymp, mcr.total_ymp, total_area);
}

static void DoIncrementalImportanceIterations(void)
{
  long step_nr=0;
  int radiance_driven = mcr.radiance_driven;
  int do_h_meshing = hierarchy.do_h_meshing;
  CLUSTERING_MODE clustering = hierarchy.clustering;
  int weighted_sampling = mcr.weighted_sampling;

  if (mcr.source_ymp < EPSILON) {
    fprintf(stderr, "No source importance!!\n");
    return;
  }

  mcr.radiance_driven = FALSE;	
  hierarchy.do_h_meshing = FALSE;
  hierarchy.clustering = NO_CLUSTERING;
  mcr.weighted_sampling = FALSE;

  PrintIncrementalRadianceStats();
  while (1) {
    
    double unshot_fraction = mcr.unshot_ymp / mcr.source_ymp;
    long nr_rays = RandomRound(unshot_fraction * (double)mcr.initial_nr_rays);
    if (unshot_fraction < 0.01) break;

    step_nr ++;
    fprintf(stderr, "Incremental importance propagation step %ld: %.3f%% unshot importance left.\n",
	    step_nr, 100.*unshot_fraction);

    DoStochasticJacobiIteration(nr_rays, NULL, ElementUnshotImportance, ElementIncrementImportance);

    McrUpdateCpuSecs();
    PrintIncrementalImportanceStats();
  }

  mcr.radiance_driven = radiance_driven;	
  hierarchy.do_h_meshing = do_h_meshing;
  hierarchy.clustering = clustering;
  mcr.weighted_sampling = weighted_sampling;
}
#endif 

static COLOR *ElementRadiance(ELEMENT *elem)
{
  return elem->rad;
}

static void ElementUpdateRadiance(ELEMENT *elem, double w)
{
  double k = (double)mcr.prev_traced_rays / (double)(mcr.traced_rays > 0 ? mcr.traced_rays : 1);

  if (!mcr.naive_merging) {
    double quality = QualityFactor(elem, w);
    if (elem->quality < EPSILON) {
      k = 0.;	
    } else if (quality < EPSILON) {
      k = 1.;	
    } else if (elem->quality + quality > EPSILON) {
      k = elem->quality / (elem->quality + quality);
    } else  
      k = 0.;
    elem->quality += quality;	
  }

  
  COLORSUBTRACT(elem->rad[0], elem->source_rad, elem->rad[0]);

  
  SCALECOEFFICIENTS(k, elem->rad, elem->basis);
  SCALECOEFFICIENTS((1.-k), elem->received_rad, elem->basis);
  ADDCOEFFICIENTS(elem->rad, elem->received_rad, elem->basis);

  
  COLORADD(elem->rad[0], elem->source_rad, elem->rad[0]);

  
  CLEARCOEFFICIENTS(elem->unshot_rad, elem->basis);
  CLEARCOEFFICIENTS(elem->received_rad, elem->basis);
}

static void PrintRegularStats(void)
{
  fprintf(stderr, "%g secs., radiance rays = %ld (%ld not to background), unshot flux = ", 
	  mcr.cpu_secs, mcr.traced_rays, mcr.traced_rays-mcr.nrmisses);
  fprintf(stderr, ", total flux = ");
  ColorPrint(stderr, mcr.total_flux);
  if (mcr.importance_driven)
    fprintf(stderr, "\ntotal importance rays = %ld, total importance = %g, total_area = %g", 
	    mcr.imp_traced_rays, mcr.total_ymp, total_area);
  fprintf(stderr, "\n");
}

static void DoRegularRadianceIteration(void)
{
  fprintf(stderr, "Regular radiance iteration %d:\n", mcr.iteration_nr);
  DoStochasticJacobiIteration(mcr.rays_per_iteration, ElementRadiance, NULL, ElementUpdateRadiance);

  McrUpdateCpuSecs();
  PrintRegularStats();
}

#ifdef IDMCR
static float ElementImportance(ELEMENT *elem)
{
  return elem->imp;
}

static void ElementUpdateImportance(ELEMENT *elem, double w)
{
  double k = (double)mcr.prev_imp_traced_rays/(double)mcr.imp_traced_rays;

  elem->imp = k * (elem->imp-elem->source_imp) +
          (1.-k) * elem->received_imp + elem->source_imp;
  elem->unshot_imp = elem->received_imp = 0.;
}

static void DoRegularImportanceIteration(void)
{
  long nr_rays;
  int do_h_meshing = hierarchy.do_h_meshing;
  CLUSTERING_MODE clustering = hierarchy.clustering;
  int weighted_sampling = mcr.weighted_sampling;
  hierarchy.do_h_meshing = FALSE;
  hierarchy.clustering = NO_CLUSTERING;
  mcr.weighted_sampling = FALSE;

  nr_rays = mcr.imp_rays_per_iteration;
  fprintf(stderr, "Regular importance iteration %d:\n", mcr.iteration_nr);
  DoStochasticJacobiIteration(nr_rays, NULL, ElementImportance, ElementUpdateImportance);

  McrUpdateCpuSecs();
  PrintRegularStats();

  hierarchy.do_h_meshing = do_h_meshing;
  hierarchy.clustering = clustering;
  mcr.weighted_sampling = weighted_sampling;
}
#endif


static void ElementDiscardIncremental(ELEMENT *elem)
{
  elem->quality = 0.;

  
  ForAllChildrenElements(elem, ElementDiscardIncremental);
}

static void DiscardIncremental(void)
{
  mcr.nr_weighted_rays = mcr.old_nr_weighted_rays = 0;
  mcr.traced_rays = mcr.prev_traced_rays = 0;

  ElementDiscardIncremental(hierarchy.topcluster);
}

static int SrrDoStep(void)
{
  McrPreStep();

  
  if (mcr.iteration_nr == 1) {
    int initial_nr_of_rays = 0;
    if (mcr.do_nondiffuse_first_shot) 
      DoNonDiffuseFirstShot();
    initial_nr_of_rays = mcr.traced_rays;

#ifdef IDMCR
    if (mcr.importance_driven) {
      if (!mcr.incremental_uses_importance) {
	Warning(NULL, "Importance is only used from the second iteration on ...");
      } else if (mcr.importance_updated) {
	mcr.importance_updated = FALSE;

	
	DoIncrementalImportanceIterations();
	if (mcr.importance_updated_from_scratch)
	  mcr.imp_rays_per_iteration = mcr.imp_traced_rays;
      }
    }
#endif
    DoIncrementalRadianceIterations();

    
    mcr.rays_per_iteration = mcr.traced_rays - initial_nr_of_rays;

    if (mcr.discard_incremental) DiscardIncremental();
  } else {
#ifdef IDMCR
    if (mcr.importance_driven) {
      if (mcr.importance_updated) {
	mcr.importance_updated = FALSE;

	
	DoIncrementalImportanceIterations();
	if (mcr.importance_updated_from_scratch)
	  mcr.imp_rays_per_iteration = mcr.imp_traced_rays;
      } else {
	DoRegularImportanceIteration();
      }
    }
#endif
    DoRegularRadianceIteration();
  }

  SrrRecomputeDisplayColors();
  McrPostStep();

  fprintf(stderr, "%s\n", SrrGetStats());

  return FALSE; 
}

static void McrRenderPatch(PATCH *patch)
{
  extern void RenderPatch(PATCH *patch);  
  if (mcr.inited)
    McrForAllSurfaceLeafs(TOPLEVEL_ELEMENT(patch), RenderElement);
  else
    RenderPatch(patch);	
}

static void SrrRender(void)
{
  if (renderopts.frustum_culling)
    RenderWorldOctree(McrRenderPatch);
  else
    PatchListIterate(Patches, McrRenderPatch);
}

RADIANCEMETHOD StochasticRelaxationRadiosity = {
  "StochJacobi", 3,
  "Stochastic Jacobi Radiosity",
  "stochrelaxButton",
  McrDefaults,
  SrrParseOptions,
  SrrPrintOptions,
  SrrInit,
  SrrDoStep,
  McrTerminate,
  McrGetRadiance,
  McrCreatePatchData,
  McrPrintPatchData,
  McrDestroyPatchData,
  SrrCreateControlPanel,
  SrrUpdateControlPanel,
  SrrShowControlPanel,
  SrrHideControlPanel,
  SrrGetStats,
  SrrRender,
  SrrRecomputeDisplayColors,
  McrUpdateMaterial
};


