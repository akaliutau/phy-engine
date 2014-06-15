

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "pools.h"
#include "error.h"
#include "options.h"

#include "scene.h"
#include "statistics.h"
#include "potential.h"
#include "camera.h"
#include "patch_flags.h"
#include "vertex.h"
#include "hierarchy.h"

#include "mcrad.h"
#include "mcradP.h"
#include "element.h"
#include "render.h"

#include "ui.h"
#include "vertex.h"



#ifdef TESTS
#include "tests.h"
#endif

STATE mcr;

static ENUMDESC approxVals[] = {
  { AT_CONSTANT	, "constant"	, 2 },
  { AT_LINEAR	, "linear"	, 2 },
  { AT_QUADRATIC, "quadratic"	, 2 },
  { AT_CUBIC	, "cubic"	, 2 },
  { 0, NULL, 0 }
};
MakeEnumOptTypeStruct(approxTypeStruct, approxVals);
#define Tapprox (&approxTypeStruct)

static ENUMDESC clusteringVals[] = {
  { NO_CLUSTERING	, "none"	, 2 },
  { ISOTROPIC_CLUSTERING, "isotropic"	, 2 },
  { ORIENTED_CLUSTERING	, "oriented"	, 2 },
  { 0, NULL, 0 }
};
MakeEnumOptTypeStruct(clusteringTypeStruct, clusteringVals);
#define Tclustering (&clusteringTypeStruct)

static ENUMDESC sequenceVals[] = {
  { S4D_RANDOM	, "PseudoRandom"	, 2 },
  { S4D_HALTON	, "Halton"		, 2 },
  { S4D_NIEDERREITER, "Niederreiter"	, 2 },
  { 0, NULL, 0 }
};
MakeEnumOptTypeStruct(sequenceTypeStruct, sequenceVals);
#define Tsequence (&sequenceTypeStruct)

static ENUMDESC estTypeVals[] = {
  { RW_SHOOTING	, "Shooting"	, 2 },
  { RW_GATHERING, "Gathering"	, 2 },
  { 0, NULL, 0 }
};
MakeEnumOptTypeStruct(estTypeTypeStruct, estTypeVals);
#define TestType (&estTypeTypeStruct)

static ENUMDESC estKindVals[] = {
  { RW_COLLISION	, "Collision"	, 2 },
  { RW_ABSORPTION	, "Absorption"	, 2 },
  { RW_SURVIVAL		, "Survival"	, 2 },
  { RW_LAST_BUT_NTH	, "Last-but-N"	, 2 },
  { RW_NLAST		, "Last-N"	, 2 },
  { 0, NULL, 0 }
};
MakeEnumOptTypeStruct(estKindTypeStruct, estKindVals);
#define TestKind (&estKindTypeStruct)

static ENUMDESC showWhatVals[] = {
  { SHOW_TOTAL_RADIANCE	, "total-radiance"	, 2 },
  { SHOW_INDIRECT_RADIANCE, "indirect-radiance"	, 2 },
  { SHOW_IMPORTANCE	, "importance"		, 2 },
  { 0, NULL, 0 }
};
MakeEnumOptTypeStruct(showWhatTypeStruct, showWhatVals);
#define TshowWhat (&showWhatTypeStruct)

static CMDLINEOPTDESC srrOptions[] = {
  {"-srr-ray-units",	8,	Tint,	&mcr.ray_units_per_it,	DEFAULT_ACTION,
   "-srr-ray-units <n>          : To tune the amount of work in a single iteration"},
  {"-srr-bidirectional", 7,	Tbool,	&mcr.bidirectional_transfers, DEFAULT_ACTION,
   "-srr-bidirectional <yes|no> : Use lines bidirectionally"},
  {"-srr-control-variate", 7, 	Tbool,  &mcr.constant_control_variate, DEFAULT_ACTION,
   "-srr-control-variate <y|n>  : Constant Control Variate variance reduction"},
  {"-srr-indirect-only", 7,	Tbool,	&mcr.indirect_only, 	DEFAULT_ACTION,
   "-srr-indirect-only <y|n>    : Compute indirect illumination only"},
#ifdef IDMCR
  {"-srr-importance-driven", 7,	Tbool,  &mcr.importance_driven,	DEFAULT_ACTION,
   "-srr-importance-driven <y|n>: Use view-importance"},
#endif
  {"-srr-sampling-sequence", 7,	Tsequence, &mcr.sequence, 	DEFAULT_ACTION,
   "-srr-sampling-sequence <type>: \"PseudoRandom\", \"Niederreiter\""},
#ifdef HOMCR
  {"-srr-approximation", 7,	Tapprox,  &mcr.approx_type, 	DEFAULT_ACTION,
   "-srr-approximation <order>  : \"constant\", \"linear\", \"quadratic\", \"cubic\""},
#endif
  {"-srr-hierarchical", 7,	Tbool,	&hierarchy.do_h_meshing, DEFAULT_ACTION,
   "-srr-hierarchical <y|n>     : hierarchical refinement"},
  {"-srr-clustering",   7,	Tclustering, &hierarchy.clustering, DEFAULT_ACTION,
   "-srr-clustering <mode>      : \"none\", \"isotropic\", \"oriented\""},
  {"-srr-epsilon",	7,	Tfloat,	&hierarchy.epsilon, 	DEFAULT_ACTION,
   "-srr-epsilon <float>        : link power threshold (relative w.r.t. max. selfemitted power)"},
  {"-srr-minarea",	7,	Tfloat,	&hierarchy.minarea,	DEFAULT_ACTION,
   "-srr-minarea <float>        : minimal element area (relative w.r.t. total area)"},
  {"-srr-display",	7,	TshowWhat, &mcr.show,	DEFAULT_ACTION,
   "-srr-display <what>         : \"total-radiance\", \"indirect-radiance\", \"weighting-gain\", \"importance\""},
  {"-srr-discard-incremental", 7,	Tbool,  &mcr.discard_incremental,	DEFAULT_ACTION,
   "-srr-discard-incremenal <y|n>: Discard result of first iteration (incremental steps)"},
  {"-srr-incremental-uses-importance", 7,	Tbool,  &mcr.incremental_uses_importance,	DEFAULT_ACTION,
   "-srr-incremental-uses-importance <y|n>: Use view-importance sampling already for the first iteration (incremental steps)"},
  {"-srr-naive-merging", 7,	Tbool,	&mcr.naive_merging, 	DEFAULT_ACTION,
   "-srr-naive-merging <y|n>    : disable intelligent merging heuristic"},
  {"-srr-nondiffuse-first-shot", 7,	Tbool,	&mcr.do_nondiffuse_first_shot, 	DEFAULT_ACTION,
   "-srr-nondiffuse-first-shot <y|n>: Do Non-diffuse first shot before real work"},
  {"-srr-initial-ls-samples",	7,	Tint,	&mcr.initial_ls_samples,	DEFAULT_ACTION,
   "-srr-initial-ls-samples <int>        : nr of samples per light source for initial shot"},
  {NULL	, 	0,	TYPELESS, 	NULL, 	DEFAULT_ACTION,
   NULL}
};

static CMDLINEOPTDESC rwrOptions[] = {
  {"-rwr-ray-units",	8,	Tint,	&mcr.ray_units_per_it,	DEFAULT_ACTION,
   "-rwr-ray-units <n>          : To tune the amount of work in a single iteration"},
  {"-rwr-continuous", 7, 	Tbool,  &mcr.continuous_random_walk, DEFAULT_ACTION,
   "-rwr-continuous <y|n>       : Continuous (yes) or Discrete (no) random walk"},
  {"-rwr-control-variate", 7, 	Tbool,  &mcr.constant_control_variate, DEFAULT_ACTION,
   "-rwr-control-variate <y|n>  : Constant Control Variate variance reduction"},
  {"-rwr-indirect-only", 7,	Tbool,	&mcr.indirect_only, 	DEFAULT_ACTION,
   "-rwr-indirect-only <y|n>    : Compute indirect illumination only"},
  {"-rwr-sampling-sequence", 7,	Tsequence, &mcr.sequence, 	DEFAULT_ACTION,
   "-rwr-sampling-sequence <type>: \"PseudoRandom\", \"Halton\", \"Niederreiter\""},
#ifdef HOMCR
  {"-rwr-approximation", 7,	Tapprox, &mcr.approx_type, 	DEFAULT_ACTION,
   "-rwr-approximation <order>  : \"constant\", \"linear\", \"quadratic\", \"cubic\""},
#endif
  {"-rwr-estimator", 	7,	TestType, &mcr.rw_estimator_type, DEFAULT_ACTION,
   "-rwr-estimator <type>       : \"shooting\", \"gathering\""},
  {"-rwr-score", 	7,	TestKind, &mcr.rw_estimator_kind, DEFAULT_ACTION,
   "-rwr-score <kind>           : \"collision\", \"absorption\", \"survival\", \"last-N\", \"last-but-N\""},
  {"-rwr-numlast", 12, 	Tint,	&mcr.rw_numlast,	DEFAULT_ACTION,
   "-rwr-numlast <int>          : N to use in \"last-N\" and \"last-but-N\" scorers"},
  {NULL	, 	0,	TYPELESS, 	NULL, 	DEFAULT_ACTION,
   NULL}
};

void McrDefaults(void)
{
  mcr.hack = FALSE;
  mcr.inited = FALSE;
  mcr.no_smoothing = FALSE;
  mcr.ray_units_per_it = 10;
  mcr.bidirectional_transfers = FALSE;
  mcr.constant_control_variate = FALSE;
  COLORCLEAR(mcr.control_radiance);
  mcr.indirect_only = FALSE;
  mcr.sequence = S4D_NIEDERREITER;
  mcr.approx_type = AT_CONSTANT;
  mcr.importance_driven = FALSE;
  mcr.radiance_driven = TRUE;
  mcr.importance_updated = FALSE;
  mcr.importance_updated_from_scratch = FALSE;
  mcr.continuous_random_walk = FALSE;
  mcr.rw_estimator_type = RW_SHOOTING;
  mcr.rw_estimator_kind = RW_COLLISION;
  mcr.rw_numlast = 1;
  mcr.k_factor = 1.;
  mcr.show_shooting_weights = FALSE;
  mcr.weighted_sampling = FALSE;
  mcr.fake_global_lines = FALSE;
  mcr.discard_incremental = FALSE;
  mcr.incremental_uses_importance = FALSE;
  mcr.naive_merging = FALSE;

  mcr.show = SHOW_TOTAL_RADIANCE;
  mcr.show_weighted = SHOW_NON_WEIGHTED;

  mcr.do_nondiffuse_first_shot = FALSE;
  mcr.initial_ls_samples = 1000;

  ElementHierarchyDefaults();
  InitBasis();
}

void SrrParseOptions(int *argc, char **argv)
{
  ParseOptions(srrOptions, argc, argv);
}

void SrrPrintOptions(FILE *fp)
{
  fprintf(fp, "\nStochastic Jacobi Radiosity options:\n");
  PrintOptions(fp, srrOptions);
}

void RwrParseOptions(int *argc, char **argv)
{
  ParseOptions(rwrOptions, argc, argv);
}

void RwrPrintOptions(FILE *fp)
{
  fprintf(fp, "\nRandom Walk Radiosity options:\n");
  PrintOptions(fp, rwrOptions);
}


void McrUpdateCpuSecs(void)
{
  clock_t t;

  t = clock();
  mcr.cpu_secs += (float)(t - mcr.lastclock)/(float)CLOCKS_PER_SEC;
  mcr.lastclock = t;
}


void McrWakeUp(int sig)
{
  mcr.wake_up = TRUE;

  signal(SIGALRM, McrWakeUp);
  alarm( 1 );

  McrUpdateCpuSecs();
}

void *McrCreatePatchData(PATCH *patch)
{
  patch->radiance_data = (void *)CreateToplevelSurfaceElement(patch);
  return patch->radiance_data;
}

void McrPrintPatchData(FILE *out, PATCH *patch)
{
  PrintElement(out, TOPLEVEL_ELEMENT(patch));
}

void McrDestroyPatchData(PATCH *patch)
{
  if (patch->radiance_data)
    McrDestroyToplevelSurfaceElement(TOPLEVEL_ELEMENT(patch));
  patch->radiance_data = (void *)NULL;
}


void McrPatchComputeNewColor(PATCH *patch)
{
  patch->color = ElementColor(TOPLEVEL_ELEMENT(patch));
  PatchComputeVertexColors(patch);
}


void McrInit(void)
{
  mcr.inited = FALSE;
}


static void McrInitPatch(PATCH *P)
{
  COLOR Ed = EMITTANCE(P);

  ReAllocCoefficients(TOPLEVEL_ELEMENT(P));
  CLEARCOEFFICIENTS(RAD(P), BAS(P));
  CLEARCOEFFICIENTS(UNSHOT_RAD(P), BAS(P));
  CLEARCOEFFICIENTS(RECEIVED_RAD(P), BAS(P));

  RAD(P)[0] = UNSHOT_RAD(P)[0] = SOURCE_RAD(P) = Ed;
#ifdef NEVER
  if (mcr.indirect_only) {
    
    COLORCLEAR(RAD(P)[0]); COLORCLEAR(SOURCE_RAD(P));
  }
#endif
  COLORCLEAR(RECEIVED_RAD(P)[0]);

  RAY_INDEX(P) = P->id * 11;
  QUALITY(P) = 0.; NG(P) = 0;

#ifdef IDMCR
  IMP(P) = UNSHOT_IMP(P) = RECEIVED_IMP(P) = SOURCE_IMP(P) = 0.;
#endif
}

#ifdef IDMCR

static void PullImportances(ELEMENT *child)
{
  ELEMENT *parent = child->parent;
  PullImportance(parent, child, &parent->imp, &child->imp);
  PullImportance(parent, child, &parent->source_imp, &child->source_imp);
  PullImportance(parent, child, &parent->unshot_imp, &child->unshot_imp);
}

static void AccumulateImportances(ELEMENT *elem)
{
  mcr.total_ymp += elem->area * elem->imp;
  mcr.source_ymp += elem->area * elem->source_imp;
  mcr.unshot_ymp += elem->area * fabs(elem->unshot_imp);
}


static void UpdateImportance(ELEMENT *elem)
{
  if (!McrForAllChildrenElements(elem, UpdateImportance)) {
    
    float delta_imp = (PATCH_IS_VISIBLE(elem->pog.patch) ? 1. : 0.) - elem->source_imp;
    elem->imp += delta_imp;
    elem->source_imp += delta_imp;
    elem->unshot_imp += delta_imp;
    AccumulateImportances(elem);
  } else {
    
    elem->imp = elem->source_imp = elem->unshot_imp = 0.;
    McrForAllChildrenElements(elem, PullImportances);
  }
}


static void ReInitImportance(ELEMENT *elem)
{
  if (!McrForAllChildrenElements(elem, ReInitImportance)) {
    
    elem->imp = elem->source_imp = elem->unshot_imp
      = PATCH_IS_VISIBLE(elem->pog.patch) ? 1. : 0.;
    AccumulateImportances(elem);
  } else {
    
    elem->imp = elem->source_imp = elem->unshot_imp = 0.;
    McrForAllChildrenElements(elem, PullImportances);
  }
}

void McrUpdateViewImportance(void)
{
  fprintf(stderr, "Updating direct visibility ... \n");

  UpdateDirectVisibility();

  mcr.source_ymp = mcr.unshot_ymp = mcr.total_ymp = 0.;
  UpdateImportance(hierarchy.topcluster);

  if (mcr.unshot_ymp < mcr.source_ymp) {
    fprintf(stderr, "Importance will be recomputed incrementally.\n");
    mcr.importance_updated_from_scratch = FALSE;
  } else {
    fprintf(stderr, "Importance will be recomputed from scratch.\n");
    mcr.importance_updated_from_scratch = TRUE;

    
    mcr.source_ymp = mcr.unshot_ymp = mcr.total_ymp = 0.;
    ReInitImportance(hierarchy.topcluster);
  }

  Camera.changed = FALSE;	
  mcr.imp_traced_rays = 0;	
  mcr.importance_updated = TRUE;
}
#endif


static int fcmp(float *f1, float *f2)
{
  return (*f1 > *f2) ? -1 : (*f1 < *f2 ? +1 : 0);
}


static double DetermineAreaFraction(void)
{
  float *areas, cumul, areafrac;
  int nrpatchids = PatchGetNextID(), i;

  if (!World) return 100;	

  
  areas = (float *)Alloc(nrpatchids * sizeof(float));
  for (i=0; i<nrpatchids; i++) areas[i] = 0.;
  ForAllPatches(P, Patches) {
    areas[P->id] = P->area;
  } EndForAll;

  
  qsort((void *)areas, nrpatchids, sizeof(float), (int (*)(const void *, const void *))fcmp);

  
  for (i=nrpatchids-1, cumul=0.; i>=0 && cumul < total_area * 0.1; i--) {
    cumul += areas[i];
  }
  areafrac = (i>=0 && areas[i] > 0.) ? total_area/areas[i] : nrpatches;

  Free((char *)areas, nrpatchids*sizeof(float));

  return areafrac;
}


static void McrDetermineInitialNrRays(void)
{
  double areafrac = DetermineAreaFraction();
  mcr.initial_nr_rays = (long)((double)mcr.ray_units_per_it * areafrac);
}


void McrReInit(void)
{
  if (mcr.inited) return;

  fprintf(stderr, "Initialising Monte Carlo radiosity ...\n");

  SetSequence4D(mcr.sequence);

  mcr.inited = TRUE;
  mcr.cpu_secs = 0.;
  mcr.lastclock = clock();
  mcr.iteration_nr = 0;
  mcr.traced_rays = mcr.prev_traced_rays = mcr.nrmisses = 0;
  mcr.imp_traced_rays = mcr.prev_imp_traced_rays = 0;
  mcr.set_source = mcr.indirect_only;
  mcr.traced_paths = 0;
  COLORCLEAR(mcr.control_radiance);
  mcr.nr_weighted_rays = mcr.old_nr_weighted_rays = 0;

  COLORCLEAR(mcr.unshot_flux);	mcr.unshot_ymp = 0.;
  COLORCLEAR(mcr.total_flux);	mcr.total_ymp = 0.;
  COLORCLEAR(mcr.imp_unshot_flux);
  ForAllPatches(P, Patches) {
    McrInitPatch(P);
    COLORADDSCALED(mcr.unshot_flux, M_PI*P->area, UNSHOT_RAD(P)[0], mcr.unshot_flux);
    COLORADDSCALED(mcr.total_flux, M_PI*P->area, RAD(P)[0], mcr.total_flux);
#ifdef IDMCR
    COLORADDSCALED(mcr.imp_unshot_flux, M_PI*P->area*(IMP(P)-SOURCE_IMP(P)), UNSHOT_RAD(P)[0], mcr.imp_unshot_flux);
    mcr.unshot_ymp += P->area * fabs(UNSHOT_IMP(P));
    mcr.total_ymp += P->area * IMP(P);
    mcr.source_ymp += P->area * SOURCE_IMP(P);
#endif
    McrPatchComputeNewColor(P);
  } EndForAll;

  McrDetermineInitialNrRays();

  ElementHierarchyInit();

#ifdef IDMCR
  if (mcr.importance_driven) {
    McrUpdateViewImportance();
    mcr.importance_updated_from_scratch = TRUE;
  }
#endif
#ifdef TESTS
  InitTests();
#endif
}

static void (*prev_alrm_handler)(int signr);
static unsigned prev_alarm_left;

void McrPreStep(void)
{
  if (!mcr.inited) McrReInit();
#ifdef IDMCR
  if (mcr.importance_driven && Camera.changed) McrUpdateViewImportance();
#endif

  
  prev_alrm_handler = signal(SIGALRM, McrWakeUp);
  prev_alarm_left = alarm( 1 );
  mcr.wake_up = FALSE;
  mcr.lastclock = clock();

  mcr.iteration_nr ++;
}

void McrPostStep(void)
{
  
  signal(SIGALRM, prev_alrm_handler);
  alarm(prev_alarm_left);

#ifdef TESTS
  do_tests();
#endif
}


void McrTerminate(void)
{
  ElementHierarchyTerminate();
  mcr.inited = FALSE;
}

static COLOR McrDiffuseReflectanceAtPoint(PATCH *patch, double u, double v)
{
  HITREC hit;
  POINT point; 
  PatchUniformPoint(patch, u, v, &point);
  InitHit(&hit, patch, NULL, &point, &patch->normal, patch->surface->material, 0.);
  hit.uv.u = u;
  hit.uv.v = v;
  hit.flags |= HIT_UV;
  return BsdfScatteredPower(hit.material->bsdf, &hit, &patch->normal, BRDF_DIFFUSE_COMPONENT);
}

static COLOR McrInterpolatedReflectanceAtPoint(ELEMENT *leaf, double u, double v)
{
  static ELEMENT *cachedleaf = NULL;
  static COLOR vrd[4], rd;

  if (leaf != cachedleaf) {
    int i;
    for (i=0; i<leaf->nrvertices; i++)
      vrd[i] = VertexReflectance(leaf->vertex[i]);
  }
  cachedleaf = leaf;

  COLORCLEAR(rd);
  switch (leaf->nrvertices) {
  case 3:
    COLORINTERPOLATEBARYCENTRIC(vrd[0], vrd[1], vrd[2], u, v, rd); break;
  case 4:
    COLORINTERPOLATEBILINEAR(vrd[0], vrd[1], vrd[2], vrd[3], u, v, rd); break;
  default:
    Fatal(-1, "McrInterpolatedReflectanceAtPoint", "Invalid nr of vertices %d", leaf->nrvertices);
  }
  return rd;
}


COLOR McrGetRadiance(PATCH *patch, double u, double v, VECTOR dir)
{
  COLOR TrueRdAtPoint = McrDiffuseReflectanceAtPoint(patch, u, v);
  ELEMENT *leaf = RegularLeafElementAtPoint(TOPLEVEL_ELEMENT(patch), &u, &v);
  COLOR UsedRdAtPoint = renderopts.smooth_shading ? McrInterpolatedReflectanceAtPoint(leaf, u, v) : leaf->Rd;
  COLOR rad = ElementDisplayRadianceAtPoint(leaf, u, v);
  COLOR source_rad; COLORCLEAR(source_rad);

  
  if (mcr.show != SHOW_INDIRECT_RADIANCE) {
    
    if (!mcr.do_nondiffuse_first_shot)
      source_rad = leaf->source_rad;
    if (mcr.indirect_only || mcr.do_nondiffuse_first_shot) {
      
      COLORADD(source_rad, leaf->Ed, source_rad);
    }
  }
  COLORSUBTRACT(rad, source_rad, rad);

  COLORPROD(rad, TrueRdAtPoint, rad);
  COLORDIV(rad, UsedRdAtPoint, rad);

  
  COLORADD(rad, source_rad, rad);

  return rad;
}

void McrRecomputeDisplayColors(void)
{
  if (!mcr.inited) return;

  fprintf(stderr, "Recomputing display colors ...\n");
  ForAllPatches(P, Patches) {
    McrPatchComputeNewColor(P);
  } EndForAll;
}

void McrUpdateMaterial(MATERIAL *oldmaterial, MATERIAL *newmaterial)
{
  Error("McrUpdateMaterial", "Not yet implemented");
}


float McrScalarReflectance(PATCH *P)
{
  return ElementScalarReflectance(TOPLEVEL_ELEMENT(P));
}


double VarianceEstimate(double N, double sum_of_squares, double square_of_sum)
{
  return 1./(N-1.) * (sum_of_squares/N - square_of_sum/(N*N));
}

void McrDumpLeaf(PATCH *patch, double u, double v)
{
  ELEMENT *leaf = RegularLeafElementAtPoint(TOPLEVEL_ELEMENT(patch), &u, &v);
  RenderElementOutline(leaf);
  PrintElement(stderr, leaf);
}
