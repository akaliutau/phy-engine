/* 
 * bipath.C: BiPath class. Just stores eyePath and lightPath, lengths and 
 * endnodes 
 */

#ifndef _BIPATH_H_
#define _BIPATH_H_

#include "pathnode.H"

class CBiPath
{
public:
  CPathNode *m_eyePath;
  CPathNode *m_eyeEndNode;
  int m_eyeSize;

  CPathNode *m_lightPath;
  CPathNode *m_lightEndNode;
  int m_lightSize;

  double m_pdfLNE; // pdf for sampling the light point separately

  VECTOR m_dirEL;
  VECTOR m_dirLE;  // connection dir's

  double m_geomConnect;

  // Constructor
  CBiPath(void);
  void Init(void);

  // ReleasePaths: release all nodes from eye and light path
  void ReleasePaths(void);

  // Some interesting methods, only to be called for
  // valid bidirectional paths
  COLOR EvalRadiance(void);

  // Evaluate accumulated PDF of a bipath (no weighting)
  double EvalPDFAcc(void);

  // Evaluate weight/pdf for a bipath, taking into account other pdf's
  // depending on the config (bcfg).
  double EvalPDFAndWeight(BP_BASECONFIG *bcfg, double *pPdf = NULL,
			  double *pWeight = NULL);

  // Product of geom factors
  double ComputeTotalGeomFactor(void);
};

#endif /* _BIPATH_H_ */
