

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

#include "background.h"

#include "rtsoptions.H"

#include "patch.h"
#include "patchlist.h"
#include "radiance.h"
#include "Float.h"
#include "bsdf.h"
#include "splitbsdf.h"
#include "grid.h"

#include "spherical.h"

#include "rtstochastic.h"
#include "rtstochastic_priv.h"

#include "eyesampler.H"
#include "pixelsampler.H"
#include "lightsampler.H"
#include "bsdfsampler.H"
#include "specularsampler.H"
#include "photonmapsampler.H"
#include "samplertools.H"
#include "raytools.H"
#include "screeniterate.H"
#include "screenbuffer.H"

#include "stratification.H"
#include "lightlist.H"

#include "../photonmapping/pmap.h"
#include "../montecarlo/mcrad.h"



// Heuristic for Multiple Importance sampling
#define MISFUNC(a) ((a)*(a))

// Heuristic minimum distance threshold for photonmap readouts
// should be tuned and dependent on scene size, ...
// -- TODO Match with importance driven photon maps: gThreshold
const float PMAP_MIN_DIST = 0.02;
const float PMAP_MIN_DIST2 = PMAP_MIN_DIST * PMAP_MIN_DIST; // squared


RTStochastic_State rts;


class CSeed
{
private:
  unsigned short m_seed[3];
public:
  unsigned short *GetSeed() { return m_seed; }
  void SetSeed(CSeed seed)
  { 
    unsigned short *s = seed.GetSeed();
    m_seed[0] = s[0];
    m_seed[1] = s[1];
    m_seed[2] = s[2];
  }
  void SetSeed(unsigned short seed16v[3])
  { m_seed[0] = seed16v[0]; m_seed[1] = seed16v[1]; m_seed[2] = seed16v[2];}
  void SetSeed(unsigned short s0,unsigned short s1, unsigned short s2)
  { m_seed[0] = s0; m_seed[1] = s1; m_seed[2] = s2;}
  void XORSeed(CSeed xorseed)
  { 
    unsigned short *s = xorseed.GetSeed();
    m_seed[0] ^= s[0];
    m_seed[1] ^= s[1];
    m_seed[2] ^= s[2];
  }  
};

class CSeedConfig
{
private:
  CSeed* m_seeds;
  static CSeed m_xorseed;

public:

  // Constructor
  CSeedConfig()
  {
    m_xorseed.SetSeed(0xF0,0x65,0xDE); 
    m_seeds = 0;
  }

  void Clear()
  {
    if(m_seeds) delete[] m_seeds;
  }

  void Init(int maxdepth)
  {
    Clear();
    m_seeds = new CSeed[maxdepth];
  }

  ~CSeedConfig()
  {
    Clear();
  }

  // Saves the current seed and generates a new seeds based
  // on the current seed
  void Save(int depth)
  {
    // Save the seed (supply dummy seed to seed48())
    m_seeds[depth].SetSeed(seed48(m_seeds[depth].GetSeed()));

    //Generate a new seed, dependent on the current seed
    CSeed tmpSeed;

    tmpSeed.SetSeed(m_seeds[depth]);
    // Fixed xor should do the trick. Note that you can not use
    // the random number generator itself to generate new seeds,
    // because the supplied random numbers *are* the (truncated) seeds
    tmpSeed.XORSeed(m_xorseed);  
    // Set the new seed and drand48 once, to be sure
    seed48(tmpSeed.GetSeed());
    drand48();
  }

  // Restores seed for a certain depth
  void Restore(int depth)
  {
    seed48(m_seeds[depth].GetSeed());
  }
};

// Class member inits

CSeed CSeedConfig::m_xorseed;







// CScatterinfo includes information about different
// scattering properties for different bsdf components
// This info is used during scattering, but also
// when weighting or reading storage decisions
// must be made.

class CScatterInfo
{
public:
  // The components under consideration
  BSDFFLAGS flags;
  // Spawning factor if no 'flags' bounce was made before
  int nrSamplesBefore;
  // Spawning factor after at least one 'flags' bounce
  int nrSamplesAfter;

  // Some utility functions

  // Were 'flags' last used in the bounce in 'node'
  bool DoneThisBounce(CPathNode *node)
  { return (node->m_usedComponents == flags); }

  // Were 'flags' used at some point in the path
  bool DoneSomeBounce(CPathNode *node)
  { return (((node->m_accUsedComponents | node->m_usedComponents) & flags) == flags); }

  // Were 'flags' used at some previous point in the path
  bool DoneSomePreviousBounce(CPathNode *node)
  { return ((node->m_accUsedComponents & flags) == flags); }
};


// Next enum is needed to track readout of storage.

typedef enum SRREADOUT
{
  SCATTER, READ_NOW
} SRREADOUT;


class SRCONFIG
{
public:
  // *** user options 

  int samplesPerPixel;

  int nextEventSamples;
  RTSLIGHT_MODE lightMode;

  RTSRAD_MODE radMode;

  int scatterSamples;
  int firstDGSamples;
  RTSSAMPLING_MODE reflectionSampling;
  bool separateSpecular;

  bool backgroundIndirect;      // use background in reflections (indirect)
  bool backgroundDirect;        // use background when no surface is hit (direct)
  bool backgroundSampling;      // use light sampling

  // *** independent variables
  CScreenBuffer *screen;

  // *** variables derived from user options
  // *** all variables must not change during raytracing...
  CSamplerConfig samplerConfig;
  CSeedConfig seedConfig;
  long baseSeed;

  // Scatter info blocks

  // Scattering info for the part of light
  // transport that is used from storage
  // (Diffuse for RAD, Diffuse & Glossy for Pmap)
  CScatterInfo siStorage;

  // Maximum 1 scattering block per component
  CScatterInfo siOthers[BSDFCOMPONENTS];
  int siOthersCount;

  SRREADOUT initialReadout;

#ifdef RTDEBUG
  // *** Debug options
  bool debugInfo;
#endif

  // *** Obsolete options
  int storedRadiance;
  int photonmapMethod;


  // *** Methods
  
public:
  void Init(RTStochastic_State& state);

  // Constructors
  SRCONFIG() {}
  SRCONFIG(RTStochastic_State& state) { Init(state); }
  ~SRCONFIG();

private:
  void InitDependentVars(void);
};

// SRCONFIG methods 

void SRCONFIG::Init(RTStochastic_State& state)
{
  // Copy state options

  samplesPerPixel = state.samplesPerPixel;

  baseSeed = state.baseSeed;

  radMode = state.radMode;

  backgroundIndirect = state.backgroundIndirect;
  backgroundDirect = state.backgroundDirect;
  backgroundSampling = state.backgroundSampling;

  if(radMode != STORED_NONE)
  {
    if(Radiance == NULL)
    {
      Error("Stored Radiance", "No radiance method active, using no storage");
      radMode = STORED_NONE;
    }
    else if((radMode == STORED_PHOTONMAP) && (Radiance != &Pmap))
    {
      Error("Stored Radiance", "Photonmap method not active, using no storage");
      radMode = STORED_NONE;
    }
  }

  if(state.nextEvent)
    nextEventSamples = state.nextEventSamples;
  else
    nextEventSamples = 0;
  lightMode = state.lightMode;

  reflectionSampling = state.reflectionSampling;

  if(reflectionSampling == CLASSICALSAMPLING)
  {
    if(radMode == STORED_INDIRECT || radMode == STORED_PHOTONMAP)
    {
      Error("Classical raytracing", 
	    "Incompatible with extended final gather, using storage directly");
      radMode = STORED_DIRECT;
    }
  }

  

  if(radMode == STORED_PHOTONMAP)
  {
    Warning("Photon map reflection Sampling", 
	    "Make sure to use the same sampling (brdf,fresnel) as for photonmap construction");
  }

  scatterSamples = state.scatterSamples;
  if(state.differentFirstDG)
    firstDGSamples = state.firstDGSamples;
  else
    firstDGSamples = scatterSamples;

  separateSpecular = state.separateSpecular;

  if(reflectionSampling == PHOTONMAPSAMPLING)  //radMode == STORED_PHOTONMAP)
  {
    Warning("Fresnel Specular Sampling", 
	    "always uses separate specular");
    separateSpecular = true;  // Always separate specular with photonmap
  }

  samplerConfig.minDepth = state.minPathDepth;
  samplerConfig.maxDepth = state.maxPathDepth;

  screen = new CScreenBuffer;
  screen->SetFactor(1.0); // We're storing plain radiance

#ifdef RTDEBUG
  debugInfo = false;
#endif

  InitDependentVars();
}

void SRCONFIG::InitDependentVars(void)
{
  // Sampler configuration
  samplerConfig.pointSampler = new CEyeSampler;
  samplerConfig.dirSampler = new CPixelSampler;

  switch(reflectionSampling)
  {
  case BRDFSAMPLING:
    //if(radMode == STORED_PHOTONMAP)
    //  samplerConfig.surfaceSampler = new CPhotonMapSampler;      
    //else
    samplerConfig.surfaceSampler = new CBsdfSampler;      
    break;
  case PHOTONMAPSAMPLING:
    samplerConfig.surfaceSampler = new CPhotonMapSampler;
    break;
  case CLASSICALSAMPLING:
    samplerConfig.surfaceSampler = new CSpecularSampler;
    break;
  default:
    Error("SRCONFIG::InitDependentVars", "Wrong sampling mode");
  }

  if(lightMode == IMPORTANT_LIGHTS)
  {
    samplerConfig.neSampler = new CImportantLightSampler;
  }
  else
  {
    samplerConfig.neSampler = new CUniformLightSampler;
  }  


  // Scatter info blocks

  // Storage block

  BSDFFLAGS storeFlags;
  
  if((Radiance == NULL) || (radMode == STORED_NONE))
    storeFlags = NO_COMPONENTS;
  else 
  {
    if(Radiance == &Pmap)
      storeFlags = BSDF_GLOSSY_COMPONENT | BSDF_DIFFUSE_COMPONENT;
    else
      storeFlags = BRDF_DIFFUSE_COMPONENT;
  }

  initialReadout = SCATTER;

  switch(radMode)
  {
  case STORED_NONE:
    siStorage.flags = NO_COMPONENTS;
    siStorage.nrSamplesBefore = 0;
    siStorage.nrSamplesAfter = 0;
    break;
  case STORED_DIRECT:
    siStorage.flags = storeFlags;
    siStorage.nrSamplesBefore = 0;
    siStorage.nrSamplesAfter = 0;
    initialReadout = READ_NOW;
    break;
  case STORED_INDIRECT:
  case STORED_PHOTONMAP:
    siStorage.flags = storeFlags;
    siStorage.nrSamplesBefore = firstDGSamples;
    siStorage.nrSamplesAfter = 0;
    break;
  default:
    Error("SRCONFIG::InitDependentVars", "Wrong Rad Mode");
  }

  // Other blocks, this is non storage with optional
  //   separation of specular components

  BSDFFLAGS remainingFlags = BSDF_ALL_COMPONENTS & ~storeFlags;
  int siIndex = 0;

  if(separateSpecular)
  {
    BSDFFLAGS flags;

    // spec reflection

    if(reflectionSampling == CLASSICALSAMPLING)
      flags = remainingFlags & (BRDF_SPECULAR_COMPONENT | 
				BRDF_GLOSSY_COMPONENT); // Glossy == Specular in classic
    else
      flags = remainingFlags & BRDF_SPECULAR_COMPONENT;


    if(flags)
    {
      siOthers[siIndex].flags = flags;
      siOthers[siIndex].nrSamplesBefore = scatterSamples;
      siOthers[siIndex].nrSamplesAfter = scatterSamples;
      siIndex++;
      remainingFlags &= ~flags;
    }

    // spec transmission

    if(reflectionSampling == CLASSICALSAMPLING)
      flags = remainingFlags & (BTDF_SPECULAR_COMPONENT | 
				BTDF_GLOSSY_COMPONENT); // Glossy == Specular in classic
    else
      flags = remainingFlags & BTDF_SPECULAR_COMPONENT;

    if(flags)
    {
      siOthers[siIndex].flags = flags;
      siOthers[siIndex].nrSamplesBefore = scatterSamples;
      siOthers[siIndex].nrSamplesAfter = scatterSamples;
      siIndex++;
      remainingFlags &= ~flags;
    }
  }

  // Glossy or diffuse with different firstDGSamples

  if((reflectionSampling != CLASSICALSAMPLING) && (scatterSamples != firstDGSamples))
  {
    BSDFFLAGS gdFlags = (remainingFlags & 
			 (BSDF_DIFFUSE_COMPONENT | BSDF_GLOSSY_COMPONENT));
    if(gdFlags)
    {
      siOthers[siIndex].flags = gdFlags;
      siOthers[siIndex].nrSamplesBefore = firstDGSamples;
      siOthers[siIndex].nrSamplesAfter = scatterSamples;
      siIndex++;
      remainingFlags &= ~gdFlags;
    }      
  }

  if(reflectionSampling == CLASSICALSAMPLING)
  {
    // Classical: Diffuse, with no scattering
    BSDFFLAGS dFlags = (remainingFlags & BSDF_DIFFUSE_COMPONENT);

    if(dFlags)
    {
      siOthers[siIndex].flags = dFlags;
      siOthers[siIndex].nrSamplesBefore = 0;
      siOthers[siIndex].nrSamplesAfter = 0;
      siIndex++;
      remainingFlags &= ~dFlags;
    }      
  }
  
  // All other flags (possibly none) just get scattered normally

  if(remainingFlags)
  {
    siOthers[siIndex].flags = remainingFlags;
    siOthers[siIndex].nrSamplesBefore = scatterSamples;
    siOthers[siIndex].nrSamplesAfter = scatterSamples;
    siIndex++;
  }

  siOthersCount = siIndex;

  // Init the light list
  if(gLightList) delete gLightList;
  gLightList = new CLightList(LightSourcePatches, (bool)backgroundSampling);


  // Init the seed config

  seedConfig.Init(samplerConfig.maxDepth);  
}

SRCONFIG::~SRCONFIG(void)
{
  // Delete any allocated objects...

  samplerConfig.ReleaseVars();
}



COLOR RndColor(int , int , void *)
{
  COLOR col;
  COLORSET(col, drand48(), drand48(), drand48());
  return col;
}






static bool IsSpikeOrNaN(COLOR col)
{
  // Check for spikes or NaN's and return true for
  // a spike or invalid sample. Normally not used...

  char str[200];
  
  sprintf(str, "%g", COLORAVERAGE(col));
  if(str[0] == 'n' || str[0] == 'N' || str[1] == 'n' || str[1] == 'N')
  {
    printf("COL ");
    ColorPrint(stdout, col);
    printf("\n");
    return true;
  }

  if(COLORAVERAGE(col) > 60000 )
  {
    return true;
  }

  if(COLORAVERAGE(col) < -1 )
  {
    return true;
  }

  return false;
}


// Forward declaration
COLOR SR_GetRadiance(CPathNode *thisNode, SRCONFIG *config, SRREADOUT readout,
		     int usedScatterSamples);


COLOR SR_GetScatteredRadiance(CPathNode *thisNode, SRCONFIG *config,
			      SRREADOUT readout)
{
  int siCurrent; // What scatterblock are we handling
  CScatterInfo *si;

  CPathNode newNode;
  thisNode->Attach(&newNode);

  COLOR result;
  COLORCLEAR(result);

  if((config->samplerConfig.surfaceSampler == NULL) ||
     (thisNode->m_depth >= config->samplerConfig.maxDepth))
  {
    // No scattering
    return result;
  }

  if((config->siStorage.flags != NO_COMPONENTS) &&
     (readout == SCATTER))
  {
    // Do storage components
    si = &config->siStorage;
    siCurrent = -1;
  }
  else
  {
    // No direct light using storage components
    si = &config->siOthers[0];
    siCurrent = 0;
  }

  while(siCurrent < config->siOthersCount)
  {
    int nrSamples;

    if(si->DoneSomePreviousBounce(thisNode))
      nrSamples = si->nrSamplesAfter;
    else
      nrSamples = si->nrSamplesBefore; // First bounce of this kind

    // A small optimisation to prevent sampling surface that
    // don't have this scattering component.
    
    if(nrSamples > 2)  // Some bigger value may be more efficient
    {
      COLOR albedo = BsdfScatteredPower(thisNode->m_useBsdf,
					&thisNode->m_hit, &thisNode->m_normal,
					si->flags);
      if(COLORAVERAGE(albedo) < EPSILON) nrSamples = 0; // Skip, no contribution anyway
    }
	
    // printf("Flags %x, nrs %i\n", (int)si->flags, nrSamples);
    
    // Do we need to compute scattered radiance at all...

    if((nrSamples > 0) && (thisNode->m_depth + 1 < config->samplerConfig.maxDepth))
    {
      int i;
      double x_1, x_2, factor;
      CStrat2D strat(nrSamples);
      COLOR radiance;
      bool doRR = thisNode->m_depth >= config->samplerConfig.minDepth;

      for(i = 0; i < nrSamples; i++)
      {
	strat.Sample(&x_1, &x_2);

	// Surface sampling

	if(config->samplerConfig.surfaceSampler->Sample(thisNode->Previous(), 
							thisNode,
							&newNode, x_1, x_2, 
							doRR,
							si->flags)
	   && ((newNode.m_rayType != Environment) || (config->backgroundIndirect)))
	{
	  if(newNode.m_rayType != Environment) 
	    newNode.AssignBsdfAndNormal();

	  // Frame coherent & correlated sampling
	  if(rts.doFrameCoherent || rts.doCorrelatedSampling)
	  {
	    config->seedConfig.Save(newNode.m_depth);    
	  }

	  // Get the incoming radiance
	  if(siCurrent == -1)  
	    // Storage bounce
	    radiance = SR_GetRadiance(&newNode, config, READ_NOW, nrSamples);
	  else
	    radiance = SR_GetRadiance(&newNode, config, readout, nrSamples);

	  // Frame coherent & correlated sampling
	  if(rts.doFrameCoherent || rts.doCorrelatedSampling)
	  {
	    config->seedConfig.Restore(newNode.m_depth);    
	  }

	  // Collect outgoing radiance
	  factor = newNode.m_G / (newNode.m_pdfFromPrev * nrSamples);

	  COLORPRODSCALED(radiance, factor, thisNode->m_bsdfEval, radiance);
	  COLORADD(radiance, result, result);
	}
      } // for each sample
    } // if nrSamples > 0

    // Next scatter info block
    siCurrent++;
    if(siCurrent < config->siOthersCount)
      si = &config->siOthers[siCurrent];
  } // while not all si done

  thisNode->SetNext(NULL);
  return result;
}


COLOR SR_GetDirectRadiance(CPathNode *prevNode, SRCONFIG *config,
			   SRREADOUT readout)
{
  COLOR result, radiance;
  COLORCLEAR(result);
  VECTOR dirEL;

  if((readout == READ_NOW) && (config->radMode == STORED_PHOTONMAP))
    return result; // We're reading out D|G, specular not with direct light

  CNextEventSampler *nes = config->samplerConfig.neSampler;

  // Check if N.E.E. can give a contribution. I.e. not inside
  // a medium or just about to leave to vacuum
  

  if((nes != NULL) && 
     (config->nextEventSamples > 0) &&
     (prevNode->m_depth + 1 < config->samplerConfig.maxDepth))
  {
    int i;
    CPathNode lightNode;
    double x_1, x_2, geom, weight, cl, cr, factor, nrs;
    bool lightsToDo = true;

    if(config->lightMode == ALL_LIGHTS)
    { 
      lightsToDo = nes->ActivateFirstUnit();
    }



    while(lightsToDo)
    {
      CStrat2D strat(config->nextEventSamples);

      for(i = 0; i < config->nextEventSamples; i++)
      {
	// Light sampling
	strat.Sample(&x_1, &x_2);
	
	if(config->samplerConfig.neSampler->Sample(prevNode->Previous(),
						   prevNode, 
						   &lightNode, x_1, x_2,
						   true , BSDF_ALL_COMPONENTS))
	{
#ifdef RTDEBUG
	  if(config->debugInfo)
	  {
		VECTOR dir;
		geom = PathNodeConnect(prevNode, &lightNode, &config->samplerConfig, 
				       NULL, // No light config
				       CONNECT_EL, BSDF_ALL_COMPONENTS, 
				       BSDF_ALL_COMPONENTS, &dir);
		prevNode->Attach(&lightNode);
		lightNode.m_inDirT = dir;
		prevNode->Print(stderr);
		lightNode.Print(stderr);
		prevNode->Detach(&lightNode);
	  }
#endif

      	  if(PathNodesVisible(prevNode, &lightNode))
	  {
	    // Now connect for all applicable scatter-info's
	    // If no weighting between reflection sampling and
	    // next event estimation were used, only one connect
	    // using the union of different scatter info flags
	    // are necessary (=speedup).

#ifdef RTBEBUG
	    if(config->debugInfo)
	    {
	      fprintf(stderr, "Visible\n");
	    }
#endif
	    
	    int siCurrent;
	    CScatterInfo *si;
	    
	    if((config->siStorage.flags != NO_COMPONENTS) &&
	       (readout == SCATTER))
	    {
	      // Do storage components
	      si = &config->siStorage;
	      siCurrent = -1;
	    }
	    else
	    {
	      // No direct light using storage components
	      si = &config->siOthers[0];
	      siCurrent = 0;
	    }

	    while(siCurrent < config->siOthersCount)
	    {
	      bool doSi = true;

	      if((config->reflectionSampling == PHOTONMAPSAMPLING) 
		 // was (config->radMode == STORED_PHOTONMAP)
		 || (config->reflectionSampling == CLASSICALSAMPLING))
	      {
		if(si->flags & BSDF_SPECULAR_COMPONENT)
		  doSi = false; // Perfect mirror reflection, no n.e.e.
	      }
	      
	      if(doSi)
	      {
		// Connect using correct flags
		geom = PathNodeConnect(prevNode, &lightNode, &config->samplerConfig, 
				       NULL, // No light config
				       CONNECT_EL, si->flags, 
				       BSDF_ALL_COMPONENTS,
				       &dirEL);
		
		// Contribution of this sample (with Multiple Imp. S.)
		
		if(config->reflectionSampling == CLASSICALSAMPLING)
		{
		  weight = 1.0;
		}
		else
		{
		  // Ndirect * pdf  for the n.e.e.
		  cl = MISFUNC(config->nextEventSamples * lightNode.m_pdfFromPrev);
		  
		  // Nscatter * pdf  for possible scattering
		  if(si->DoneSomePreviousBounce(prevNode))
		    nrs = si->nrSamplesAfter;
		  else
		    nrs = si->nrSamplesBefore;
		  
		  cr = MISFUNC(nrs * lightNode.m_pdfFromNext);
		  
		  // Are we deep enough to do russian roulette
		  if(lightNode.m_depth >= config->samplerConfig.minDepth)
		    cr *= MISFUNC(lightNode.m_rrPdfFromNext);
		  
		  weight = cl / (cl + cr);
		}
	    
		// printf("cl %g : %i %g, cr %g : %i %g\n", cl, 
		//  config->nextEventSamples, lightNode.m_pdfFromPrev,
		//  cr, config->scatterSamples, lightNode.m_pdfFromNext);

		factor = weight * geom / (lightNode.m_pdfFromPrev * 
					  config->nextEventSamples);
		COLORPRODSCALED(prevNode->m_bsdfEval, factor, lightNode.m_bsdfEval,
				radiance);
		
		// Collect outgoing radiance
		COLORADD(result, radiance, result);
	      } // if not photonmap or no caustic path

	      // Next scatter info block
	      siCurrent++;
	      if(siCurrent < config->siOthersCount)
		si = &config->siOthers[siCurrent];

	    } // while not all si done
	  } // if pathnodes visible
	} // for each sample
      } // if light point sampled

      if(config->lightMode == ALL_LIGHTS)
	lightsToDo = nes->ActivateNextUnit();
      else
	lightsToDo = false;

    } // while(!lightsDone)
  }
  return result;
}


COLOR SR_GetRadiance(CPathNode *thisNode, SRCONFIG *config, SRREADOUT readout,
		     int usedScatterSamples)
{
  COLOR result, radiance;
  //  BSDFFLAGS bsdfFlags = BSDF_ALL_COMPONENTS;
  XXDFFLAGS edfFlags = ALL_COMPONENTS;

  // handle background
  if(thisNode->m_rayType == Environment)
    {
      // check for  weighting
      double weight = 1, cr, cl;
	  
      bool doWeight = true;
	  
      if(thisNode->m_depth <= 1)   // don't weight direct light
	doWeight = false;
	  
      if(config->reflectionSampling == CLASSICALSAMPLING)
	doWeight = false;

      
      
      

      if(!(config->backgroundSampling)) doWeight = false;
      
      if(doWeight)
	{
	  cl = config->nextEventSamples *
	    config->samplerConfig.neSampler->EvalPDF(thisNode->Previous(), 
						     thisNode);
	  cl = MISFUNC(cl);
	  cr = usedScatterSamples * thisNode->m_pdfFromPrev;
	  cr = MISFUNC(cr);
	  
	  weight = cr / (cr + cl);
	}
      
      result = BackgroundRadiance(Background, &(thisNode->Previous()->m_hit.point), &(thisNode->m_inDirF), NULL);
      
      COLORSCALE(weight, result, result);
  }
  // handle non-background
  else 
  {
    EDF *thisEdf = thisNode->m_hit.material->edf;

    COLORCLEAR(result);

    // Stored radiance

    if((readout == READ_NOW) && (config->siStorage.flags != NO_COMPONENTS))
    {
      

      if(Radiance == &Pmap)
      {
	if(config->radMode == STORED_PHOTONMAP)
	{
	  // Check if the distance to the previous point is big enough
	  // otherwise we need more scattering...
	  
	  float dist2;
	  VECTORDIST2(thisNode->m_hit.point, thisNode->Previous()->m_hit.point, dist2);
	  
	  if(dist2 > PMAP_MIN_DIST2)
	  {
	    radiance = GetPmapNodeGRadiance(thisNode);
	    // This does not include Le (self emitted light)
	  }
	  else
	  {
	    COLORCLEAR(radiance);
	    readout = SCATTER; // This ensures extra scattering, direct light and c-map
	  }
	}
	else
	{
	  radiance = GetPmapNodeGRadiance(thisNode);
	  // This does not include Le (self emitted light)
	}
      }
      else
      {
	// Other radiosity method
	
	double u,v;
	
	
	PatchUV(thisNode->m_hit.patch, 
		&thisNode->m_hit.point, &u, &v);
	
	radiance = Radiance->GetRadiance(thisNode->m_hit.patch, u, v, 
					 thisNode->m_inDirF);
	
	// This includes Le diffuse, subtract first and
	// handle total emitted later (possibly weighted)
	// -- Interface mechanism needed to determine what a
	// -- radiance method does...
	
	COLOR diffEmit;
	
	diffEmit = EdfEval(thisEdf, &thisNode->m_hit, &(thisNode->m_inDirF),
			   BRDF_DIFFUSE_COMPONENT, (double *)0);
	
	COLORSUBTRACT(radiance, diffEmit, radiance);
      }
      
      COLORADD(result, radiance, result);
      
    } // Done: Stored radiance, no self emitted light included!
    
    // Stored caustic maps
    
    if((config->radMode == STORED_PHOTONMAP) && readout == SCATTER)
    {
      radiance = GetPmapNodeCRadiance(thisNode);
      COLORADD(result, radiance, result);
    }
    
    radiance = SR_GetDirectRadiance(thisNode, config, readout);
    
    COLORADD(result, radiance, result);
    
    // Scattered light
    
    radiance = SR_GetScatteredRadiance(thisNode, config, readout);
    
    COLORADD(result, radiance, result);
    
    // Emitted Light
    
    if((config->radMode == STORED_PHOTONMAP) && (Radiance == &Pmap))
    {
      // Check if Le would contribute to a caustic
      
      if((readout == READ_NOW) && !(config->siStorage.DoneThisBounce(thisNode->Previous())))
      {
	// Caustic contribution:  (E...(D|G)...?L) with ? some specular bounce
	edfFlags = 0;
      }
    }
    
    if((thisEdf != NULL) && (edfFlags != 0))
    {
      double weight, cr, cl;
      COLOR col;
      
      bool doWeight = true;
      
      if(thisNode->m_depth <= 1)
	doWeight = false;
      
      if(config->reflectionSampling == CLASSICALSAMPLING)
	doWeight = false;
      
      if((config->reflectionSampling == PHOTONMAPSAMPLING)
	  && (thisNode->m_depth > 1))
      {
	if(thisNode->Previous()->m_usedComponents & BSDF_SPECULAR_COMPONENT)
	  doWeight = false;  // Perfect Specular scatter, no weighting
      }
      
      if(doWeight)
      {
	cl = config->nextEventSamples *
	  config->samplerConfig.neSampler->EvalPDF(thisNode->Previous(), 
						   thisNode);
	cl = MISFUNC(cl);
	cr = usedScatterSamples * thisNode->m_pdfFromPrev;
	cr = MISFUNC(cr);

	weight = cr / (cr + cl);
      }
      else
      {
	weight = 1;  // We don't do N.E.E. from the eye !
      }

      col = EdfEval(thisEdf, &thisNode->m_hit,
		    &(thisNode->m_inDirF),
		    edfFlags, (double *)0);

      COLORADDSCALED(result, weight, col, result);
    }  
  }
  
  return result;
}

static COLOR CalcPixel(int nx, int ny, SRCONFIG *config)
{
  int i;
  CPathNode eyeNode, pixelNode;
  double x_1, x_2;
  COLOR col, result;
  CStrat2D strat(config->samplesPerPixel);

  COLORCLEAR(result);

  // Frame coherent & correlated sampling
  if(rts.doFrameCoherent || rts.doCorrelatedSampling)
  {
    if(rts.doCorrelatedSampling)
    {
      // Correlated : start each pixel with same seed
      srand48(rts.baseSeed);
    }
    drand48(); // (randomize seed, gives new seed for uncorrelated sampling)
    config->seedConfig.Save(0);    
  }

  // Calc pixel data

  //  printf("Pix %i %i : ", nx, ny);

  // Sample eye node

  config->samplerConfig.pointSampler->Sample(NULL, NULL, &eyeNode, 0, 0);
  ((CPixelSampler *)config->samplerConfig.dirSampler)->SetPixel(nx, ny);


  eyeNode.Attach(&pixelNode);

  // Stratified sampling of the pixel

  for(i = 0; i < config->samplesPerPixel; i++)
  {
    strat.Sample(&x_1, &x_2);

    if(config->samplerConfig.dirSampler->Sample(NULL, &eyeNode, 
						  &pixelNode, x_1,
						  x_2)
       && ((pixelNode.m_rayType != Environment) || (config->backgroundDirect)))
    {
      pixelNode.AssignBsdfAndNormal();

      // Frame coherent & correlated sampling
      if(rts.doFrameCoherent || rts.doCorrelatedSampling)
      {
	config->seedConfig.Save(pixelNode.m_depth);    
      }


      col = SR_GetRadiance(&pixelNode, config, config->initialReadout, 
			   config->samplesPerPixel);

      // Frame coherent & correlated sampling
      if(rts.doFrameCoherent || rts.doCorrelatedSampling)
      {
	config->seedConfig.Restore(pixelNode.m_depth);    
      }

      // col represents the radiance reflected towards the eye
      // in the pixel sampled point.

      // Account for the eye sampling 
      // -- Not neccessary yet ...

      // Account for pixel sampling
      COLORSCALE(pixelNode.m_G / pixelNode.m_pdfFromPrev, col, col);
      COLORADD(result, col, result);
    }
  }
    
  // We have now the FLUX for the pixel (x N), convert it to radiance
  
  double factor = (ComputeFluxToRadFactor(nx, ny) / 
		   config->samplesPerPixel);

  COLORSCALE(factor, result, result);

  // printf("RESULT %g %g %g \n", result.r, result.g, result.b);

  config->screen->Add(nx, ny, result);


  // Frame coherent & correlated sampling
  if(rts.doFrameCoherent || rts.doCorrelatedSampling)
  {
    config->seedConfig.Restore(0);    
  }

    
  return result;
}





void RTStochastic_DebugPixel(int nx, int ny)
{
  SRCONFIG config(rts);

  // -- Maybe this whole function should be ifdef'd
#ifdef RTDEBUG
  config.debugInfo = true;
  config.samplerConfig.dirSampler->SetDebugOut(stderr);
#endif

  CalcPixel(nx, ny, &config);
}

COLOR RTStochastic_GetPixel(int nx, int ny)
{
  return rts.lastscreen->Get(nx, ny);
} 



void RTStochastic_Trace(ImageOutputHandle *ip)
{
  SRCONFIG config(rts); // config filled in by constructor

  // Frame Coherent sampling : init fixed seed
  if(rts.doFrameCoherent)
  {
    srand48(rts.baseSeed);
  }

  CPathNode::m_dmaxsize = 0; // No need for derivative structures

  if(!rts.progressiveTracing)
  {
    ScreenIterateSequential((COLOR(*)(int,int,void *))CalcPixel, &config);
  }
  else
  {
    ScreenIterateProgressive((COLOR(*)(int,int,void *))CalcPixel, &config);
  }

  config.screen->Render();

  if (ip) config.screen->WriteFile(ip);

  if (rts.lastscreen)
    delete rts.lastscreen;
  rts.lastscreen = config.screen;
  config.screen = NULL;
}

void RTStochastic_RecomputeDisplayColors(void)
{
  if (rts.lastscreen)
    rts.lastscreen->Sync();
}

double RTStochastic_AdaptationLuminance(float scale)
{
  if (rts.lastscreen) {
    return rts.lastscreen->AdaptationLuminance(scale);
  } else
    return 1.0;
}

int RTStochastic_Redisplay(void)
{
  if (rts.lastscreen) {
    rts.lastscreen->Render();
    return true;
  } else
    return false;
}

static int RTStochastic_Reproject(void)
{
  if (rts.lastscreen) {
    // bidir.lastscreen->Sync();
    rts.lastscreen->Reproject();
    return true;
  } else
    return false;
}

int RTStochastic_SaveImage(ImageOutputHandle *ip)
{
  if (ip && rts.lastscreen) {
    rts.lastscreen->Sync();
    rts.lastscreen->WriteFile(ip);
    return true;
  } else
    return false;
}

void RTStochastic_Interrupt(void)
{
  interrupt_raytracing = TRUE;
}

void RTStochastic_Init(void)
{
  // Init the light list
  if(gLightList) delete gLightList;
  gLightList = new CLightList(LightSourcePatches);
}  

void RTStochastic_Terminate(void)
{
  if (rts.lastscreen)
    delete rts.lastscreen;
  rts.lastscreen = (CScreenBuffer *)0;
}

RAYTRACINGMETHOD RT_StochasticMethod =
{
  "StochasticRaytracing", 4,
  "Stochastic Raytracing & Final Gathers",
  "rtmStochasticRaytracingButton",
  RTStochasticDefaults,
  CreateRTStochasticControlPanel,
  RTStochasticParseOptions,
  RTStochasticPrintOptions,
  RTStochastic_Init,
  RTStochastic_Trace,
  RTStochastic_AdaptationLuminance,
  RTStochastic_RecomputeDisplayColors,
  RTStochastic_Redisplay,
  RTStochastic_Reproject,
  RTStochastic_SaveImage,
  RTStochastic_Interrupt,
  RTStochastic_ShowControlPanel,
  RTStochastic_HideControlPanel,
  RTStochastic_Terminate
};

