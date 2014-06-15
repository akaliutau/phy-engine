

#include "photonmap.H"
#include "statistics.h"

#include "error.h"



bool ZeroAlbedo(BSDF *bsdf, HITREC *hit, BSDFFLAGS flags)
{
  COLOR col;

  col = BsdfScatteredPower(bsdf, hit, &hit->gnormal, flags); 
  
  return (COLORAVERAGE(col) < EPSILON);
}


float GetFalseMonochrome(float val)
{
  float tmp; // r=0,g=0,b=0;

  float max = pmapstate.falseColMax;

  if(pmapstate.falseColLog)
  {
    max = log(1.0 + max);    
    val = log(1.0 + val);
  }

  //printf("log 10 = %g\n", log(10));

  tmp = MIN(val,max);
  tmp = (tmp/max);

  return tmp;
}


COLOR GetFalseColor(float val)
{
  RGB rgb;
  COLOR col;
  float max, tmp, r=0,g=0,b=0;

  if(pmapstate.falseColMono)
  {
    tmp = GetFalseMonochrome(val);
    RGBSET(rgb, tmp, tmp, tmp);
    RGBToColor(rgb, &col);
    return col;
  }

  max = pmapstate.falseColMax;

  if(pmapstate.falseColLog)
  {
    max = log(1.0 + max);
    val = log(1.0 + val);
  }

  //printf("log 10 = %g\n", log(10));

  tmp = MIN(val,max);

  // Do some log scale ?

  tmp = 3.0 * (tmp/max);

  if(tmp <= 1.0)
  {
    b = tmp;
  }
  else if(tmp < 2.0)
  {
    g = tmp - 1.0;
    b = 1.0 - g;
  }
  else
  {
    r = tmp - 2.0;
    g = 1.0 - r;
  }

  RGBSET(rgb, r, g, b);
  RGBToColor(rgb, &col);

  return col;
}




// Constructor

CPhotonMap::CPhotonMap(int *estimate_nrp, bool doPrecomputeIrradiance)
{
  m_balanced = true;
  m_doBalancing = false;
  
  m_estimate_nrp = estimate_nrp;

  m_precomputeIrradiance = doPrecomputeIrradiance;
  m_irradianceComputed = false;

  if(doPrecomputeIrradiance)
  {
    //fprintf(stderr, "Irradiance photon kdtree\n");
    m_kdtree = new CPhotonkdtree(3, sizeof(CIrrPhoton), true); // Data is copied
  }
  else
  {
    //fprintf(stderr, "Normal photon kdtree\n");
    m_kdtree = new CPhotonkdtree(3, sizeof(CPhoton), true); // Data is copied
  }

  m_totalPaths = 0;
  m_nrPhotons = 0;
  m_totalPhotons = 0;

  m_grid = new CSampleGrid2D(2,4);
  VECTORSET(m_sampleLastPos, HUGE, HUGE, HUGE);

  m_photons = new CPhoton*[MAXRECONPHOTONS];
  m_distances = new float[MAXRECONPHOTONS];
  m_cosines = new float[MAXRECONPHOTONS];

  m_nrpFound = 0;  // no valid photons in array
  m_cosinesOk = true;
}


// Destructor

CPhotonMap::~CPhotonMap(void)
{
  delete m_kdtree;
  delete[] m_photons;
  delete[] m_distances;
  delete[] m_cosines;
}

// Initialize

void CPhotonMap::Init(void)
{
  // ???
}



void CPhotonMap::ComputeCosines(VECTOR &normal)
{
  VECTOR dir;
  
  if(!m_cosinesOk)
  {
    m_nrpCosinePos = 0;
    
    for(int i = 0; i < m_nrpFound; i++)
    {
      dir = m_photons[i]->Dir();
      m_cosines[i] = VECTORDOTPRODUCT(dir, normal);
      if(m_cosines[i] > 0) m_nrpCosinePos++;
    }
    
    m_cosinesOk = true;
  }
}


// Adding photons

void CPhotonMap::DoAddPhoton(CPhoton &photon, VECTOR &normal,
			     short flags)
{
  if(m_precomputeIrradiance)
  {
    CIrrPhoton irrPhoton;
    irrPhoton.Copy(photon);
    irrPhoton.SetNormal(normal);
    m_kdtree->AddPoint(&irrPhoton, flags);
  }
  else
  {
    m_kdtree->AddPoint(&photon, flags);
  }
}

bool CPhotonMap::AddPhoton(CPhoton &photon, VECTOR &normal,
			   short flags)
{
  drand48(); // Just to keep in sync with density controlled storage

  DoAddPhoton(photon, normal, flags);
  //m_kdtree->AddPoint(&photon, flags);
  m_nrPhotons++;
  m_totalPhotons++;
  m_balanced = false;
  m_irradianceComputed = false;

  return true;
}


double ComputeAcceptProb(float currentD, float requiredD)
{
  // Step function

  if(pmapstate.acceptPdfType == STEP)
  {
    if(currentD > requiredD) return 0.0;
    else return 1.0;
  }
  else if(pmapstate.acceptPdfType == TRANSCOSINE)
  {
    // Translated cosine
    double ratio = MIN(1.0, currentD / requiredD); // in [0,1]

    return (0.5 * (1.0 + cos(ratio * M_PI)));
  }
  else
  {
    Error("ComputeAcceptProb", "Unknown accept pdf type");
    return 0.0;
  }
}

void CPhotonMap::Redistribute(CPhoton &photon, float acceptProb, short flags)
{
  // Redistribute this photon over the nearest neighbours
  // m_distances, m_photons and m_cosines should be filled correctly!
  // only photons are used for which direction * normal > 0

  // -- Check the flags
  // -- normal weighted average?

  COLOR deltaPower;
  COLOR pow;
  float factor = 1.0 / m_nrpCosinePos;

  pow = photon.Power();
  COLORSCALE(factor, pow, deltaPower);

  for(int i = 0; i < m_nrpFound; i++)
  {
    if(m_cosines[i] > 0.0)
    {
      m_photons[i]->AddPower(deltaPower);
    }
  }
}


bool CPhotonMap::DC_AddPhoton(CPhoton &photon, HITREC &hit,
			      float requiredD, short flags)
{
  // Get current density
  // VECTOR pos = photon.Pos();
  bool stored;

  float currentD = GetCurrentDensity(hit, pmapstate.distribPhotons);
  // m_photons and m_distances is valid now !!


  // Compute acceptance probability

  double acceptProb = ComputeAcceptProb(currentD, requiredD);

  // printf("A prob %g, CD %g RD %g\n", acceptProb, currentD, requiredD);

  // Roulette

  if(drand48() < acceptProb)
  {
    // Store
    //photon.SetDCWeight(1.0 / acceptProb);
    DoAddPhoton(photon, hit.normal, flags);
    //m_kdtree->AddPoint(&photon, flags);
    m_nrPhotons++;
    m_balanced = false;
    m_irradianceComputed = false;
    stored = true;
  }
  else
  {
    // Redistribute power over neighbours or ignore
    stored = false;
    Redistribute(photon, acceptProb, flags);
  }

  m_totalPhotons++; // All photons including non stored photons

  return stored;
}


// End adding photons

// void CPhotonMap::StopAddingMode(void)
// {
// m_addingMode = false;
// 
// // Balance tree...
// }





double CPhotonMap::GetMaxR2(void)
{
  
  const double radfraction = 0.03;

  double maxr2 = (*m_estimate_nrp * total_area /
		 (M_PI * m_totalPaths * radfraction));

  return maxr2;
}

// Precompute Irradiance

void CPhotonMap::PhotonPrecomputeIrradiance(CIrrPhoton *photon)
{
  COLOR irradiance, power;
  COLORCLEAR(irradiance);

  // locate nearest photons using a max radius limit

  VECTOR pos = photon->Pos();
  m_nrpFound = DoQuery(&pos);

  if(m_nrpFound > 3)
  {
    // Construct irradiance estimate
    float maxDistance = m_distances[0];

    for(int i = 0; i < m_nrpFound; i++)
    {
      if((photon->Normal() & m_photons[i]->Dir()) > 0)
      {
	power = m_photons[i]->Power();
	COLORADD(irradiance, power, irradiance);
      }
    }

    // Now we have incoming radiance integrated over area estimate, 
    // so we convert it to irradiance, maxDistance is already squared
    // An extra factor PI is added, that accounts for Albedo -> diffuse brdf...
    float factor = 1.0 / (M_PI * M_PI * maxDistance * m_totalPaths);
    COLORSCALE(factor, irradiance, irradiance);
  }

  photon->SetIrradiance(irradiance);
}

static void PrecomputeIrradianceCallback(CPhotonMap *map, CIrrPhoton *photon)
{
  map->PhotonPrecomputeIrradiance(photon);
}

void CPhotonMap::PrecomputeIrradiance(void)
{
  fprintf(stderr, "CPhotonMap::PrecomputeIrradiance\n");
  if(m_precomputeIrradiance && !m_irradianceComputed)
  {
    m_kdtree->IterateNodes((void (*)(void *, void *))PrecomputeIrradianceCallback,
			   this);
    m_irradianceComputed = true;
  }
}


// Reconstruct

// bool CPhotonMap::IrradianceReconstruct(HITREC *hit, VECTOR &outDir, 
// BSDF *bsdf, BSDF *inBsdf,
// BSDF *outBsdf,
// COLOR *result)
bool CPhotonMap::IrradianceReconstruct(HITREC *hit, VECTOR &outDir, 
			   COLOR& diffuseAlbedo,
			   COLOR *result)
{
  if(!m_irradianceComputed)
    PrecomputeIrradiance();

  CIrrPhoton *photon = DoIrradianceQuery(&hit->point, &hit->normal);

  if(photon)
  {
    //float factor = 1.0 / (M_PI * m_totalPaths);
    //COLORPRODSCALED(photon->m_irradiance, factor, diffuseAlbedo, *result);

    COLORPROD(photon->m_irradiance, diffuseAlbedo, *result);

    // COLOR eval = BsdfEval(bsdf, hit, inBsdf, outBsdf, &outDir, 
    // &hit->normal, BRDF_DIFFUSE_COMPONENT);
    // COLORPROD(photon->m_irradiance, eval, *result);
    return true;
  }
  else
  {
    // No appropriate photon found
    return false;
  }
}

COLOR CPhotonMap::Reconstruct(HITREC *hit, VECTOR &outDir, 
			      BSDF *bsdf, BSDF* inBsdf, BSDF *outBsdf)
{
  // Find nearest photons

  float maxDistance = 0.0;
  COLOR result, eval, power, col;
  float factor;

  COLORCLEAR(result);

  COLOR diffuseAlbedo, glossyAlbedo;

  diffuseAlbedo = BsdfScatteredPower(bsdf, hit, &hit->gnormal, BRDF_DIFFUSE_COMPONENT); 
  // -- TODO Irradiance precomputation for diffuse transmission
  glossyAlbedo = BsdfScatteredPower(bsdf, hit, &hit->gnormal, BTDF_DIFFUSE_COMPONENT | BSDF_GLOSSY_COMPONENT); 
  
  //if(ZeroAlbedo(bsdf, hit, BSDF_DIFFUSE_COMPONENT | BSDF_GLOSSY_COMPONENT))
  //  return result;

  CheckNBalance();

  if(COLORAVERAGE(glossyAlbedo) < EPSILON)
  {
    if(COLORAVERAGE(diffuseAlbedo) < EPSILON)
      return result; // No reflectance
    else
    {
      if(m_precomputeIrradiance)
      {
	// if(IrradianceReconstruct(hit, outDir, bsdf, inBsdf, outBsdf, &result))
	if(IrradianceReconstruct(hit, outDir, diffuseAlbedo, &result))
	  return result;
	else // no appropriate irradiance photon -> do normal reconstruction
	  { 
	    // fprintf(stderr, "No irradiance photon found\n");
	  }
      }
    }
  }

  // Normal reconstruct...

  // locate nearest photons using a max radius limit
  m_nrpFound = DoQuery(&hit->point);

  if(m_nrpFound < 3) return result;

  // Construct radiance estimate

  maxDistance = m_distances[0];

  for(int i = 0; i < m_nrpFound; i++)
  {
    VECTOR dir = m_photons[i]->Dir();
    eval = BsdfEval(bsdf, hit, inBsdf, outBsdf, &outDir, 
		    &dir,
		    BSDF_DIFFUSE_COMPONENT|BSDF_GLOSSY_COMPONENT);
    power = m_photons[i]->Power();

    COLORPROD(eval, power, col);
    COLORADD(result, col, result);
  }

  // Now we have a radiance integrated over area estimate, 
  // so we convert it to radiance, maxDistance is already squared

  factor = 1.0 / (M_PI * maxDistance * m_totalPaths);

  COLORSCALE(factor, result, result);


#ifdef PMAP_DEBUG
  printf("\nReconstruct: NRP %i MAXD %f\n", m_nrpFound, maxDistance);
  printf("POS: "); VectorPrint(stdout, pos); printf("\n");
  printf("OUT: "); VectorPrint(stdout, outDir); printf("\n");
  printf("N: "); VectorPrint(stdout, normal); printf("\n");

  printf("RES: "); ColorPrint(stdout, result); printf("\n");
  printf("-------------------------------------------------\n");
#endif

  return result;
}

float CPhotonMap::GetCurrentDensity(HITREC &hit, int nrPhotons)
{
  // Find nearest photons

  if(nrPhotons == 0) nrPhotons=*m_estimate_nrp;

  float maxDistance = 0.0;

  if(nrPhotons == 0) return 0.0;

  m_nrpFound = DoQuery(&hit.point, nrPhotons, GetMaxR2());

  if(m_nrpFound < 3) return 0.0;

  // Construct density estimate
  maxDistance = m_distances[0]; // Only valid since max heap is used in kdtree

  ComputeCosines(hit.gnormal); // -- shading normal ?

  if(m_nrpCosinePos <= 3) return 0.0;

  return(m_nrpCosinePos / (M_PI * maxDistance));
}




// Return a color coded density of the photonmap
COLOR CPhotonMap::GetDensityColor(HITREC &hit)
{
  float density;
  COLOR result;

  density = GetCurrentDensity(hit);

  result = GetFalseColor(density);

  return result;
}



double CPhotonMap::Sample(VECTOR &pos, double *r, double *s, 
			  COORDSYS *coord,
			  BSDFFLAGS flag, float n)
{
  COLOR col;

  //  printf("Sample\n");

  // PrintCoordSys(stdout, coord);

  // -- Epsilon in as a function of scene/camera measure ??
  if(!VECTOREQUAL(m_sampleLastPos, pos, 0.0001))
  {
    // Need a new grid

    m_grid->Init();

    // Find nearest photons

    m_nrpFound = DoQuery(&pos, m_sample_nrp, KDMAXRADIUS, NO_IMPSAMP_PHOTON);

    //printf("m_nrpFound %i\n", m_nrpFound);

    double pr,ps;
    
    for(int i = 0; i < m_nrpFound; i++)
    {
      m_photons[i]->FindRS(&pr, &ps, coord, flag, n);
      
      col = m_photons[i]->Power();

      //printf("Photon %2i, D: ", i); VectorPrint(stdout, m_photons[i]->Dir()); 
      //printf(" P: "); VectorPrint(stdout, m_photons[i]->Pos()); 
      //printf("\n");
      //printf("Ph %i, pr %g, ps %g, pow %g\n", i, pr, ps, 
      //	     COLORAVERAGE(col)/m_nrPhotons);

      m_grid->Add(pr, ps, COLORAVERAGE(col)/m_nrPhotons);
    }
    
    m_grid->EnsureNonZeroEntries();
    m_sampleLastPos = pos; // Caching
  }

  // Sample

  double pdf;
  m_grid->Sample(r,s, &pdf);

  return pdf;
  //  printf("CPhotonMap::Sample nrp %i r %g s %g\n", m_nrpFound, *r, *s);
}



#include "hemirenderer.H"

float CPhotonMap::GetGridValue(double phi, double theta)
{
  double s = phi / (2 * M_PI);
  double tmp = cos(theta);
  double r = - tmp*tmp + 1;
  
  return m_grid->GetValue(r,s);
}

static void CPhotonMapGetGrid(VECTOR *pos, COORDSYS *coord, 
			      double phi, double theta, 
			      void *map, RGB *color, 
			      double *distance)
{
  CPhotonMap *pm = (CPhotonMap *)map;

  COLOR colg,colr,colb;

  RGBToColor(Green, &colg);
  RGBToColor(Red, &colr);
  RGBToColor(Blue, &colb);

  COLOR dest=colb;

  COLORADDSCALED(colb, phi / (2*M_PI), colr, dest);
  COLORADDSCALED(dest, theta / (M_PI), colg, dest);

  ColorToRGB(dest, color);
  *distance = 0.1 * pm->GetGridValue(phi, theta);
}

double CPhotonMap::DebugSample(VECTOR &pos, double *r, double *s, COORDSYS *coord,
	    BSDFFLAGS flag, float n)
{
  double pdf = Sample(pos, r, s, coord, flag, n);

  // Now m_grid has the correct data
  m_grid->Print();
  
  CHemisphereRenderer *hemi = new CHemisphereRenderer;

  hemi->Initialize(32,8, pos, coord, CPhotonMapGetGrid, (void *)this);
  hemi->EnableRendering(true);

  return pdf;
}
