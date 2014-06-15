

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "pmap.h"
#include "pmapP.h"
#include "pmapoptions.H"
#include "pmapconfig.H"
#include "pmapimportance.H"

#include "pools.h"
#include "scene.h"	
#include "error.h"
#include "options.h"	

#include "vertex.h"
#include "render.h"
#include "ui.h"

#include "raytracing.h"
#include "raycasting.h"

#include "patch.h"
#include "pathnode.H"
#include "bipath.H"
#include "eyesampler.H"
#include "lightsampler.H"
#include "lightdirsampler.H"
#include "bsdfsampler.H"
#include "photonmapsampler.H"
#include "samplertools.H"
#include "raytools.H"
#include "lightlist.H"
#include "screensampler.H"
#include "pixelsampler.H"

#include "screenbuffer.H"

#include "color.h"
#include "rgb.h"

#include "photonmap.H"
#include "importancemap.H"

#include "mcsampling/nied31.h"

#define PMAP_MISFUNC(a) (a)

PMAPCONFIG pmapconfig;



static void PmapDefaults(void)
{
  pmapstate.Defaults();
}


static void UpdateCpuSecs(void)
{
  clock_t t;

  t = clock();
  pmapstate.cpu_secs += (float)(t - pmapstate.lastclock)/(float)CLOCKS_PER_SEC;
  pmapstate.lastclock = t;
}

static void wake_up(int)
{
  pmapstate.wake_up = TRUE;

  signal(SIGALRM, wake_up);
  alarm( 1 );

  UpdateCpuSecs();
}

static void *CreatePatchData(PATCH *patch)
{
  return patch->radiance_data = NULL;
}

static void PrintPatchData(FILE *out, PATCH *patch)
{
  fprintf(out, "No data\n");
}

static void DestroyPatchData(PATCH *patch)
{
  patch->radiance_data = (void *)NULL;
}


static void InitPatch(PATCH *P)
{
  // Nothing to do yet. No patch data in photonmaps
}



static void PatchComputeNewColor(PATCH *patch)
{
  COLOR Rd = PatchAverageNormalAlbedo(patch, BRDF_DIFFUSE_COMPONENT);
  ColorToRGB(Rd, &patch->color);
  PatchComputeVertexColors(patch);
}

static void PatchUpdateColor(PATCH *P)
{
  PatchComputeNewColor(P);
}

static void PatchDefaultColor(PATCH *patch)
{
  COLOR Rd = PatchAverageNormalAlbedo(patch, BRDF_DIFFUSE_COMPONENT);
  ColorToRGB(Rd, &patch->color);
  PatchComputeVertexColors(patch);  
}

static void ChooseSurfaceSampler(CSurfaceSampler **samplerPtr)
{
  if(*samplerPtr)
    delete *samplerPtr;
  
  if(pmapstate.usePhotonMapSampler)
  {
    *samplerPtr = new CPhotonMapSampler;
  }
  else
  {
    *samplerPtr = new CBsdfSampler;    
  }
}

void PmapChooseSurfaceSampler(void)
{
  ChooseSurfaceSampler(&(pmapconfig.eyeConfig.surfaceSampler));  
  ChooseSurfaceSampler(&(pmapconfig.lightConfig.surfaceSampler));  
}



static void InitPmap(void)
{
  fprintf(stderr, "Photonmap activated\n");

  

  pmapstate.lastclock = clock();
  pmapstate.cpu_secs = 0.;
  pmapstate.g_iteration_nr = 0;
  pmapstate.c_iteration_nr = 0;
  pmapstate.i_iteration_nr = 0;
  pmapstate.iteration_nr = 0;
  pmapstate.runstop_nr = 0;
  pmapstate.total_gpaths = 0;
  pmapstate.total_cpaths = 0;
  pmapstate.total_ipaths = 0;
  pmapstate.total_rays = 0;

  if(pmapconfig.screen) delete pmapconfig.screen;
  pmapconfig.screen = new CScreenBuffer;

  // Init samplers

  pmapconfig.lightConfig.ReleaseVars();
  pmapconfig.eyeConfig.ReleaseVars();

  CSamplerConfig *cfg = &pmapconfig.eyeConfig;

  cfg->pointSampler = new CEyeSampler;

  cfg->dirSampler = new CScreenSampler;  // ps;

  ChooseSurfaceSampler(&cfg->surfaceSampler);
  // cfg->surfaceSampler = new CPhotonMapSampler;
  // cfg->surfaceSampler = new CBsdfSampler;
  cfg->surfaceSampler->SetComputeFromNextPdf(false);
  cfg->neSampler = NULL;

  cfg->minDepth = 1;
  cfg->maxDepth = 1;  // Only eye point needed, for Particle tracing test

  cfg = &pmapconfig.lightConfig;

  cfg->pointSampler = new CUniformLightSampler;
  cfg->dirSampler = new CLightDirSampler;
  ChooseSurfaceSampler(&cfg->surfaceSampler);
  // cfg->surfaceSampler = new CPhotonMapSampler; //new CBsdfSampler; 
  cfg->surfaceSampler->SetComputeFromNextPdf(false);  // Only 1 pdf

  cfg->minDepth = pmapstate.minimumLightPathDepth;
  cfg->maxDepth = pmapstate.maximumLightPathDepth;
  
  rt_raycount = 0;

  // Init the photonmap

  if(pmapconfig.globalMap) { delete pmapconfig.globalMap; }
  pmapconfig.globalMap = new CPhotonMap(&pmapstate.reconGPhotons, 
					pmapstate.precomputeGIrradiance);

  if(pmapconfig.importanceMap) { delete pmapconfig.importanceMap; }
  pmapconfig.importanceMap = new CImportanceMap(&pmapstate.reconIPhotons,
						&pmapstate.gImpScale);

  if(pmapconfig.importanceCMap) { delete pmapconfig.importanceCMap; }
  pmapconfig.importanceCMap = new CImportanceMap(&pmapstate.reconIPhotons,
						 &pmapstate.cImpScale);

  if(pmapconfig.causticMap) { delete pmapconfig.causticMap; }
  pmapconfig.causticMap = new CPhotonMap(&pmapstate.reconCPhotons);
}


static void PatchReduceFlux(PATCH *P)
{
}





static COLOR DoComputePixelFluxEstimate(PMAPCONFIG *config)
{
  CBiPath *bp = &config->bipath;
  CPathNode *eyePrevNode, *lightPrevNode;
  COLOR oldBsdfL, oldBsdfE;
  CBsdfComp oldBsdfCompL, oldBsdfCompE;
  double oldPdfL, oldPdfE, oldRRPdfL, oldRRPdfE;
  double oldPdfLP=0., oldPdfEP=0., oldRRPdfLP=0., oldRRPdfEP=0.;
  COLOR f;
  CPathNode *eyeEndNode, *lightEndNode;

  // Store PDF and BSDF evals that will be overwritten
  
  eyeEndNode = bp->m_eyeEndNode;
  lightEndNode = bp->m_lightEndNode;
  eyePrevNode = eyeEndNode->Previous();
  lightPrevNode = lightEndNode->Previous();

  oldBsdfL = lightEndNode->m_bsdfEval;
  oldBsdfCompL = lightEndNode->m_bsdfComp;

  oldBsdfE = eyeEndNode->m_bsdfEval;
  oldBsdfCompE = eyeEndNode->m_bsdfComp;
  
  oldPdfL = lightEndNode->m_pdfFromNext;
  PNAN(lightEndNode->m_pdfFromNext);

  oldRRPdfL = lightEndNode->m_rrPdfFromNext;
  
  if(lightPrevNode)
  {
    oldPdfLP = lightPrevNode->m_pdfFromNext;
    PNAN(lightPrevNode->m_pdfFromNext);
    oldRRPdfLP = lightPrevNode->m_rrPdfFromNext;
  }

  oldPdfE = eyeEndNode->m_pdfFromNext;
  PNAN(eyeEndNode->m_pdfFromNext);
  oldRRPdfE = eyeEndNode->m_rrPdfFromNext;

  if(eyePrevNode)
  {
    oldPdfEP = eyePrevNode->m_pdfFromNext;
    PNAN(eyePrevNode->m_pdfFromNext);
    oldRRPdfEP = eyePrevNode->m_rrPdfFromNext;
  }

  // Connect the subpaths
  
  bp->m_geomConnect = 
    PathNodeConnect(eyeEndNode, lightEndNode, 
		    &config->eyeConfig, &config->lightConfig,
		    CONNECT_EL | CONNECT_LE,
		    BSDF_ALL_COMPONENTS, BSDF_ALL_COMPONENTS, &bp->m_dirEL);
  
  VECTORSCALE(-1, bp->m_dirEL, bp->m_dirLE);

  //PNAN(eyeEndNode->m_pdfFromNext);
  //if(eyePrevNode) PNAN(eyePrevNode->m_pdfFromNext);
  //PNAN(lightEndNode->m_pdfFromNext);
  //if(lightPrevNode) PNAN(lightPrevNode->m_pdfFromNext);

  // Evaluate radiance and pdf and weight

  f = bp->EvalRadiance();

  double factor = 1.0 / bp->EvalPDFAcc();

  COLORSCALE(factor, f, f); // Flux estimate

  // Restore old values
  
  lightEndNode->m_bsdfEval = oldBsdfL;
  lightEndNode->m_bsdfComp = oldBsdfCompL;

  eyeEndNode->m_bsdfEval = oldBsdfE;
  eyeEndNode->m_bsdfComp = oldBsdfCompE;
  
  lightEndNode->m_pdfFromNext = oldPdfL;
  lightEndNode->m_rrPdfFromNext = oldRRPdfL;
  
  if(lightPrevNode)
  {
    lightPrevNode->m_pdfFromNext = oldPdfLP;
    lightPrevNode->m_rrPdfFromNext = oldRRPdfLP;
  }
  
  eyeEndNode->m_pdfFromNext = oldPdfE;
  eyeEndNode->m_rrPdfFromNext = oldRRPdfE;

  if(eyePrevNode)
  {
    eyePrevNode->m_pdfFromNext = oldPdfEP;
    eyePrevNode->m_rrPdfFromNext = oldRRPdfEP;
  }

  return f;
}








void DoScreenNEE(PMAPCONFIG *config)
{
  int nx,ny;
  float pix_x, pix_y;
  COLOR f;
  CBiPath *bp = &config->bipath;

  if(config->currentMap == config->importanceMap)
  {
    return;
  }

  //if(bp->m_lightSize <= 2)
  //  return;

  // First we need to determine if the lightEndNode can be seen from
  // the camera. At the same time the pixel hit is computed

  if(EyeNodeVisible(bp->m_eyeEndNode, bp->m_lightEndNode, 
		    &pix_x, &pix_y))
  {
    // Visible !

    f = DoComputePixelFluxEstimate(config);

    config->screen->GetPixel(pix_x, pix_y, &nx, &ny);

    float factor;

    if(config->currentMap == config->globalMap)
    {
      factor = (ComputeFluxToRadFactor(nx, ny) 
		/ (float)pmapstate.total_gpaths);
    }
    else
    {
      factor = (ComputeFluxToRadFactor(nx, ny) 
		/ (float)pmapstate.total_cpaths);
    }

    COLORSCALE(factor, f, f);

    config->screen->Add(nx, ny, f);
  }
}





bool DoPhotonStore(CPathNode *node, COLOR power)
{
  //float scatteredPower;
  //COLOR col;
  BSDF *bsdf;

  if(node->m_hit.patch && node->m_hit.patch->surface->material)
  {
    // Only add photons on surfaces with a certain reflection
    // coefficient.

    bsdf = node->m_hit.patch->surface->material->bsdf;

    if(!ZeroAlbedo(bsdf, &node->m_hit, BSDF_DIFFUSE_COMPONENT|BSDF_GLOSSY_COMPONENT))
    {
      CPhoton photon(node->m_hit.point, power, node->m_inDirF);
      
      // Determine photon flags
      short flags = 0;

      if(node->m_depth == 1)
      {
	// Direct light photon
	flags |= DIRECT_LIGHT_PHOTON;
      }

      if(pmapstate.densityControl == NO_DENSITY_CONTROL)
      {
	return pmapconfig.currentMap->AddPhoton(photon, node->m_hit.normal,
					    flags);
      }
      else
      {
	float reqDensity;
	if(pmapstate.densityControl == CONSTANT_RD)
	{
	  reqDensity = pmapstate.constantRD;
	}
	else
	{
	  reqDensity = pmapconfig.currentImpMap->GetRequiredDensity(node->m_hit.point, 
								    node->m_hit.normal);
	}

	return pmapconfig.currentMap->DC_AddPhoton(photon, node->m_hit,
					       reqDensity, flags);
      } 
    }
  }
  return false;
}



void HandlePath(PMAPCONFIG *config)
{
  bool ldone;
  CBiPath *bp = &config->bipath;
  COLOR accPower;
  float factor;


  // Iterate over all light nodes

  bp->m_lightSize = 1;
  CPathNode *currentNode = bp->m_lightPath;

  bp->m_eyeSize = 1;
  bp->m_eyeEndNode = bp->m_eyePath;
  bp->m_geomConnect = 1.0; // No connection yet 

  ldone = false;
  COLORSETMONOCHROME(accPower, 1.0);

  while(!ldone)
  {
    // Adjust accPower

    factor = currentNode->m_G / currentNode->m_pdfFromPrev;
    COLORSCALE(factor, accPower, accPower);

    // Store photon, but not emitted light

    if(config->currentMap == config->globalMap)
    {
      if(bp->m_lightSize > 1)
      {
	// Store
	
	if(DoPhotonStore(currentNode, accPower))
	{
	  // Screen next event estimation for testing
	  
	  bp->m_lightEndNode = currentNode;
	  DoScreenNEE(config);
	}
      }
    }
    else  // Caustic map...
    {
      if(bp->m_lightSize > 2)
      {
	// Store
	
	if(DoPhotonStore(currentNode, accPower))
	{
	  // Screen next event estimation for testing
	  
	  bp->m_lightEndNode = currentNode;
	  DoScreenNEE(config);
	}
      }
    }


    // Account for bsdf, node that for the first node, this accounts
    // for the emitted radiance.

    if(!(currentNode->Ends()))
    {
      COLORPROD(currentNode->m_bsdfEval, accPower, accPower);

      currentNode = currentNode->Next();
      bp->m_lightSize++;
    }
    else
    {
      ldone = true;
    }
  }
}




void TestPMAP(void)
{
  // COLOR col;

  // Trace a test eye path

  pmapconfig.eyeConfig.maxDepth = 2;

  pmapconfig.bipath.m_eyePath = pmapconfig.eyeConfig.TracePath(pmapconfig.bipath.m_eyePath);

  // Reconstruct at the nodes

  CPathNode *node = pmapconfig.bipath.m_eyePath;
  bool done = (node->Ends());

  while(!done)
  {
    node = node->Next();
    pmapconfig.currentMap->Reconstruct(&node->m_hit, node->m_inDirF, 
				   node->m_useBsdf,
				   node->m_inBsdf, node->m_outBsdf);

    done = (node->Ends());
  }

  // Restore sampler settings

  pmapconfig.eyeConfig.maxDepth = 1;
}





static unsigned pmapQMCseed_s = 123456;

static void TracePath(PMAPCONFIG *config, BSDFFLAGS bsdfFlags)
{
  config->bipath.m_eyePath = config->eyeConfig.TracePath(config->bipath.m_eyePath);

  // Use qmc for light sampling

  double x_1, x_2;
  // unsigned *nrs = Nied31(pmapQMCseed_s++);

  CPathNode *path = config->bipath.m_lightPath;

  // First node
  x_1 = drand48(); //nrs[0] * RECIP;
  x_2 = drand48(); //nrs[1] * RECIP;

  path = config->lightConfig.TraceNode(path, x_1, x_2, bsdfFlags);
  if(path == NULL) return;

  config->bipath.m_lightPath = path;  // In case no nodes were present

  path->EnsureNext();

  // Second node
  CPathNode *node = path->Next();
  x_1 = drand48(); // nrs[2] * RECIP;
  x_2 = drand48(); // nrs[3] * RECIP; // 4D Niederreiter...

  if(config->lightConfig.TraceNode(node, x_1, x_2, bsdfFlags))
  {
    // Succesful trace...
    node->EnsureNext();
    config->lightConfig.TracePath(node->Next(), bsdfFlags);
  }
}

static void TracePaths(int nrPaths, BSDFFLAGS bsdfFlags = BSDF_ALL_COMPONENTS)
{
  int i;

  // Fill in config structures

  for(i = 0; i < nrPaths; i++)
  {
    TracePath(&pmapconfig, bsdfFlags);
    // config.currentMap->AddPath();

    HandlePath(&pmapconfig);

    if (pmapstate.wake_up) {
      ProcessWaitingEvents();
      pmapstate.wake_up = FALSE;
    }
  }
}

static void BRRealIteration(void)
{
  pmapstate.iteration_nr++;

  fprintf(stderr, "Pmap Iteration %li\n", (long)pmapstate.iteration_nr);

  if((pmapstate.iteration_nr > 1) && (pmapstate.doGlobalMap || pmapstate.doCausticMap))
  {
    float scaleFactor = (pmapstate.iteration_nr-1.0)/(float)pmapstate.iteration_nr; 
    pmapconfig.screen->ScaleRadiance(scaleFactor);
  }

  if((pmapstate.densityControl == IMPORTANCE_RD) && pmapstate.doImportanceMap)
  {
    pmapstate.i_iteration_nr++;
    pmapconfig.currentMap = pmapconfig.importanceMap;
    pmapstate.total_ipaths = pmapstate.i_iteration_nr * pmapstate.ipaths_per_iteration;
    pmapconfig.currentMap->SetTotalPaths(pmapstate.total_ipaths);
    pmapconfig.importanceCMap->SetTotalPaths(pmapstate.total_ipaths);
    
    TracePotentialPaths(pmapstate.ipaths_per_iteration);
    
    fprintf(stderr,"Total potential paths : %li, Total rays %li\n", 
	    pmapstate.total_ipaths,
	    rt_raycount);
    //pmapconfig.importanceMap->BalanceAnalysis();
  }


  // Global map

  if(pmapstate.doGlobalMap)
  {
    pmapstate.g_iteration_nr++;
    pmapconfig.currentMap = pmapconfig.globalMap;
    pmapstate.total_gpaths = pmapstate.g_iteration_nr * pmapstate.gpaths_per_iteration;
    pmapconfig.currentMap->SetTotalPaths(pmapstate.total_gpaths);

    // Set correct importance map: indirect importance
    pmapconfig.currentImpMap = pmapconfig.importanceMap;
    
    TracePaths(pmapstate.gpaths_per_iteration);
    
    fprintf(stderr, "Global map: ");
    pmapconfig.globalMap->PrintStats(stderr);

    
  }

  // Caustic map

  if(pmapstate.doCausticMap)
  {
    pmapstate.c_iteration_nr++;
    pmapconfig.currentMap = pmapconfig.causticMap;
    pmapstate.total_cpaths = pmapstate.c_iteration_nr * pmapstate.cpaths_per_iteration;
    pmapconfig.currentMap->SetTotalPaths(pmapstate.total_cpaths);

    // Set correct importance map: direct importance
    pmapconfig.currentImpMap = pmapconfig.importanceCMap;
    
    TracePaths(pmapstate.cpaths_per_iteration, BSDF_SPECULAR_COMPONENT);

    fprintf(stderr, "Caustic map: ");
    pmapconfig.causticMap->PrintStats(stderr);

    
  }

}




static int DoPmapStep(void)
{
  void (*prev_alrm_handler)(int signr); 
  unsigned prev_alarm_left;

  
  prev_alrm_handler = signal(SIGALRM, wake_up);
  prev_alarm_left = alarm( 1 );
  pmapstate.wake_up = FALSE;
  pmapstate.lastclock = clock();

  

  BRRealIteration();

  UpdateCpuSecs();

  
  signal(SIGALRM, prev_alrm_handler);
  alarm(prev_alarm_left);

  pmapstate.runstop_nr++;
  
  return FALSE;	
}


static void TerminatePmap(void)
{
  if(pmapconfig.screen) 
  {
    delete pmapconfig.screen;
    pmapconfig.screen = NULL;
  }

  pmapconfig.lightConfig.ReleaseVars();
  pmapconfig.eyeConfig.ReleaseVars();

  if(pmapconfig.globalMap) 
  { 
    delete pmapconfig.globalMap;
    pmapconfig.globalMap = NULL;
  }

  if(pmapconfig.importanceMap) 
  { 
    delete pmapconfig.importanceMap; 
    pmapconfig.importanceMap = NULL;
  }

  if(pmapconfig.importanceCMap) 
  { 
    delete pmapconfig.importanceCMap; 
    pmapconfig.importanceCMap = NULL;
  }

  if(pmapconfig.causticMap) 
  { 
    delete pmapconfig.causticMap;
    pmapconfig.causticMap = NULL;
  }
}


// Returns the radiance emitted in the node related direction
COLOR GetPmapNodeGRadiance(CPathNode *node)
{
  COLOR col;

  pmapconfig.globalMap->DoBalancing(pmapstate.balanceKDTree);
  col = pmapconfig.globalMap->Reconstruct(&node->m_hit, node->m_inDirF, 
				      node->m_useBsdf,
				      node->m_inBsdf, node->m_outBsdf);
  return col;
}


// Returns the radiance emitted in the node related direction
COLOR GetPmapNodeImportance(CPathNode *node)
{
  COLOR col;

  // pmapconfig.importanceMap->DoBalancing(pmapstate.balanceKDTree);

  col = pmapconfig.importanceMap->Reconstruct(&node->m_hit, node->m_inDirF, 
				      node->m_useBsdf,
				      node->m_inBsdf, node->m_outBsdf);
  return col;
}


// Returns the radiance emitted in the node related direction
COLOR GetPmapNodeCRadiance(CPathNode *node)
{
  COLOR col;

  pmapconfig.causticMap->DoBalancing(pmapstate.balanceKDTree);

  col = pmapconfig.causticMap->Reconstruct(&node->m_hit, node->m_inDirF, 
				      node->m_useBsdf,
				      node->m_inBsdf, node->m_outBsdf);
  return col;
}

CPhotonMap *GetPmapGlobalMap(void)
{
  return pmapconfig.globalMap;
}


// To adjust GetPmapRadiance returns
static bool s_doingLocalRaycast = false;

static COLOR GetPmapRadiance(PATCH *patch, 
			      double u, double v, 
			      VECTOR dir)
{
  HITREC hit;
  VECTOR point;
  BSDF *bsdf = patch->surface->material->bsdf;
  COLOR col;
  float density;

  PatchPoint(patch, u, v, &point);
  InitHit(&hit, patch, NULL, &point, &patch->normal, patch->surface->material, 0.);
  HitShadingNormal(&hit, &hit.normal);

  if(ZeroAlbedo(bsdf, &hit, BSDF_DIFFUSE_COMPONENT | BSDF_GLOSSY_COMPONENT))
  {
    COLORCLEAR(col);
    return col;
  }

  RADRETURN_OPTION radreturn = GLOBAL_RADIANCE;

  if(s_doingLocalRaycast)
  {
    radreturn = pmapstate.radianceReturn;
  }

  switch(radreturn)
  {
  case GLOBAL_DENSITY:
    col = pmapconfig.globalMap->GetDensityColor(hit);
    break;
  case CAUSTIC_DENSITY:
    col = pmapconfig.causticMap->GetDensityColor(hit);
    break;
  case IMPORTANCE_CDENSITY:
    col = pmapconfig.importanceCMap->GetDensityColor(hit);
    break;
  case IMPORTANCE_GDENSITY:
    col = pmapconfig.importanceMap->GetDensityColor(hit);
    break;
  case REC_CDENSITY:
    pmapconfig.importanceCMap->DoBalancing(pmapstate.balanceKDTree);
    density = pmapconfig.importanceCMap->GetRequiredDensity(hit.point, hit.normal);
    col = GetFalseColor(density);
    break;
  case REC_GDENSITY:
    pmapconfig.importanceMap->DoBalancing(pmapstate.balanceKDTree);
    density = pmapconfig.importanceMap->GetRequiredDensity(hit.point, hit.normal);
    col = GetFalseColor(density);
    break;
  case GLOBAL_RADIANCE:
    col = pmapconfig.globalMap->Reconstruct(&hit, dir, 
					bsdf,
					NULL, bsdf);
    break;
  case CAUSTIC_RADIANCE:
    col = pmapconfig.causticMap->Reconstruct(&hit, dir, 
					 bsdf,
					 NULL, bsdf);
    break;
  default:
    COLORCLEAR(col);
    Error("GetPmapRadiance", "Unknown radiance return");
  }

  return col;
}


void PmapBalance(PMAP_TYPE type)
{
  CPhotonMap *map;
  switch(type)
  {
  case GLOBAL_MAP:
    map = pmapconfig.globalMap;
    break;
  case CAUSTIC_MAP:
    map = pmapconfig.causticMap;
    break;
  case IMPORTANCE_MAP:
    map = pmapconfig.importanceMap;
    break;
  default:
    return;
  }

  if(map)
    map->Balance();
}

void PmapRaycast(void)
{
  if(pmapstate.rcScreen) delete pmapstate.rcScreen;
  pmapstate.rcScreen = new CScreenBuffer();

  if((pmapstate.radianceReturn == GLOBAL_DENSITY) ||
     (pmapstate.radianceReturn == CAUSTIC_DENSITY) ||
     (pmapstate.radianceReturn == IMPORTANCE_GDENSITY) ||
     (pmapstate.radianceReturn == IMPORTANCE_CDENSITY) ||
     (pmapstate.radianceReturn == REC_CDENSITY) ||
     (pmapstate.radianceReturn == REC_GDENSITY)
    )
  {
    pmapstate.rcScreen->SetRGBImage(true);
  }

  // We need to know if we're raycasting ourselves, so that we can adjust the radiance
  // return
  s_doingLocalRaycast = true;
  RayCast(GetPmapRadiance, pmapstate.rcScreen);
  s_doingLocalRaycast = false;
}

void PmapRaycastInterrupt(void)
{
  if(s_doingLocalRaycast)
    RayCasting.InterruptRayTracing();
}

void PmapRedisplayRaycast(void)
{
  if(pmapstate.rcScreen)
    pmapstate.rcScreen->Render();
}

void PmapDoPrintValue(PATCH *P, VECTOR *hitp)
{
  double u,v;
  PatchUV(P, hitp, &u, &v);
  COLOR col = GetPmapRadiance(P, u, v, P->normal);

  fprintf(stderr, "Value: ");
  ColorPrint(stderr, col);
  fprintf(stderr, "\n");

  VECTOR d = *hitp - Camera.eyep;
  float dist = VECTORNORM(d);
  d = d / dist;
  fprintf(stderr, "Distance : %g\n", dist);
  fprintf(stderr, "Cos: %g\n", (d & P->normal));
}

static void PmapRenderScreen(void)
{
  if(pmapconfig.screen && pmapstate.renderImage)
    pmapconfig.screen->Render();
  else
  {
    PatchListIterate(Patches, RenderPatch);
  }
}

static char *GetPmapStats(void)
{
  static char stats[1000];
  char *p;
  int n;

  p = stats;
  sprintf(p, "PMAP Statistics:\n\n%n", &n); p += n;
  sprintf(p, "Ray count %li\n%n", rt_raycount, &n); p += n;
  sprintf(p, "Time %g\n%n",pmapstate.cpu_secs, &n); p += n;

  if(pmapconfig.globalMap)
  {
    sprintf(p, "Global Map: %n", &n); p += n;
    pmapconfig.globalMap->GetStats(p);
    p += strlen(p);
    sprintf(p, "\n%n", &n); p += n;
  }
  if(pmapconfig.causticMap)
  {
    sprintf(p, "Caustic Map: %n", &n); p += n;
    pmapconfig.causticMap->GetStats(p);
    p += strlen(p);
    sprintf(p, "\n%n", &n); p += n;
  }
  if(pmapconfig.importanceMap)
  {
    sprintf(p, "Global Importance Map: %n", &n); p += n;
    pmapconfig.importanceMap->GetStats(p);
    p += strlen(p);
    sprintf(p, "\n%n", &n); p += n;
  }
  if(pmapconfig.importanceCMap)
  {
    sprintf(p, "Caustic Importance Map: %n", &n); p += n;
    pmapconfig.importanceCMap->GetStats(p);
    p += strlen(p);
    sprintf(p, "\n%n", &n); p += n;
  }

  return stats;
}

static void PmapRecomputeDisplayColors(void)
{
  ForAllPatches(P, Patches) {
    PatchComputeNewColor(P);
  } EndForAll;
}

static void PmapUpdateMaterial(MATERIAL *, 
				MATERIAL *)
{
  Error("PmapUpdateMaterial", "Not yet implemented");
}

RADIANCEMETHOD Pmap = {
  "PMAP", 4,
  "PhotonMap",
  "pmapButton",
  PmapDefaults,
  ParsePmapOptions,
  PrintPmapOptions,
  InitPmap,
  DoPmapStep,
  TerminatePmap,
  GetPmapRadiance,
  CreatePatchData,
  PrintPatchData,
  DestroyPatchData,
  CreatePmapControlPanel,
  (void (*)(void *))NULL,
  ShowPmapControlPanel,
  HidePmapControlPanel,
  GetPmapStats,
  PmapRenderScreen,		
  PmapRecomputeDisplayColors,
  PmapUpdateMaterial,
  (void (*)(FILE *))NULL	
};
