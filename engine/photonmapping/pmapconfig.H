// pmapconfig.H : class for Photon map configuration
/* photon map configuration structure, used during construction */

#ifndef _PMAPCONFIG_H_
#define _PMAPCONFIG_H_

#include "screenbuffer.H"
#include "bipath.H"
#include "samplertools.H"
#include "photonmap.H"
#include "importancemap.H"

class PMAPCONFIG
{
public:
  CSamplerConfig lightConfig;
  CSamplerConfig eyeConfig;
  CBiPath bipath;

  CImportanceMap *importanceMap;
  CImportanceMap *importanceCMap;
  CPhotonMap *globalMap;
  CPhotonMap *causticMap;

  CPhotonMap *currentMap; // Map in current use: global or caustic
  CImportanceMap *currentImpMap; // Importance Map in current use: global or caustic

  CScreenBuffer *screen;

  PMAPCONFIG(void) { screen = NULL; currentMap = NULL; }
};

extern PMAPCONFIG pmapconfig;

#endif
