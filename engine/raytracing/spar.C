

#include <stdio.h>

#include <spar.H>

#include <statistics.h>
#include <error.h>
#include <patch.h>

#define ZEROEPSILON 1e-12
#define MPMISFUNC(Ci) (Ci)

// Utility routines



static void StripLightNode(CBiPath *path, CBiPath *newPath)
{
  // Copy first. No deep copy so the new path should not be deleted
  *newPath = *path;

  // Now remove node
  if(newPath->m_lightSize == 0)
  {
    newPath->m_eyeSize -= 1; // Last node from an eyepath
  }
  else if(newPath->m_lightSize == 1)
  {
    newPath->m_lightSize = 0;
    newPath->m_lightPath = newPath->m_lightEndNode = NULL;
  }
  else
  {
    newPath->m_lightSize -= 1;
    newPath->m_lightPath = newPath->m_lightPath->Next();
  }
}




COLOR CSparList::HandlePath(CSparConfig *sconfig,
			    CBiPath *path)
{
  CSparListIter iter(*this);
  CSpar **pspar;
  COLOR total, col;

  COLORCLEAR(total);

  while((pspar = iter.Next()))
  {
    col = (*pspar)->HandlePath(sconfig, path);
    COLORADD(col, total, total);
  }

  return total;
}

void CSparList::HandlePath(CSparConfig *sconfig,
			    CBiPath *path, COLOR *frad, COLOR *fbpt)
{
  CSparListIter iter(*this);
  CSpar **pspar;
  COLOR col;

  COLORCLEAR(*fbpt);
  COLORCLEAR(*frad);

  while((pspar = iter.Next()))
  {
    col = (*pspar)->HandlePath(sconfig, path);

    if(*pspar == sconfig->m_leSpar)
    {
      COLORADD(col, *fbpt, *fbpt);
    }
    else
    {
      COLORADD(col, *frad, *frad);
    }
  }
}




CSpar::CSpar(void)
{
  m_contrib = new CContribHandler[MAXPATHGROUPS];
  m_sparList = new CSparList[MAXPATHGROUPS];  
}

void CSpar::Init(CSparConfig *sconfig)
{
  int i;

  for(i = 0; i < MAXPATHGROUPS; i++)
  {
    m_contrib[i].Init(sconfig->m_bcfg->maximumPathDepth);
    m_sparList[i].RemoveAll();
  }
}


void CSpar::ParseAndInit(int group, char *regExp)
{
  int beginPos = 0, endPos=0;
  char tmpChar;

  while(regExp[endPos] != '\0')
  {
    if(regExp[endPos] == ',')
    {
      // Next RegExp

      tmpChar = regExp[endPos];
      regExp[endPos] = '\0';

      m_contrib[group].AddRegExp(regExp + beginPos);

      regExp[endPos] = tmpChar; // Restore
      beginPos = endPos + 1; // Begin next regexp
    }

    endPos++;
  }

  // Still parse last regexp in list

  if(beginPos != endPos)
  {
    m_contrib[group].AddRegExp(regExp + beginPos);
  }    
}

CSpar::~CSpar(void)
{
  // printf("Del Spar\n");

  delete[] m_contrib;
  delete[] m_sparList;
}

COLOR CSpar::HandlePath(CSparConfig *,	CBiPath *)
{
  COLOR result;

  COLORCLEAR(result);

  return result;
}

COLOR CSpar::EvalFunction(CContribHandler *contrib,
			  CBiPath *path)
{
  COLOR col;
  CBsdfComp oldComp;
  CPathNode *readoutNode;


  // First readout is filled in in the path

  if(path->m_lightSize == 0)
  {
    readoutNode = path->m_eyeEndNode;
  }
  else
  {
    readoutNode = path->m_lightPath;
  }

  oldComp = readoutNode->m_bsdfComp;

  GetStoredRadiance(readoutNode);

  col = contrib->Compute(path);
  
  // Restore 

  readoutNode->m_bsdfComp = oldComp;

  return col;
}

double CSpar::EvalPDFAndWeight(CSparConfig *, CBiPath *path)
{
  // Standard bipath pdf evaluation

  double pdf = path->EvalPDFAcc();

  if(path->m_lightSize == 1)
  {
    // Account for the importance sampling of the light point
    pdf = pdf * path->m_pdfLNE / path->m_lightEndNode->m_pdfFromPrev;
  }

  return 1.0 / pdf; // Weight = 1;


  
}

double CSpar::ComputeWeightTerms(TPathGroupID , CSparConfig *,
				 CBiPath *, double *sum)
{
  *sum = 0.0;
  return 0.0;
}



bool CSpar::IsCoveredID(TPathGroupID)
{
  return false;
}

void CSpar::GetStoredRadiance(CPathNode *node)
{
  node->m_bsdfComp.Clear();
}





void CLeSpar::Init(CSparConfig *sconfig)
{
   CSpar::Init(sconfig);

   // Disjunct path group for BPT

   if(sconfig->m_bcfg->doLe)
   {
     ParseAndInit(DISJUNCTGROUP, sconfig->m_bcfg->leRegExp);
   }

   if(sconfig->m_bcfg->doWeighted)
   {
     ParseAndInit(LDGROUP, sconfig->m_bcfg->wleRegExp);
     m_sparList[LDGROUP].Add(sconfig->m_ldSpar);
   }

   //printf("Le disjunct\n");
   //m_contrib[DISJUNCTGROUP].Print();
   //printf("LDGroup\n");
   //m_contrib[LDGROUP].Print();
}


COLOR CLeSpar::HandlePath(CSparConfig *sconfig,
			   CBiPath *path)
{
  COLOR result, col;
  double totalGeom = path->ComputeTotalGeomFactor();
  double wp;

  COLORCLEAR(result);

  if(!sconfig->m_bcfg->doLe && !sconfig->m_bcfg->doWeighted)
    return(result);
  
  // DISJUNCT GROUP

  if(sconfig->m_bcfg->doLe)
  {
    col = EvalFunction(&m_contrib[DISJUNCTGROUP], path);
  
    if(COLORAVERAGE(col) > ZEROEPSILON)
    {
      wp = EvalPDFAndWeight(sconfig, path);
      
      COLORADDSCALED(result, wp * totalGeom, col, result);
    }
  }


  // OVERLAP GROUP

  if(sconfig->m_bcfg->doWeighted && sconfig->m_bcfg->doWLe)
  {
    if(path->m_lightSize > 0)
    {
      // No direct hits on a lightsource 
      
      col = EvalFunction(&m_contrib[LDGROUP], path);
      
      if(COLORAVERAGE(col) > ZEROEPSILON)
      {
	wp = EvalPDFAndMPWeight(sconfig, path);
	
	COLORADDSCALED(result, wp * totalGeom, col, result);
      }
    }
  }
   

  return result;
}


static double ComputeCx(CSparConfig *sconfig,
			int currentConnect,
			int nodesTotal,
			double pdf,
			double Se, double Sl)
{
  double Cx, S;
  int Ni;
			
  if(currentConnect == 1)
    Ni = sconfig->m_bcfg->totalSamples;
  else
    Ni = sconfig->m_bcfg->samplesPerPixel;
  if(currentConnect == nodesTotal)
    S = Se;
  else
    S = Sl;
  if(S > ZEROEPSILON)
  {
    Cx = Ni * pdf / S;
  }
  else
  {
    Cx = 0.0; 
  }
  
  return Cx;
}

double CLeSpar::ComputeWeightTerms(TPathGroupID,
				   CSparConfig *sconfig,
				   CBiPath *path, double *sum)
{
  CPathNode *nextNode, *L1, *L2;
  double pdfRR, oldPdfL2, oldPdfRR=0.;
  double *p_pdfL2, *p_pdfRR;

  double currentPdf, pdfAcc, newPdf;
  double Ci, weight, Sl, Se;
  int currentConnect, nodesTotal;
  double L2L1pdf;


  // name L1, L2 nodes in the path and
  // compute the pdf for generating L2, so that it is
  // independent of L0. L2 can be the eye !!

  if(path->m_lightSize >= 1)
  {
    L1 = path->m_lightPath;

    if(path->m_lightSize >= 2)
    {
      L2 = L1->Next(); // L2 is part of lightpath : pdfFromPrev changes
      p_pdfL2 = &L2->m_pdfFromPrev;
      p_pdfRR = NULL;

      oldPdfL2 = *p_pdfL2;

      BsdfEvalPdf(L1->m_useBsdf, &L1->m_hit,
		  L1->m_inBsdf, L1->m_outBsdf,
		  &L1->m_normal, // DUMMY IN
		  &L2->m_inDirT, // away from L1 
		  //		  &L1->m_normal,
		  BRDF_DIFFUSE_COMPONENT,
		  p_pdfL2, &pdfRR);
      if(L1->m_depth >= sconfig->m_bcfg->minimumPathDepth)
	*p_pdfL2 *= pdfRR; // Deep enough for russian roulette
      // Normally depth == 1 and mindepth >= 2, so no rr

      L2L1pdf = L1->m_pdfFromNext; //pdf from eye
      if(path->m_lightSize - L1->m_depth + path->m_eyeSize)
      {
	L2L1pdf *= L1->m_rrPdfFromNext;
      }
    }
    else
    {
      L2 = path->m_eyeEndNode;

      p_pdfL2 = &L2->m_pdfFromNext;
      p_pdfRR = &L2->m_rrPdfFromNext;

      oldPdfL2 = *p_pdfL2;
      oldPdfRR = *p_pdfRR;

      if(L2 == path->m_eyePath)
      {
	L2->m_pdfFromNext = 0.0;
	L2->m_rrPdfFromNext = 0.0;
      }
      else
      {
	BsdfEvalPdf(L1->m_useBsdf, &L1->m_hit,
		    L1->m_inBsdf, L1->m_outBsdf,
		    &L1->m_normal, // DUMMY IN
		    &path->m_dirLE, // away from L1, N.E.E.
		    //		    &L1->m_normal,
		    BRDF_DIFFUSE_COMPONENT,
		    p_pdfL2, p_pdfRR);
	if(L1->m_depth >= sconfig->m_bcfg->minimumPathDepth)
	  *p_pdfL2 *= pdfRR; // Deep enough for russian roulette
	// Normally depth == 1 and mindepth >= 2, so no rr
      }

      L2L1pdf = L1->m_pdfFromNext; //pdf from eye
      if(path->m_lightSize - L1->m_depth + path->m_eyeSize)
      {
	L2L1pdf *= L1->m_rrPdfFromNext;
      }
    }
  }
  else
  {
    L1 = path->m_eyeEndNode;
    L2 = L1->Previous();
    
    p_pdfL2 = &L2->m_pdfFromNext;
    p_pdfRR = &L2->m_rrPdfFromNext;

    oldPdfL2 = *p_pdfL2;
    oldPdfRR = *p_pdfRR;
    
      if(L2 == path->m_eyePath)
      {
	L2->m_pdfFromNext = 0.0;
	L2->m_rrPdfFromNext = 0.0;
      }
      else
      {
	BsdfEvalPdf(L1->m_useBsdf, &L1->m_hit,
		    L1->m_inBsdf, L1->m_outBsdf,
		    &L1->m_normal, // DUMMY IN
		    &L1->m_inDirF, // away from L1, N.E.E.
		    //		    &L1->m_normal,
		    BRDF_DIFFUSE_COMPONENT,
		    p_pdfL2, p_pdfRR);

      }

      L2L1pdf = L1->m_pdfFromPrev; //pdf from eye
  }


  // precompute Sl and Se (S value for light path and eyepath/nee to light)

  double sumAl = COLORAVERAGE(total_emitted_power);
  double u,v;
  COLOR col;

  
  PatchUV(L1->m_hit.patch, 
	  &L1->m_hit.point, &u, &v);

#ifdef WMP_WEIGHTS
  col = GAL_GetDirectRadiance(L1->m_hit.patch, u,v, L1->m_inDirF);
#else
  COLORCLEAR(col);
#endif

  //ColorPrint(stdout, col);

  COLORCLIPPOSITIVE(col, col);

  Sl = sumAl * COLORAVERAGE(col);

#ifdef WMP_WEIGHTS
  col = GAL_GetSeWeight(L1->m_hit.patch, u,v, L1->m_inDirF);
#else
  COLORCLEAR(col);
#endif

  COLORCLIPPOSITIVE(col, col);

  Se = COLORAVERAGE(col) * L2L1pdf;

  //printf("Sl %g, Se %g\n", Sl, Se);

  // pdfAcc for the path

  pdfAcc = path->EvalPDFAcc();

  pdfAcc /= L1->m_pdfFromPrev; // this pdf can be dependent on L0 !

//  printf("pdfAcc %g\n", pdfAcc);

  // now compute the weight of the current path pdf

  currentPdf = pdfAcc;
  currentConnect = path->m_eyeSize;
  nodesTotal = path->m_eyeSize + path->m_lightSize;


  Ci = ComputeCx(sconfig, currentConnect, nodesTotal, currentPdf, 
		 Se, Sl);

  weight = MPMISFUNC(Ci);
  *sum = weight;
 
  // compute other weights (recurrence relation !)

  // To the light

  nextNode = path->m_lightEndNode;

  while(currentConnect < nodesTotal)
  {
    currentConnect++; // Handle next N.E.E.

    if(currentConnect != nodesTotal)
    {
      newPdf = currentPdf * nextNode->m_pdfFromNext / nextNode->m_pdfFromPrev; 
      if( currentConnect >= 
	  sconfig->m_bcfg->minimumPathDepth)
      {
	// At this eye depth, RR is applied
	newPdf *= nextNode->m_rrPdfFromNext;
      }
    }
    else
      newPdf = currentPdf; // Change is in Se <-> Sl

    currentPdf = newPdf;

    Ci = ComputeCx(sconfig, currentConnect, nodesTotal, currentPdf, 
		   Se, Sl);
    *sum += MPMISFUNC(Ci);

    nextNode = nextNode->Previous();
  }


  // To the eye

  nextNode = path->m_eyeEndNode;
  currentConnect = path->m_eyeSize;
  currentPdf = pdfAcc; // Start from actual path pdf

  while(currentConnect > 1)  // N.E.E. to eye (=1) included, direct hit 
    // on eye not !
  {
    currentConnect--;

    if(currentConnect != nodesTotal - 1)
    {
      newPdf = currentPdf * nextNode->m_pdfFromNext / nextNode->m_pdfFromPrev; 
      
      if( nodesTotal - currentConnect >= sconfig->m_bcfg->minimumPathDepth)
      {
	// At this light depth, RR is applied
	newPdf *= nextNode->m_rrPdfFromNext;
      }
    }
    else
    {
      newPdf = currentPdf;
    }

    currentPdf = newPdf;

    Ci = ComputeCx(sconfig, currentConnect, nodesTotal, currentPdf, 
		   Se, Sl);

    *sum += MPMISFUNC(Ci);

    nextNode = nextNode->Previous();
  }

  // printf("W %f P %f\n", weight, realPdf);

  PNAN(weight);
  PNAN(*sum);

  // restore path

  *p_pdfL2 = oldPdfL2;
  if(p_pdfRR)
    *p_pdfRR = oldPdfRR ;

  //  printf("Le W %g, S %g\n", weight, *sum);

  return weight;
}


double CLeSpar::EvalPDFAndMPWeight(CSparConfig *sconfig, 
			  CBiPath *path)
{
  CBiPath ldPath;
  double totalPdf;
  double sum=0.0, partialSum, thisCi;

  // bi path pdf (weight = 1.0 in CSpar)

  totalPdf = 1.0 / CSpar::EvalPDFAndWeight(sconfig, path);

  StripLightNode(path, &ldPath);

  // Ci's for ldPath

  thisCi = ComputeWeightTerms(LDGROUP, sconfig, &ldPath, &partialSum);
  sum += partialSum;

  // sum Ci's from other Spars

  CSparListIter iter(m_sparList[LDGROUP]);
  CSpar **pspar;

  while((pspar = iter.Next()))
  {
    (*pspar)->ComputeWeightTerms(LDGROUP, sconfig, &ldPath, &partialSum);

    sum += partialSum;
  }

  //  printf("Le L %i E %i W %g, S %g, PS %g\n", 
  //	 path->m_lightSize, path->m_eyeSize, thisCi, sum, partialSum);

  // Combine weight and pdf
  return(thisCi / (sum * totalPdf));
}


// Standard BPT pdf and weights
double CLeSpar::EvalPDFAndWeight(CSparConfig *sconfig,
			   CBiPath *path)
{
  return path->EvalPDFAndWeight(sconfig->m_bcfg);

  
}


void CLeSpar::GetStoredRadiance(CPathNode *node)
{
  // If node is on an edf, the correct value is filled in,
  // otherwise make any current value zero.

  EDF *endingEdf = node->m_hit.material->edf;

  if(endingEdf == NULL)
  {
    node->m_bsdfComp.Clear();
  }
}






void CLDSpar::Init(CSparConfig *sconfig)
{
  CSpar::Init(sconfig);

  if(!(sconfig->m_bcfg->doLD || sconfig->m_bcfg->doWeighted))
    return;
  
  // Overlap group
  
  if(sconfig->m_bcfg->doLD)
  {
    ParseAndInit(DISJUNCTGROUP, sconfig->m_bcfg->ldRegExp);
  }
  
  if(sconfig->m_bcfg->doWeighted)
  {
    ParseAndInit(LDGROUP, sconfig->m_bcfg->wldRegExp);
    m_sparList[LDGROUP].Add(sconfig->m_leSpar);
  }
}

COLOR CLDSpar::HandlePath(CSparConfig *sconfig,
			   CBiPath *path)
{
   COLOR result, col;
       
   COLORCLEAR(result);


   // Only path tracing paths !!
   if(path->m_lightSize == 0)
   {
     double wp;
     double totalGeom = path->ComputeTotalGeomFactor();

     // DISJUNCT GROUP
     if(sconfig->m_bcfg->doLD)
     {
       col = EvalFunction(&m_contrib[DISJUNCTGROUP], path);
       
       if(COLORAVERAGE(col) > ZEROEPSILON)
       {
	 wp = EvalPDFAndWeight(sconfig, path);
	 COLORADDSCALED(result, wp * totalGeom, col, result);
       }
     }

     // OVERLAP GROUP
     if(sconfig->m_bcfg->doWeighted && sconfig->m_bcfg->doWLD)
     {
       col = EvalFunction(&m_contrib[LDGROUP], path);
       
       if(COLORAVERAGE(col) > ZEROEPSILON)
       {
	 wp = EvalPDFAndMPWeight(sconfig, path);

	 COLORADDSCALED(result, wp * totalGeom, col, result);
       }
     }
   }	 

   return result;
}

double CLDSpar::ComputeWeightTerms(TPathGroupID,
				   CSparConfig *sconfig,
				   CBiPath *path, double *sum)
{
  double Sld, weight;
  CPathNode *node, *L1;
  double pdfAcc = 1.0;
  int i;

  if(path->m_lightSize >= 1)
  {
    L1 = path->m_lightPath;
  }
  else
  {
    L1 = path->m_eyeEndNode;
  }

  node = path->m_eyePath;

  for(i = 0; i < path->m_eyeSize; i++)
  {
    pdfAcc *= node->m_pdfFromPrev;
    node = node->Next();
  }


  node = path->m_lightPath;

  for(i = 0; i < path->m_lightSize; i++)
  {
    //    if(node->m_pdfFromNext < EPSILON)
    //    {
    //      printf("PDF %g, i %i\n", node->m_pdfFromNext, i);
    //      BsdfPrint(stdout, node->m_useBsdf);
    //    }
    //    if(node->m_rrPdfFromNext < EPSILON)
    //    {
    //      printf("RR %g, i %i\n", node->m_rrPdfFromNext, i);
    //      BsdfPrint(stdout, node->m_useBsdf);
    //    }

    pdfAcc *= node->m_pdfFromNext;

    if(path->m_eyeSize + path->m_lightSize - i >= sconfig->m_bcfg->minimumPathDepth)
      pdfAcc *= node->m_rrPdfFromNext;

    node = node->Next();
  }  

  double u,v;
  COLOR col;

  
  PatchUV(L1->m_hit.patch, 
	  &L1->m_hit.point, &u, &v);

#ifdef WMP_WEIGHTS
  col = GAL_GetDirectRadiance(L1->m_hit.patch, u,v, L1->m_inDirF);
#else
  COLORCLEAR(col);
#endif

  COLORCLIPPOSITIVE(col, col);

  Sld = COLORAVERAGE(col);
  Sld = Sld * Sld;

  //  COLOR accuracy;

  //  accuracy = GAL_GetAccuracy(L1->m_hit.patch, u,v, L1->m_inDirF);

  //  printf("Sld %g\n", Sld);

  weight = sconfig->m_bcfg->samplesPerPixel * pdfAcc / Sld;
  weight = MPMISFUNC(weight);

  *sum = weight;

  return weight;
}


double CLDSpar::EvalPDFAndMPWeight(CSparConfig *sconfig, 
				   CBiPath *path)
{
  double totalPdf;
  double sum=0.0, partialSum, thisCi;

  // bi path pdf (weight = 1.0 in CSpar)

  totalPdf = 1.0 / CSpar::EvalPDFAndWeight(sconfig, path);

  // Ci's for path

  thisCi = ComputeWeightTerms(LDGROUP, sconfig, path, &partialSum);
  sum += partialSum;

  // sum Ci's from other Spars

  CSparListIter iter(m_sparList[LDGROUP]);
  CSpar **pspar;

  while((pspar = iter.Next()))
  {
    (*pspar)->ComputeWeightTerms(LDGROUP, sconfig, path, &partialSum);

    sum += partialSum;
  }

  //  printf("LD W %g L %i E %i P %g, S %g, PS %g\n", 
  //	 thisCi / sum, path->m_lightSize, path->m_eyeSize, thisCi, sum, partialSum);

  // Combine weight and pdf
  return(thisCi / (sum * totalPdf));
}

  // standard bpt weights, no other spars
double CLDSpar::EvalPDFAndWeight(CSparConfig *sconfig,
			    CBiPath *path)
{
  return CSpar::EvalPDFAndWeight(sconfig, path);
}

void CLDSpar::GetStoredRadiance(CPathNode *node)
{
  return;
}








void CIDSpar::Init(CSparConfig *sconfig)
{
   CSpar::Init(sconfig);

  if(!sconfig->m_bcfg->doLI)
    return;

   if(sconfig->m_bcfg->doLI)
   {
     ParseAndInit(DISJUNCTGROUP, sconfig->m_bcfg->liRegExp);
   }
}

COLOR CIDSpar::HandlePath(CSparConfig *sconfig,
			   CBiPath *path)
{
   COLOR result;

   COLORCLEAR(result);

   if(sconfig->m_bcfg->doLI)
   {
     // Only path tracing paths !!
     if(path->m_lightSize == 0)
     {
       double totalGeom = path->ComputeTotalGeomFactor();
       double wp;
       
       result = EvalFunction(&m_contrib[DISJUNCTGROUP], path);
       
       //     ColorPrint(stdout, result);
       //     printf(", Size : %i\n", path->m_eyeSize); 
       
       wp = EvalPDFAndWeight(sconfig, path);
       
       COLORSCALE(wp * totalGeom, result, result);
     }
   }

   return result;
}

void CIDSpar::GetStoredRadiance(CPathNode *node)
{
  return;
}
