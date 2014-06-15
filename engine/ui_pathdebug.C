

#include <limits.h>

#include <Xm/MessageB.h>

#include "ui_sdk.h"
#include "ui_pathdebug.h"
#include "select.h"
#include "canvas.h"
#include "error.h"
#include "renderhook.h"
#include "render.h"

#include "CSList.H"

#include "raytracing/samplertools.H"
#include "raytracing/pathnode.H"
#include "raytracing/bipath.H"

#include "raytracing/eyesampler.H"
#include "raytracing/pixelsampler.H"
#include "raytracing/lightsampler.H"
#include "raytracing/lightdirsampler.H"
#include "raytracing/bsdfsampler.H"
#include "raytracing/photonmapsampler.H"
#include "raytracing/specularsampler.H"

#include "stratification.H"


class CPathDebug;
static CPathDebug *currentPathDebug = NULL;

class CRenderNodeConfig
{
public:
  bool m_doShadingFrame;
  bool m_doShadingFrameDerivative;
  bool m_doDirDifferentials;
  bool m_doPointDifferentials;
  bool m_doGaussian;
  float m_dirFactor;
  float m_pointFactor;

  CRenderNodeConfig(void)
  {
    m_doShadingFrame = false;
    m_doShadingFrameDerivative = false;
    m_doDirDifferentials = false;
    m_doPointDifferentials = false;
    m_doGaussian = true;
    m_dirFactor = 3;
    m_pointFactor = 3;
  }
};

static CRenderNodeConfig nodeCfg;



// -- TODO Relocate to 'renderpathnode.C'

void RenderPathNode(CPathNode *node, CRenderNodeConfig *cfg)
{
  //RenderSetColor(&Red);
  //RenderPoint(&node->m_hit.point);

  if(cfg->m_doShadingFrame)
  {
    VECTOR X,Y,Z;
    
    HitShadingFrame(&node->m_hit, &X, &Y, &Z);
    RenderCoordinateFrame(&(node->m_hit.point), &X, &Y, &Z);
  }

    
}

void RenderPathNodeConnect(CPathNode *node, CPathNode *nextNode)
{
  RenderSetColor(&Yellow);
  RenderAALine(&node->m_hit.point, &nextNode->m_hit.point);  
  RenderArrow(&node->m_hit.point, &nextNode->m_hit.point);  
}

// RenderPath renders all nodes until maxdepth or
// a raytype 'Ends' or a NULL node is reached
void RenderPath(CPathNode *path, CRenderNodeConfig *cfg, int maxdepth=INT_MAX)
{
  CPathNode *node = path;
  CPathNode *nextNode;

  while(node)
  {
    // The node axes
    RenderPathNode(node, cfg);
    
    // The connection to the next node
    if(!(node->Ends()) && (node->m_depth < maxdepth))
      nextNode = node->Next();
    else
      nextNode = NULL;
    
    if(nextNode)
      RenderPathNodeConnect(node, nextNode);
    
    // Next node
    node = nextNode;
  }
}




// A class for a single render path
class CRenderPath
{
private:
  CBiPath *m_bipath;  
public:
  CRenderPath(CBiPath *bipath);
  void Release(void);
  void Render(CRenderNodeConfig *cfg);
};

// Implemenation

CRenderPath::CRenderPath(CBiPath *bipath)
{
  m_bipath = bipath;
}

void CRenderPath::Release(void)
{
  if(m_bipath)
  {
	m_bipath->ReleasePaths();
	delete m_bipath;
  }
}

void CRenderPath::Render(CRenderNodeConfig *cfg)
{
  if(m_bipath->m_eyePath)
    RenderPath(m_bipath->m_eyePath, cfg);

  if(m_bipath->m_lightPath)
    RenderPath(m_bipath->m_lightPath, cfg);
}


// RenderPath list type

typedef CTSList<CRenderPath> CRenderPathList;
typedef CTSList_Iter<CRenderPath> CRenderPathList_Iter;

// The path debug class manages the render-paths and traces new paths
class CPathDebug
{
private:
  // paths & other info
  
  CRenderPathList m_bpList;
  bool m_renderEnabled;

public:
  CPathDebug(void);
  ~CPathDebug(void);

  void Reinit(void);
  void ReleasePaths(void);
  void ToggleRender(void);
  void RenderPaths(void);
  void SetRendering(bool on);
  void TraceNewEyePath(int nx, int ny);
  void TraceNewLightPath(void);
  void TraceNewEyeSplit(int nx, int ny);
};


// Callback for rendering paths of a CPathDebug instance
static void PathDebugRenderHook(void *data)
{
  ((CPathDebug *)data)->RenderPaths();
}


// CPathDebug methods

CPathDebug::CPathDebug(void)
{
  m_renderEnabled = false;
}

CPathDebug::~CPathDebug(void)
{
  if(m_renderEnabled)
	RemoveRenderHook(PathDebugRenderHook, this);  

  ReleasePaths();
}


void CPathDebug::Reinit(void)
{
  ReleasePaths();
  m_renderEnabled = false;
  SetRendering(true);
}

void CPathDebug::ReleasePaths(void)
{
   CRenderPathList_Iter iter(m_bpList);
   CRenderPath *rp;

   while((rp = iter.Next()))
   {
	 rp->Release();
   }

   m_bpList.RemoveAll();
}

void CPathDebug::SetRendering(bool on)
{
  if(on && !m_renderEnabled)
  {
	AddRenderHook(PathDebugRenderHook, this);	
  }
  else if(!on && m_renderEnabled)
  {
	RemoveRenderHook(PathDebugRenderHook, this);
  }

  m_renderEnabled = on;
}

void CPathDebug::ToggleRender(void)
{
  SetRendering(!m_renderEnabled);
}

void CPathDebug::RenderPaths(void)
{
   CRenderPathList_Iter iter(m_bpList);
   CRenderPath *rp;

   while((rp = iter.Next()))
   {
	 rp->Render(&nodeCfg);
   }
}

void CPathDebug::TraceNewEyePath(int nx, int ny)
{
  CSamplerConfig scfg;
  CBiPath *bipath = new CBiPath;

  scfg.pointSampler = new CEyeSampler;
  scfg.dirSampler = new CPixelSampler;
  ((CPixelSampler *)scfg.dirSampler)->SetPixel(nx,ny);
  scfg.surfaceSampler = new CPhotonMapSampler; // CBsdfSampler; //CSpecularSampler;

  scfg.minDepth = 3;
  scfg.maxDepth = 4;

  CPathNode::m_dmaxsize = 2 * scfg.maxDepth;

  bipath->m_eyePath = scfg.TracePath(NULL, BSDF_ALL_COMPONENTS);

  scfg.ReleaseVars();

  m_bpList.Append(CRenderPath(bipath));

  RenderScene();
}

void CPathDebug::TraceNewEyeSplit(int nx, int ny)
{
  CSamplerConfig scfg;

  scfg.pointSampler = new CEyeSampler;
  scfg.dirSampler = new CPixelSampler;
  ((CPixelSampler *)scfg.dirSampler)->SetPixel(nx,ny);
  scfg.surfaceSampler = new CBsdfSampler; //CSpecularSampler;

  scfg.minDepth = 3;
  scfg.maxDepth = 3;

  CPathNode::m_dmaxsize = 2 * scfg.maxDepth;

  const int div = 100;
  // const int split = div*div;
  // CStrat2D strat(split);
  double x_1,x_2;

  for(int i = 0; i < div ; i++)
  {
    CBiPath *bipath = new CBiPath;
    CPathNode *node = NULL;

    // eye
    node = bipath->m_eyePath = scfg.TraceNode(NULL, 0, 0, BSDF_ALL_COMPONENTS);
    // pixel center
    if(node)
    {
      node->EnsureNext();
      node = scfg.TraceNode(node->Next(), 0.5, 0.5, BSDF_ALL_COMPONENTS);
      
      if(node)
      {
	// scatter
	// strat.Sample(&x_1, &x_2);

	int a = i / div;
	int b = i % div;
	x_1 = (a + 0.5) / (double)(div);
	x_2 = (b + 0.5) / (double)(div);

	node->EnsureNext();
	node = scfg.TraceNode(node->Next(), x_1, x_2, BSDF_ALL_COMPONENTS);
      }
    }
	
    m_bpList.Append(CRenderPath(bipath));
  }

  scfg.ReleaseVars();
  RenderScene();
}


void CPathDebug::TraceNewLightPath(void)
{
  CSamplerConfig scfg;
  CBiPath *bipath = new CBiPath;

  scfg.pointSampler = new CUniformLightSampler;
  scfg.dirSampler = new CLightDirSampler;
  scfg.surfaceSampler = new CBsdfSampler; //CSpecularSampler;

  scfg.minDepth = 3;
  scfg.maxDepth = 3;

  CPathNode::m_dmaxsize = 2 * scfg.maxDepth;

  bipath->m_eyePath = NULL;
  bipath->m_lightPath = scfg.TracePath(NULL, BSDF_ALL_COMPONENTS);

  scfg.ReleaseVars();

  m_bpList.Append(CRenderPath(bipath));

  RenderScene();
}




static void DoDebugTracePath(int nx, int ny)
{
  fprintf(stderr, "%i %i\n", nx, ny);
  currentPathDebug->TraceNewEyePath(nx, ny);

}
static void DebugTracePathCallback(Widget w, XtPointer client_data, 
				   XtPointer calldata)
{
  fprintf(stderr, "Select a pixel ... "); fflush(stderr);
  SelectPixelSetCallback(DoDebugTracePath);
  CanvasPushMode(CANVASMODE_SELECT_PIXEL);
}

static void DebugTraceLightPathCallback(Widget w, XtPointer client_data, 
				   XtPointer calldata)
{
  currentPathDebug->TraceNewLightPath();
}

static void DoDebugTraceSplit(int nx, int ny)
{
  fprintf(stderr, "%i %i\n", nx, ny);
  currentPathDebug->TraceNewEyeSplit(nx, ny);

}
static void DebugTraceSplitCallback(Widget w, XtPointer client_data, 
				   XtPointer calldata)
{
  fprintf(stderr, "Select a pixel ... "); fflush(stderr);
  SelectPixelSetCallback(DoDebugTraceSplit);
  CanvasPushMode(CANVASMODE_SELECT_PIXEL);
}


static void DebugReleasePathsCallback(Widget w, XtPointer client_data, 
				   XtPointer calldata)
{
  if(currentPathDebug)
  {
    currentPathDebug->ReleasePaths();
    RenderScene();
  }
}

static void PathDebugOKCallback(Widget w, XtPointer client_data, 
								   XtPointer calldata)
{
  if(currentPathDebug)
  {
    delete currentPathDebug;
    currentPathDebug = NULL;
  }

  RenderScene();
}

void InitPathDebug(void)
{
  if(currentPathDebug)
    currentPathDebug->Reinit();
}


static void ToggleCallback(Widget w, XtPointer client_data,
				       XtPointer call_data)
{
  bool *toggle = (bool *)client_data;
  int set = ((XmToggleButtonCallbackStruct *)call_data)->set;
  if (set == XmSET)
    *toggle = true;
  else
    *toggle = false;

  RenderScene();
}

void ShowPathDebugPanel(Widget w, XtPointer client_data, 
						XtPointer call_data)
{
  fprintf(stderr, "ShowPathDebugPanel\n");

  // currentPathDebug should be NULL, otherwise the control is already open

  if(currentPathDebug == NULL)
    currentPathDebug = new CPathDebug;
  else
    return;

  currentPathDebug->SetRendering(true);

  Widget pathDebugPanel, form;

  pathDebugPanel = CreateDialog(w, "pathDebugPanel");
  form = CreateRowColumn(pathDebugPanel, "pathDebugForm");

  CreateToggleButton(form, "pathDebugToggleShadingFrame",
			    (nodeCfg.m_doShadingFrame), 
			    ToggleCallback,
			    (XtPointer)&nodeCfg.m_doShadingFrame);

  CreatePushButton(form,"pathDebugTracePath", DebugTracePathCallback, 
				   (XtPointer)NULL);

  CreatePushButton(form,"pathDebugTraceLightPath", 
		   DebugTraceLightPathCallback, 
		   (XtPointer)NULL);

  CreatePushButton(form,"pathDebugTraceSplit", DebugTraceSplitCallback, 
				   (XtPointer)NULL);

  CreatePushButton(form,"pathDebugReleasePaths", DebugReleasePathsCallback, 
				   (XtPointer)NULL);

  XtManageChild(form);

  // OK and Cancel buttons are automatically created when 
  // a callback or labelstring is defined for them.
  XtAddCallback(pathDebugPanel, XmNokCallback, PathDebugOKCallback, 
				(XtPointer)NULL);

 
  Widget temp = XmMessageBoxGetChild(pathDebugPanel, XmDIALOG_CANCEL_BUTTON);
  XtUnmanageChild(temp);

  
  XtManageChild(pathDebugPanel);
}
