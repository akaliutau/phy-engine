

#include <math.h>
#include <stdlib.h>

#include "raytracing.h"
#include "scene.h"
#include "camera.h"
#include "render.h"
#include "ray.h"
#include "ui.h"
#include "error.h"
#include "pools.h"
#include "options.h"

#include "patch.h"
#include "patchlist.h"
#include "radiance.h"
#include "Float.h"
#include "bsdf.h"
#include "grid.h"

#include "bidirpath.h"
#include "bidirpath_priv.h"

#include "eyesampler.H"
#include "pixelsampler.H"
#include "lightsampler.H"
#include "lightdirsampler.H"
#include "bsdfsampler.H"
#include "samplertools.H"
#include "raytools.H"
#include "screeniterate.H"
#include "screenbuffer.H"

#include "stratification.H"
#include "lightlist.H"
#include "bipath.H"



#include "spar.H"

#include "densitybuffer.H"
#include "densitykernel.H"




BIDIRPATH_STATE bidir;



typedef struct BPCONFIG
{
  BP_BASECONFIG *bcfg;

  // Configuration for tracing the paths
  CSamplerConfig eyeConfig;
  CSamplerConfig lightConfig;

  // Internal vars
  CScreenBuffer *screen;
  double fluxToRadFactor;
  int nx,ny;
  double pdfLNE; // pdf for sampling light point separately

  CDensityBuffer *dBuffer;
  CDensityBuffer *dBuffer2;
  float xsample, ysample;

  CPathNode *eyePath;
  CPathNode *lightPath;

  //  int maxCombinedLength;

  // SPaR configuration

  CSparConfig sparConfig;
  CSparList *sparList;

  bool deStoreHits;

  CScreenBuffer *ref;
  CScreenBuffer *dest;
  CScreenBuffer *ref2;
  CScreenBuffer *dest2;
  CKernel2D kernel;
  int scaleSamples;
} BPCONFIG;


static bool SpikeCheck(COLOR col)
{
//  return false;

  double colAvg = COLORAVERAGE(col);

  if(ISNAN(colAvg))
  {
    printf("COL ");
    ColorPrint(stdout, col);
    printf("\n");
    return true;
  }

  if(colAvg > 60000 )
  {
    printf("Spike\n");
    return true;
  }

  if(colAvg < 0)
  {
    printf("Negative");
    return true;
  }

  return false;
}

static void AddWithSpikeCheck(BPCONFIG *config, CBiPath *path, int nx, int ny, 
			      float pix_x, float pix_y, COLOR f,
			      double pdf, double weight, bool radSample=false)
{
  if(config->bcfg->doDensityEstimation)
  {
    CScreenBuffer *rs, *ds;
    CDensityBuffer *db;
    float baseSize;

    if(radSample)
    {
      rs = config->ref2;
      ds = config->dest2;
      db = config->dBuffer2;
      baseSize = 1.5;
    }
    else
    {
      rs = config->ref;
      ds = config->dest;
      db = config->dBuffer;
      baseSize = 5;
    }

    if(config->deStoreHits && db)
    {
      db->Add(pix_x, pix_y, f, pdf, weight);
    }
    else
    {
      // Now splat directly into the dest screen

      VEC2D center;
      COLOR g;

      // Get the center:

      center.u = pix_x;
      center.v = pix_y;

      // Convert f into DE quantities

      float factor = rs->GetPixXSize() * rs->GetPixYSize()
	* (float)config->bcfg->totalSamples;

      if(COLORAVERAGE(f) > EPSILON)
      {
	COLORSCALE(factor, f, g); // Undo part of flux to rad factor

	config->kernel.VarCover(center, g, rs, ds, 
				config->bcfg->totalSamples, config->scaleSamples,
				baseSize);
      }
      return;
    }
  }

  if(config->bcfg->eliminateSpikes)
  {
    if(!SpikeCheck(f))
    {
      

      config->screen->Add(nx, ny, f);
    }
    else
    {
      // Wanna see the spikes !
      //  config->screen->Add(config->nx, config->ny, f);
      
      //  printf("SPIKE %g\n", COLORAVERAGE(f));

      
    }	  
  }
  else
  {
    config->screen->Add(nx, ny, f);
  }
}

void HandlePath_X_0(BPCONFIG *config, CBiPath *path)
{
  EDF *endingEdf = path->m_eyeEndNode->m_hit.material ? path->m_eyeEndNode->m_hit.material->edf : NULL;
  COLOR oldBsdfEval, f, frad;
  CBsdfComp oldBsdfComp;
  double factor, pdfLNE, oldPdfLNE;
  double oldPDFLightEval, oldPDFDirEval;
  double oldRRPDFLightEval, oldRRPDFDirEval;
  double pdf = 1.0, weight = 1.0;
  CPathNode *eyePrevNode, *eyeEndNode;

  if(path->m_eyeSize > config->bcfg->maximumPathDepth)
    return;

  if(endingEdf != NULL || config->bcfg->useSpars)
  {
    eyeEndNode = path->m_eyeEndNode;
    
    // Store the Bsdf and PDF evaluations that will be overwritten
    // by other values needed to compute the contribution
    
    eyePrevNode = eyeEndNode->Previous(); // always != NULL
    
    oldBsdfEval = eyeEndNode->m_bsdfEval;
    oldBsdfComp = eyeEndNode->m_bsdfComp;
    oldPDFLightEval = eyeEndNode->m_pdfFromNext;
    PNAN(eyeEndNode->m_pdfFromNext);
    
    oldPDFDirEval = eyePrevNode->m_pdfFromNext;
    PNAN(eyePrevNode->m_pdfFromNext);
    
    oldRRPDFLightEval = eyeEndNode->m_rrPdfFromNext;
    oldRRPDFDirEval = eyePrevNode->m_rrPdfFromNext;
    
    oldPdfLNE = path->m_pdfLNE;
    
    // Fill in new values
    
    eyeEndNode->m_bsdfComp.Clear();
    
    if(endingEdf != NULL)
    {
      // Landed on a light : fill in values for BPT
      eyeEndNode->m_bsdfEval = EdfEval(endingEdf, 
				       &eyeEndNode->m_hit,
				       &eyeEndNode->m_inDirF, 
				       ALL_COMPONENTS, (double *)0);
      eyeEndNode->m_bsdfComp.Fill(eyeEndNode->m_bsdfEval, 
				  BRDF_DIFFUSE_COMPONENT);
      
      if(config->lightConfig.maxDepth > 0)
      {
	eyeEndNode->m_pdfFromNext = 
	  config->lightConfig.pointSampler->EvalPDF(NULL, eyeEndNode);
	PNAN(eyeEndNode->m_pdfFromNext);
	
	eyeEndNode->m_rrPdfFromNext = 1.0; // Light point: no Russian R.
      }
      else
      {
	// Impossible to generate light point with a light sub path
	eyeEndNode->m_pdfFromNext = 0.0;
	eyeEndNode->m_rrPdfFromNext = 0.0;
      }
      
      if(config->lightConfig.maxDepth > 1)
      {
	eyePrevNode->m_pdfFromNext = 
	  config->lightConfig.dirSampler->EvalPDF(eyeEndNode, eyePrevNode);
	PNAN(eyePrevNode->m_pdfFromNext);
	
	eyePrevNode->m_rrPdfFromNext = 1.0;
      }
      else
      {
	eyePrevNode->m_pdfFromNext = 0.0;
	eyePrevNode->m_rrPdfFromNext = 0.0;
      }

      // Compute 
      if((config->bcfg->sampleImportantLights) && (config->lightConfig.maxDepth > 0)
	 && (path->m_eyeSize > 2))
      {
	pdfLNE = config->eyeConfig.neSampler->EvalPDF(eyePrevNode, eyeEndNode);
      }
      else
      {
	pdfLNE = eyeEndNode->m_pdfFromNext; // same sampling as lightpath
      }
    
      PNAN(pdfLNE);
      path->m_pdfLNE = pdfLNE;
    }

    // Path radiance evaluation

    path->m_geomConnect = 1.0; // Fake connection for X_0 paths

    if(config->bcfg->useSpars)
    {
      // f = config->sparList->HandlePath(&config->sparConfig, path);
      config->sparList->HandlePath(&config->sparConfig, path, &frad, &f);
      factor = 1.0; // pdf and weight already taken into account
    }
    else
    {
      // endingEdf != NULL !!
      f = path->EvalRadiance();
      
      //      if(COLORAVERAGE(f) > EPSILON)
      //{
      factor = path->EvalPDFAndWeight(config->bcfg, &pdf, &weight);
      //}
      //else
      //{
      //factor = 0.0;
      //}
    }

    factor *= config->fluxToRadFactor / config->bcfg->samplesPerPixel;

    if(config->bcfg->useSpars)
    {
      COLORSCALE(factor, frad, frad);
      AddWithSpikeCheck(config, path, config->nx, config->ny,
			config->xsample, config->ysample, frad, pdf, weight, true);
      COLORSCALE(factor, f, f);
      AddWithSpikeCheck(config, path, config->nx, config->ny,
			config->xsample, config->ysample, f, pdf, weight, false);

    }
    else
    {
      COLORSCALE(factor, f, f);
      AddWithSpikeCheck(config, path, config->nx, config->ny,
			config->xsample, config->ysample, f, pdf, weight);
    }

    // Restore the Brdf and PDF evaluations 
    
    path->m_pdfLNE = oldPdfLNE;
    eyeEndNode->m_bsdfEval = oldBsdfEval;
    eyeEndNode->m_bsdfComp = oldBsdfComp;
    eyeEndNode->m_pdfFromNext = oldPDFLightEval;
    eyePrevNode->m_pdfFromNext = oldPDFDirEval;
    eyeEndNode->m_rrPdfFromNext = oldRRPDFLightEval;
    eyePrevNode->m_rrPdfFromNext = oldRRPDFDirEval;
  }
}




COLOR ComputeNEFluxEstimate(BPCONFIG *config, CBiPath *path, 
			    double *pPdf = NULL, double *pWeight = NULL, COLOR *frad = NULL)
{
  CPathNode *eyePrevNode, *lightPrevNode;
  COLOR oldBsdfL, oldBsdfE;
  CBsdfComp oldBsdfCompL, oldBsdfCompE;
  double oldPdfL, oldPdfE, oldRRPdfL, oldRRPdfE;
  double oldPdfLP=0., oldPdfEP=0., oldRRPdfLP=0., oldRRPdfEP=0.;
  COLOR f;
  CPathNode *eyeEndNode, *lightEndNode;

  // Store PDF and BSDF evals that will be overwritten
  
  eyeEndNode = path->m_eyeEndNode;
  lightEndNode = path->m_lightEndNode;
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
  
  path->m_geomConnect = 
    PathNodeConnect(eyeEndNode, lightEndNode, 
		    &config->eyeConfig, &config->lightConfig,
		    CONNECT_EL | CONNECT_LE | FILL_OTHER_PDF,
		    BSDF_ALL_COMPONENTS, BSDF_ALL_COMPONENTS, &path->m_dirEL);
  
  VECTORSCALE(-1, path->m_dirEL, path->m_dirLE);

  //PNAN(eyeEndNode->m_pdfFromNext);
  //if(eyePrevNode) PNAN(eyePrevNode->m_pdfFromNext);
  //PNAN(lightEndNode->m_pdfFromNext);
  //if(lightPrevNode) PNAN(lightPrevNode->m_pdfFromNext);

  // Evaluate radiance and pdf and weight

  if(config->bcfg->useSpars)
  {
    // f = config->sparList->HandlePath(&config->sparConfig, path);
    config->sparList->HandlePath(&config->sparConfig, path, frad, &f);
  }
  else
  {
    f = path->EvalRadiance();


    //if(COLORAVERAGE(f) > EPSILON)
    //{
      double factor = path->EvalPDFAndWeight(config->bcfg, pPdf, pWeight);
      COLORSCALE(factor, f, f); // Flux estimate
      //}
      //else
      //{
      //  COLORCLEAR(f);
      //}
  }

  //PNAN(eyeEndNode->m_pdfFromNext);
  //if(eyePrevNode) PNAN(eyePrevNode->m_pdfFromNext);
  //PNAN(lightEndNode->m_pdfFromNext);
  //if(lightPrevNode) PNAN(lightPrevNode->m_pdfFromNext);

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




void HandlePath_X_X(BPCONFIG *config, CBiPath *path)
{
  COLOR f, frad;
  double oldPdfLNE, pdf, weight;
  bool doLNE;
  CPathNode newLightNode;
  CPathNode *oldLightPath=NULL, *oldLightEndNode;
  int oldLightSize;

  if((path->m_eyeSize + path->m_lightSize) > config->bcfg->maximumPathDepth)
    return;

  // Check if we need to sample another lightpoint with
  // importance sampling.

  doLNE = (path->m_lightSize == 1) && config->bcfg->sampleImportantLights;

  if(doLNE)
  {
    // Replace the current lightpath with new importance sampled path

    oldLightPath = path->m_lightPath;
    oldLightSize = path->m_lightSize;
    oldLightEndNode = path->m_lightEndNode;
    
    path->m_lightPath = &newLightNode;

    newLightNode.m_pdfFromPrev = 0.0;
    newLightNode.m_pdfFromNext = 0.0;

    if(!config->eyeConfig.neSampler->Sample(NULL, path->m_eyeEndNode, 
					    &newLightNode,
					    drand48(), drand48()))
    {
      // No light point sampled, no contribution possible

      path->m_lightPath = oldLightPath;
      return;
    }

    // Compute the normal sampling pdf for the new path/point

    oldPdfLNE = path->m_pdfLNE;
    path->m_pdfLNE = newLightNode.m_pdfFromPrev;
    newLightNode.m_pdfFromPrev = 
      config->lightConfig.pointSampler->EvalPDF(NULL, &newLightNode);

    PNAN(newLightNode.m_pdfFromPrev);
    PNAN(path->m_pdfLNE);

    path->m_lightEndNode = &newLightNode;

  }

//  else
//  {
//    pdfLNE = config->pdfLNE;
//  }

  if(PathNodesVisible(path->m_eyeEndNode, path->m_lightEndNode))
  {
    f = ComputeNEFluxEstimate(config, path, &pdf, &weight, &frad);

    float factor = config->fluxToRadFactor / config->bcfg->samplesPerPixel;
    COLORSCALE(factor, f, f);
    AddWithSpikeCheck(config, path, config->nx, config->ny,
		      config->xsample, config->ysample, f, pdf, weight);

    if(config->bcfg->useSpars)
    {
      COLORSCALE(factor, frad, frad);
      AddWithSpikeCheck(config, path, config->nx, config->ny,
			config->xsample, config->ysample, frad, pdf, weight, true);
    }    
  }

  if(doLNE)
  {
    // Restore the current lightpath

    path->m_lightPath = oldLightPath;
    path->m_lightSize = oldLightSize;
    path->m_lightEndNode = oldLightEndNode;
    path->m_pdfLNE = oldPdfLNE;
  }
}


void HandlePath_1_X(BPCONFIG *config, CBiPath *path)
{
  int nx,ny;
  float pix_x, pix_y;
  COLOR f, frad;
  double oldPdfLNE, pdf, weight;

  if((path->m_eyeSize + path->m_lightSize) > config->bcfg->maximumPathDepth)
    return;


  // We don't do light importance sampling for particle tracing paths

  oldPdfLNE = path->m_pdfLNE;
  path->m_pdfLNE = config->lightPath->m_pdfFromPrev;

  // First we need to determine if the lightEndNode can be seen from
  // the camera. At the same time the pixel hit is computed

  if(EyeNodeVisible(path->m_eyeEndNode, path->m_lightEndNode, 
		    &pix_x, &pix_y))
  {
    // Visible !

    f = ComputeNEFluxEstimate(config, path, &pdf, &weight, &frad);

    config->screen->GetPixel(pix_x, pix_y, &nx, &ny);

    float factor;
    if(config->bcfg->doDensityEstimation)
    {
      factor = (ComputeFluxToRadFactor(nx, ny)
		/ (float)config->bcfg->totalSamples);
    }
    else
    {
      factor = (ComputeFluxToRadFactor(nx, ny) 
		/ (float)config->bcfg->totalSamples);
    }
    COLORSCALE(factor, f, f);

    AddWithSpikeCheck(config, path, nx, ny, pix_x, pix_y, f, pdf, weight);

    if(config->bcfg->useSpars)
    {
      COLORSCALE(factor, frad, frad);
      AddWithSpikeCheck(config, path, nx, ny, pix_x, pix_y, frad, pdf, weight, true);
    }

  }

  // restore
  path->m_pdfLNE = oldPdfLNE;    
}



void BPCombinePaths(BPCONFIG *config)
{
  int eyeSize, lightSize;
  bool eyeSubPathDone, lightSubPathDone;
  CPathNode *eyeEndNode, *lightEndNode;
  CPathNode *eyePath, *lightPath;
  CBiPath path;

  eyePath = config->eyePath;
  lightPath = config->lightPath;


  // Compute pdfLNE for the second point in the light path

  if(config->lightPath)
  {
    // Single light point get importance sampled, so compute
    // the pdf for it, in order to get correct weights.
    if(config->bcfg->sampleImportantLights && config->lightPath->Next())
    {
      config->pdfLNE = 
	config->eyeConfig.neSampler->EvalPDF(lightPath->Next(),
					     lightPath);
							   
    }
    else
    {
      config->pdfLNE = lightPath->m_pdfFromPrev;
    }
  }
  else
  {
    config->pdfLNE = 0.0;
  }

  path.m_eyePath = eyePath;
  path.m_lightPath = lightPath;
  path.m_pdfLNE = config->pdfLNE;

  // Loop over all possible paths that can be constructed from
  // the eye and light path

  eyeSubPathDone = false;
  eyeSize = 1;  // Always include the eyepoint !
  eyeEndNode = eyePath;
  
  while(!eyeSubPathDone)
  {
    // Handle direct hit on light (lightSize = 0)

    if(eyeSize > 1)
    {
      // printf("Handle : E %i L %i\n", eyeSize, 0);

      path.m_eyeSize = eyeSize;
      path.m_eyeEndNode = eyeEndNode;
      path.m_lightSize = 0;
      path.m_lightEndNode = NULL;

      HandlePath_X_0(config, &path);
    }

    // Handle lightSize > 0 (with N.E.E.)

    lightSubPathDone = lightPath == NULL;
    lightSize = 1; // lightSize == 0 handled in X_0
    lightEndNode = lightPath;

    while(!lightSubPathDone)
    {
      // Compute N.E.

      if(eyeSize > 1)
      {
	// printf("Handle : E %i L %i\n", eyeSize, lightSize);

	path.m_eyeSize = eyeSize;
	path.m_eyeEndNode = eyeEndNode;
	path.m_lightSize = lightSize;
	path.m_lightEndNode = lightEndNode;
	HandlePath_X_X(config, &path);
      }
      else
      {
	// printf("Handle : E %i L %i\n", eyeSize, lightSize);

	path.m_eyeSize = eyeSize;
	path.m_eyeEndNode = eyeEndNode;
	path.m_lightSize = lightSize;
	path.m_lightEndNode = lightEndNode;

	HandlePath_1_X(config, &path);
      }

      if(lightEndNode->Ends())
      {
	lightSubPathDone = true;
      }
      else
      {
	lightSize++;
	lightEndNode = lightEndNode->Next();
      }	
    }

    if(eyeEndNode->Ends())
    {
      eyeSubPathDone = true;
    }
    else
    {
      eyeSize++;
      eyeEndNode = eyeEndNode->Next();
    }
  }
}

static COLOR BPCalcPixel(int nx, int ny, BPCONFIG *config)
{
  int i;
  double x_1, x_2;
  COLOR result;
  CStrat2D strat(config->bcfg->samplesPerPixel);
  CPathNode *pixNode, *nextNode;

  COLORCLEAR(result);

  // We sample the eye here since it's always the same point

  if(config->eyePath == NULL)
  {
    config->eyePath = new CPathNode;
  }

  config->eyeConfig.pointSampler->Sample(NULL, NULL, config->eyePath, 0, 0);
  ((CPixelSampler *)config->eyeConfig.dirSampler)->SetPixel(nx, ny);

  // Provide a node for the pixel sampling

  pixNode = config->eyePath->Next();

  if(pixNode == NULL)
  {
    pixNode = new CPathNode;
    config->eyePath->Attach(pixNode);
  }

  nextNode = pixNode->Next();
  if(nextNode == NULL)
  {
    nextNode = new CPathNode;
    pixNode->Attach(nextNode);
  }

  config->nx = nx;
  config->ny = ny;
  if(config->bcfg->doDensityEstimation)
    config->fluxToRadFactor = ComputeFluxToRadFactor(nx, ny);
  else
    config->fluxToRadFactor = ComputeFluxToRadFactor(nx, ny);

  for(i = 0; i < config->bcfg->samplesPerPixel; i++)
  {
    if(config->eyeConfig.maxDepth > 1)
    {
      // Generate an eye path 
      strat.Sample(&x_1, &x_2);
      
      config->eyePath->m_rayType = Starts;

      VEC2D tmpVec2D = config->screen->GetPixelCenter(x_1, x_2);
      config->xsample = tmpVec2D.u; // pix_x + (Camera.pixh * x_1);
      config->ysample = tmpVec2D.v; //pix_y + (Camera.pixv * x_2);

      //int mx,my;
      //config->screen->GetPixel(config->xsample, config->ysample, &mx, &my);
      //if(mx != nx || my != ny)
      //{
      //printf("Whoops mx %i nx %i my %i ny %i\n", mx, nx, my, ny);
      //printf("   pix_x %f x_1 %f pix_y %f  x_2 %f\n", pix_x, x_1,
      //       pix_y, x_2);
      //}
      
      if(config->eyeConfig.dirSampler->Sample(NULL, config->eyePath, 
					      pixNode, x_1, x_2))
      {
	pixNode->AssignBsdfAndNormal();
	
	config->eyeConfig.TracePath(nextNode);
      }
    }
    else
    {
      config->eyePath->m_rayType = Stops;
    }

    // Generate a light path

    if(config->lightConfig.maxDepth > 0)
      config->lightPath = config->lightConfig.TracePath(config->lightPath);
    else
      config->lightPath = NULL; // Normally this is already so, so no delete necessary ?!
      
    // Connect all endpoints and compute contribution

    BPCombinePaths(config);
  }

  // Radiance contributions are added to the screen buffer directly

  if(config->bcfg->doDensityEstimation)
  {
    if(config->dBuffer != NULL)
      result = config->screen->Get(nx, ny);
    else
      result = config->dest->Get(nx, ny);
  }
  else
    result = config->screen->Get(nx, ny);
    
  return result;
}




static void DoBPTAndSubsequentImages(BPCONFIG *config)
{
  int maxSamples, nrIterations;
  char *format1 = new char[300];
  char *format2 = new char[300];
  char *filename = new char[300];

  // Do some trick to render several images, with different
  // number of samples per pixel.
  
  // Get highest power of two < number of samples
  
  nrIterations = (int)(log((double)bidir.basecfg.samplesPerPixel)/ log(2.0));
  maxSamples = (int)pow(2.0, nrIterations);

  nrIterations += 1; // First two are 1 and 1
  
  printf("nrIter %i, maxSamples %i, origSamples %i\n", nrIterations,
	 maxSamples, bidir.basecfg.samplesPerPixel);
  
  printf("Base name '%s'\n", bidir.baseFilename);

  // Numbers are placed after the last point. If
  // no point is given, both ppm.gz and tif images
  // are saved.

  char *last_occ = strrchr(bidir.baseFilename, '.');

  if(last_occ == NULL)
  {
    sprintf(format1, "%s%%i.tif", bidir.baseFilename);
    sprintf(format2, "%s%%i.ppm.gz", bidir.baseFilename);
  }
  else
  {
    strncpy(format1, bidir.baseFilename, last_occ - bidir.baseFilename);
    strcat(format1, "%i");
    strcat(format1, last_occ);
    format2[0] = '\0';
  }

  printf("Format 1 '%s'\n", format1);
  printf("Format 2 '%s'\n", format2);


  // Now trace the screen several times, each time
  // with twice the number of samples as the previous
  // time. The screenbuffer is scaled by 0.5 each iteration

  int currentSamples = 1;
  int totalSamples = 1;

  for(int i = 0; i < nrIterations; i++)
  {
    // Halve the screen contribs
    
    if(i > 0)
    {
      config->screen->ScaleRadiance(0.5);
      config->screen->SetAddScaleFactor(0.5);
    }

    config->bcfg->samplesPerPixel = currentSamples;
    config->bcfg->totalSamples = currentSamples * Camera.hres * Camera.vres;;

    ScreenIterateSequential((COLOR(*)(int,int,void *))BPCalcPixel, config);

    config->screen->Render();

    // Save images
    if(format1[0] != '\0')
    {
      sprintf(filename, format1, totalSamples);
      config->screen->WriteFile(filename);
    }
    if(format2[0] != '\0')
    {
      sprintf(filename, format2, totalSamples);
      config->screen->WriteFile(filename);
    }


    if(i > 0)
      currentSamples *= 2;

    totalSamples += currentSamples;
  }

  delete[] format1;
  delete[] format2;
  delete[] filename;
}


void DoBPTDensityEstimation(BPCONFIG *config)
{
  char *fname = new char[300];

  // Init the screens, one reference one destination

  config->ref = new CScreenBuffer;
  config->dest = new CScreenBuffer;

  config->ref->SetFactor(1.0);
  config->dest->SetFactor(1.0);

  config->dBuffer = new CDensityBuffer(config->ref, config->bcfg);

  if(config->bcfg->useSpars)
  {
    config->ref2 = new CScreenBuffer;
    config->dest2 = new CScreenBuffer;

    config->ref2->SetFactor(1.0);
    config->dest2->SetFactor(1.0);

    config->dBuffer2 = new CDensityBuffer(config->ref2, config->bcfg);
  }
  else
  {
    config->ref2 = NULL;
    config->dest2 = NULL;
    config->dBuffer2 = NULL;
  }

  
  // First make a run with D.E. to build ref
  

  config->bcfg->samplesPerPixel = 1;
  config->bcfg->totalSamples = (config->bcfg->samplesPerPixel *
				config->ref->GetHRes() * config->ref->GetVRes());

  config->deStoreHits = true;

  // Do the run
  ScreenIterateSequential((COLOR(*)(int,int,void *))BPCalcPixel, config);
  
  // Now we have a noisy screen in dest and hits in dbuffer

  // Density estimation

  config->dBuffer->Reconstruct(); // Estimates ref with fixed kernel width
  config->ref->Render();
  //  config->ref->WriteFile("./firstreffix.ppm.gz");

  if(config->dBuffer2)
  {
    config->dBuffer2->Reconstruct(); // Estimates ref with fixed kernel width
    config->ref2->Render();
    //    config->ref2->WriteFile("./firstref2fix.ppm.gz");
  }

  // Now reconstruct the hits using variable kernels
  config->dBuffer->ReconstructVariable(config->dest, 5.0);
  config->dest->Render();
  //  config->dest->WriteFile("./firstrefvar.ppm.gz");

  if(config->dBuffer2)
  {
    config->dBuffer2->ReconstructVariable(config->dest2, 1.5);
    config->dest2->Render();
    //    config->dest2->WriteFile("./firstref2var.ppm.gz");
  }

  delete config->dBuffer; // Not needed anymore
  config->dBuffer = NULL;

  
  if(config->dBuffer2)
  {
    delete config->dBuffer2; // Not needed anymore
    config->dBuffer2 = NULL;
  }

  // We have a better approximation in dest based
  // on some samples per pixel BPT.

  // We will now do subsequent BPT passes, adding the
  // hits directly to dest.

  config->deStoreHits = false;

  int nrIterations = (int)floor(log(bidir.basecfg.samplesPerPixel)/(log(2)));
  int maxSamples = (int)pow(2.0, nrIterations);

  printf("Doing %i iterations, thus %i samples per pixel\n", nrIterations,
	 maxSamples);

  int oldSPP = config->bcfg->samplesPerPixel; // These were stored
  int newSPP = oldSPP;
  int oldTotalSPP = oldSPP;
  int newTotalSPP = oldTotalSPP + newSPP;

  for(int i = 1; i <= nrIterations && !interrupt_raytracing; i++)
  {
    printf("Doing run with %i samples, %i samples already done\n", newSPP, 
	   oldTotalSPP);

    // copy dest to ref

    config->ref->Copy(config->dest);

    if(config->ref2)
      config->ref2->Copy(config->dest2);


    // Rescale dest : Nold / Nnew
    config->dest->ScaleRadiance((float)oldTotalSPP / (float)newTotalSPP);

    // Set scale factor for added radiance in this run
    config->dest->SetAddScaleFactor((float)newSPP/(float)newTotalSPP);

    if(config->dest2)
    {
      // Rescale dest : Nold / Nnew
      config->dest2->ScaleRadiance((float)oldTotalSPP / (float)newTotalSPP);
      
      // Set scale factor for added radiance in this run
      config->dest2->SetAddScaleFactor((float)newSPP/(float)newTotalSPP);
    }

    // Set current vars

    config->bcfg->samplesPerPixel = newSPP;
    config->bcfg->totalSamples = (config->bcfg->samplesPerPixel *
				config->ref->GetHRes() * config->ref->GetVRes());

    config->scaleSamples = newTotalSPP; // Base kernel size on total #s/pix

    // Iterate screen : Nnew - Nold, using an appropriate scale factor

    ScreenIterateSequential((COLOR(*)(int,int,void *))BPCalcPixel, config);

    // Render screen & write

    config->dest->Render();
    sprintf(fname, "deScreen%i.ppm.gz", newTotalSPP);
    //    config->dest->WriteFile(fname);


    if(config->dest2)
    {
      config->dest2->Render();
      sprintf(fname, "de2Screen%i.ppm.gz", newTotalSPP);
      //      config->dest2->WriteFile(fname);

      // Merge two images (just add!) into screen

      config->screen->Merge(config->dest, config->dest2);
      config->screen->Render();
      sprintf(fname, "deMRGScreen%i.ppm.gz", newTotalSPP);
      //      config->screen->WriteFile(fname);
    }
    else
    {
      config->screen->Copy(config->dest);
    }

    // Update vars

    oldSPP = newSPP;
    newSPP = newSPP * 2;
    oldTotalSPP = newTotalSPP;
    newTotalSPP = oldTotalSPP + newSPP;    
  }

  if(interrupt_raytracing)
  {
    if(config->ref2)
    {
      config->screen->Merge(config->ref, config->ref2);
    }
    else
    {
      config->screen->Copy(config->ref);  // Interrupt, use last reference image
    }
  }

  delete config->dest;
  delete config->ref;
  if(config->ref2) delete config->ref2;
  if(config->dest2) delete config->dest2;
}




static void BidirPathTrace(ImageOutputHandle *ip)
{
  // Install the samplers to be used in the state

  BPCONFIG config;

  // Copy base config (so that rendering is independent of GUI)
  config.bcfg = new BP_BASECONFIG;
  *(config.bcfg) = bidir.basecfg;
  config.bcfg->totalSamples = bidir.basecfg.samplesPerPixel * Camera.hres * Camera.vres;

  config.dBuffer = NULL;

  // Eye and light path sampling config
  config.eyeConfig.pointSampler = new CEyeSampler;
  config.eyeConfig.dirSampler = new CPixelSampler;
  config.eyeConfig.surfaceSampler = new CBsdfSampler;
  config.eyeConfig.surfaceSampler->SetComputeFromNextPdf(true);
  config.eyeConfig.surfaceSampler->SetComputeBsdfComponents(bidir.basecfg.useSpars);

  if(bidir.basecfg.sampleImportantLights)
  {
    config.eyeConfig.neSampler = new CImportantLightSampler;
  }
  else
  {
    config.eyeConfig.neSampler = new CUniformLightSampler;
  }

  config.eyeConfig.minDepth = bidir.basecfg.minimumPathDepth;

  if(bidir.basecfg.maximumEyePathDepth < 1)
  {
    fprintf(stderr, "Maximum Eye Path Length too small (<1), using 1\n");
    config.eyeConfig.maxDepth = 1;
  }
  else
    config.eyeConfig.maxDepth = bidir.basecfg.maximumEyePathDepth;

  config.lightConfig.pointSampler = new CUniformLightSampler;
  config.lightConfig.dirSampler = new CLightDirSampler;
  config.lightConfig.surfaceSampler = new CBsdfSampler;
  config.lightConfig.surfaceSampler->SetComputeFromNextPdf(true);
  config.lightConfig.surfaceSampler->SetComputeBsdfComponents(bidir.basecfg.useSpars);

  config.lightConfig.minDepth = bidir.basecfg.minimumPathDepth;
  config.lightConfig.maxDepth = bidir.basecfg.maximumLightPathDepth;
  config.lightConfig.neSampler = NULL; // eyeSampler ?


  // config.maxCombinedLength = bidir.basecfg.maximumPathDepth;

  config.screen = new CScreenBuffer;
  config.screen->SetFactor(1.0); // We're storing plain radiance
    
  config.eyePath = NULL;
  config.lightPath = NULL;

  // SPaR configuration

  CLeSpar *leSpar;
  CLDSpar *ldSpar;
#ifdef WMP_WEIGHTS
  CIDSpar *idSpar;
#endif 

  if(bidir.basecfg.useSpars)
  {
    CSparConfig *sc = &config.sparConfig;
    
    sc->m_bcfg = config.bcfg; // Share base config options

#ifdef WMP_WEIGHTS
    if(config.bcfg->doLD || config.bcfg->doLI)
    {
      Warning("Regular Expressions, direct & indirect", 
	      "Only works with GALERKING radiosity!");
    }
#endif 
    
    config.sparList = new CSparList;
    
    leSpar = new CLeSpar;
    ldSpar = new CLDSpar;
#ifdef WMP_WEIGHTS
    idSpar = new CIDSpar;
#endif 
    
    sc->m_leSpar = leSpar;
    sc->m_ldSpar = ldSpar;
    
    leSpar->Init(sc);
    ldSpar->Init(sc);
#ifdef WMP_WEIGHTS
    idSpar->Init(sc);
#endif 
    
    config.sparList->Add(leSpar);
    config.sparList->Add(ldSpar);
#ifdef WMP_WEIGHTS
    config.sparList->Add(idSpar);
#endif 
  }

  if(bidir.saveSubsequentImages)
  {
    DoBPTAndSubsequentImages(&config);
  }
  else if(config.bcfg->doDensityEstimation)
  {
    DoBPTDensityEstimation(&config);
  }
  else if(!bidir.basecfg.progressiveTracing)
  {
    ScreenIterateSequential((COLOR(*)(int,int,void *))BPCalcPixel, &config);
  }
  else
  {
    ScreenIterateProgressive((COLOR(*)(int,int,void *))BPCalcPixel, &config);
  }

  config.screen->Render();

  if(ip) config.screen->WriteFile(ip);

  if (bidir.lastscreen)
    delete bidir.lastscreen;
  bidir.lastscreen = config.screen;

  
  if(bidir.basecfg.useSpars)
  {
    delete config.sparList;
    
    delete leSpar;
    delete ldSpar;
#ifdef WMP_WEIGHTS
    delete idSpar;
#endif 
  }

  delete config.eyeConfig.pointSampler;
  delete config.eyeConfig.dirSampler;
  delete config.eyeConfig.surfaceSampler;
  delete config.eyeConfig.neSampler;

  delete config.lightConfig.pointSampler;
  delete config.lightConfig.dirSampler;
  delete config.lightConfig.surfaceSampler;

  if(config.dBuffer)
  {
    delete config.dBuffer;
    config.dBuffer = NULL;
  }

  delete config.bcfg;
}



COLOR Bidir_GetPixel(int nx, int ny)
{
  return bidir.lastscreen->Get(nx, ny);
}



static double BidirPathAdaptationLuminance(float scale)
{
  if (bidir.lastscreen) {
    return bidir.lastscreen->AdaptationLuminance(scale);
  } else
    return 1.0;
}

static void BidirPathRecomputeDisplayColors(void)
{
  if (bidir.lastscreen) bidir.lastscreen->Sync();
}

static int BidirPathRedisplay(void)
{
  if (bidir.lastscreen) {
    // bidir.lastscreen->Sync();
    bidir.lastscreen->Render();
    return true;
  } else
    return false;
}

static int BidirPathReproject(void)
{
  if (bidir.lastscreen) {
    // bidir.lastscreen->Sync();
    bidir.lastscreen->Reproject();
    return true;
  } else
    return false;
}

static int BidirPathSaveImage(ImageOutputHandle *ip)
{
  if (ip && bidir.lastscreen) {
    bidir.lastscreen->Sync();
    bidir.lastscreen->WriteFile(ip);
    return true;
  } else
    return false;
}

void BidirPathInterrupt(void)
{
  interrupt_raytracing = TRUE;
}

static void BidirPathInit(void)
{
  // Init the light list
  if(gLightList) delete gLightList;
  gLightList = new CLightList(LightSourcePatches);
}  

static void BidirPathTerminate(void)
{
  if (bidir.lastscreen)
    delete bidir.lastscreen;
  bidir.lastscreen = (CScreenBuffer *)0;
}

RAYTRACINGMETHOD RT_BidirPathMethod =
{
  "BidirectionalPathTracing", 4,
  "Bidirectional Path Tracing",
  "rtmBidirPathButton",
  BidirPathDefaults,
  CreateBidirPathControlPanel,
  BidirPathParseOptions,
  BidirPathPrintOptions,
  BidirPathInit,
  BidirPathTrace,
  BidirPathAdaptationLuminance,
  BidirPathRecomputeDisplayColors,
  BidirPathRedisplay,
  BidirPathReproject,
  BidirPathSaveImage,
  BidirPathInterrupt,
  BidirPathShowControlPanel,
  BidirPathHideControlPanel,
  BidirPathTerminate
};
