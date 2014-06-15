





#include "scene.h"
#include "spherical.h"
#include "statistics.h"
#include "error.h"
#include "cie.h"

#include "mcradP.h"
#include "hierarchy.h"
#include "stochjacobi.h"
#include "ccr.h"
#include "niederreiter.h"
#include "sample4d.h"
#include "localline.h"


static COLOR *(*get_radiance)(ELEMENT *);
static float (*get_importance)(ELEMENT *);

static void (*reflect)(ELEMENT *, double) = (void (*)(ELEMENT*,double))NULL;

static int do_control_variate;	
static int nr_rays;		
static double sum_probs = 0.0; 	

static void InitGlobals(int nrrays,
			COLOR *(*GetRadiance)(ELEMENT *),
			float (*GetImportance)(ELEMENT *),
			void (*Update)(ELEMENT *P, double w))
{
  nr_rays = nrrays;
  get_radiance = GetRadiance;
  get_importance = GetImportance;
  reflect = Update;
  
  do_control_variate = (mcr.constant_control_variate && (GetRadiance));

  if (get_radiance) mcr.prev_traced_rays = mcr.traced_rays;
  if (get_importance) mcr.prev_imp_traced_rays = mcr.imp_traced_rays;
}

static void PrintMessage(long nr_rays)
{
  fprintf(stderr, "%s-directional ",
	  mcr.bidirectional_transfers ? "Bi" : "Uni");
  if (get_radiance && get_importance)
    fprintf(stderr, "combined ");
  if (get_radiance)
    fprintf(stderr, "%sradiance ",
	    mcr.importance_driven ? "importance-driven " : "");
  if (get_radiance && get_importance)
    fprintf(stderr, "and ");
  if (get_importance)
    fprintf(stderr, "%simportance ",
	    mcr.radiance_driven ? "radiance-driven " : "");
  fprintf(stderr, "propagation");
  if (do_control_variate)
    fprintf(stderr, "using a constant control variate ");
  fprintf(stderr, "(%ld rays):\n", nr_rays);
}


static double Probability(ELEMENT *elem)
{
  double prob = 0.;

  if (get_radiance) {
    
    COLOR radiance = get_radiance(elem)[0];
    if (mcr.constant_control_variate) {
      COLORSUBTRACT(radiance, mcr.control_radiance, radiance);
    }
    prob =  elem->area * COLORSUMABSCOMPONENTS(radiance);
#ifdef IDMCR
    if (mcr.importance_driven) {
      
      float w = (elem->imp - elem->source_imp);
      prob *= ((w>0.) ? w : 0.);
    }
#endif
#ifdef FAKE_GLOBAL_LINES
    if (mcr.fake_global_lines &&
	(!mcr.indirect_only || mcr.iteration_nr > 1))
      prob = elem->area;
#endif
  }

#ifdef IDMCR
  if (get_importance && mcr.importance_driven) {
    double prob2 = elem->area * fabs(get_importance(elem)) * ElementScalarReflectance(elem);

    if (mcr.radiance_driven) {
      
      COLOR received_radiance;
      COLORSUBTRACT(elem->rad[0], elem->source_rad, received_radiance);
      prob2 *= COLORSUMABSCOMPONENTS(received_radiance);
    }

    
    prob = prob * approxdesc[mcr.approx_type].basis_size + prob2;
  }
#endif

  return prob;
}


static void ElementClearAccumulators(ELEMENT *elem)
{
  if (get_radiance) {
    CLEARCOEFFICIENTS(elem->received_rad, elem->basis);
  }
#ifdef IDMCR
  if (get_importance)
    elem->received_imp = 0.;
#endif
}


static void ElementSetup(ELEMENT *elem)
{
  elem->prob = 0.;
  if (!ForAllChildrenElements(elem, ElementSetup)) {
    
    elem->prob = Probability(elem);
    sum_probs += elem->prob;
  }
  if (elem->parent)
    
    elem->parent->prob += elem->prob;

  ElementClearAccumulators(elem);
}


static int Setup(void)
{
  
  COLORCLEAR(mcr.control_radiance);
  if (do_control_variate)
    mcr.control_radiance = DetermineControlRadiosity(get_radiance, NULL);

  sum_probs = 0.;
  ElementSetup(hierarchy.topcluster);

  if (sum_probs < EPSILON*EPSILON) {
    Warning("Iteration", "No sources");
    return FALSE;
  }
  return TRUE;
}


static COLOR GetSourceRadiance(ELEMENT *src, double us, double vs)
{
  COLOR *srcrad = get_radiance(src);
  return ColorAtUV(src->basis, srcrad, us, vs);
}

static void PropagateRadianceToSurface(ELEMENT *rcv, double ur, double vr, 
				       COLOR raypow, ELEMENT *src, 
				       double fraction, double weight)
{
  int i;
  for (i=0; i<rcv->basis->size; i++) {
    double dual = rcv->basis->dualfunction[i](ur,vr) / rcv->area;
    double w = dual * fraction / (double)nr_rays;
    COLORADDSCALED(rcv->received_rad[i], w, raypow, rcv->received_rad[i]);
  }
}

static void PropagateRadianceToClusterIsotropic(ELEMENT *cluster, COLOR raypow, ELEMENT *src,
				       double fraction, double weight)
{
  double w = fraction / cluster->area / (double)nr_rays;
  COLORADDSCALED(cluster->received_rad[0], w, raypow, cluster->received_rad[0]);
}

static void PropagateRadianceToClusterOriented(ELEMENT *cluster, COLOR raypow, RAY *ray, float dir,
					       ELEMENT *src, double projarea,
					       double fraction, double weight)
{
  REC_ForAllClusterSurfaces(rcv, cluster) {
    double c = -dir * VECTORDOTPRODUCT(rcv->pog.patch->normal, ray->dir);
    if (c > 0.) {
      double afrac = fraction * (c * rcv->area / projarea);
      double w = afrac / rcv->area / (double)nr_rays;
      COLORADDSCALED(rcv->received_rad[0], w, raypow, rcv->received_rad[0]);
    }
  } REC_EndForAllClusterSurfaces;
}

static double ReceiverProjectedArea(ELEMENT *cluster, RAY *ray, float dir)
{
  double area = 0.;
  REC_ForAllClusterSurfaces(rcv, cluster) {
    double c = -dir * VECTORDOTPRODUCT(rcv->pog.patch->normal, ray->dir);
    if (c > 0.) area += c * rcv->area;
  } REC_EndForAllClusterSurfaces;
  return area;
}


static void PropagateRadiance(ELEMENT *src, double us, double vs,
			      ELEMENT *rcv, double ur, double vr,
			      double src_prob, double rcv_prob, 
			      RAY *ray, float dir)
{
  COLOR rad, raypow;
  double area,
    weight = sum_probs/src_prob,		
    fraction = src_prob/(src_prob+rcv_prob);	

  if (src_prob < EPSILON*EPSILON 
      || fraction < EPSILON) 
    return;

  rad = GetSourceRadiance(src, us, vs);
  if (mcr.constant_control_variate) {
    COLORSUBTRACT(rad, mcr.control_radiance, rad);
  }
  COLORSCALE(weight, rad, raypow);

  if (!rcv->iscluster) {
    PropagateRadianceToSurface(rcv, ur, vr, raypow, src, fraction, weight);
  } else {
    switch (hierarchy.clustering) {
    case NO_CLUSTERING:
      Fatal(-1, "Propagate", "Refine() returns cluster although clustering is disabled.\n");
      break;
    case ISOTROPIC_CLUSTERING:
      PropagateRadianceToClusterIsotropic(rcv, raypow, src, fraction, weight);
      break;
    case ORIENTED_CLUSTERING:
      area = ReceiverProjectedArea(rcv, ray, dir);
      if (area > EPSILON)
	PropagateRadianceToClusterOriented(rcv, raypow, ray, dir, src, area, fraction, weight);
      break;
    default:
      Fatal(-1, "Propagate", "Invalid clustering mode %d\n", (int)hierarchy.clustering);
    }
  }
}


static void PropagateImportance(ELEMENT *src, double us, double vs,
				ELEMENT *rcv, double ur, double vr,
				double src_prob, double rcv_prob, 
				RAY *ray, float dir)
{
#ifdef IDMCR
  double w = sum_probs/(src_prob+rcv_prob) / rcv->area / (double)nr_rays;
  rcv->received_imp += w * ElementScalarReflectance(src) * get_importance(src);
#endif

  if (hierarchy.do_h_meshing || hierarchy.clustering != NO_CLUSTERING)
    Fatal(-1, "Propagate", "Importance propagation not implemented in combination with hierarchical refinement");
}


static void RefineAndPropagateRadiance(ELEMENT *src, double us, double vs,
				       ELEMENT *P, double up, double vp,
				       ELEMENT *Q, double uq, double vq,
				       double src_prob, double rcv_prob,
				       RAY *ray, float dir)
{
  LINK link;
  link = TopLink(Q, P);
  Refine(&link, Q, &uq, &vq, P, &up, &vp, hierarchy.oracle);
  
  PropagateRadiance(src, us, vs, link.rcv, uq, vq, src_prob, rcv_prob, ray, dir);
}

static void RefineAndPropagateImportance(ELEMENT *src, double us, double vs,
					 ELEMENT *P, double up, double vp,
					 ELEMENT *Q, double uq, double vq,
					 double src_prob, double rcv_prob,
					 RAY *ray, float dir)
{
  
  PropagateImportance(P, up, vp, Q, uq, vq, src_prob, rcv_prob, ray, dir);
}


static void RefineAndPropagate(ELEMENT *P, double up, double vp,
			       ELEMENT *Q, double uq, double vq,
			       RAY *ray)
{
  double src_prob;
  double us=up, vs=vp;
  ELEMENT *src = RegularLeafElementAtPoint(P, &us, &vs);
  src_prob = (double)src->prob/(double)src->area;
  if (mcr.bidirectional_transfers) {
    double rcv_prob;
    double ur=uq, vr=vq;
    ELEMENT *rcv = RegularLeafElementAtPoint(Q, &ur, &vr);
    rcv_prob = (double)rcv->prob/(double)rcv->area;

    if (get_radiance) {
      RefineAndPropagateRadiance(src, us, vs, P, up, vp, Q, uq, vq, src_prob, rcv_prob, ray, +1);
      RefineAndPropagateRadiance(rcv, ur, vr, Q, uq, vq, P, up, vp, rcv_prob, src_prob, ray, -1);
    }
    if (get_importance) {
      RefineAndPropagateImportance(src, us, vs, P, up, vp, Q, uq, vq, src_prob, rcv_prob, ray, +1);
      RefineAndPropagateImportance(rcv, ur, vr, Q, uq, vq, P, up, vp, rcv_prob, src_prob, ray, -1);
    }
  } else {
    if (get_radiance)
      RefineAndPropagateRadiance(src, us, vs, P, up, vp, Q, uq, vq, src_prob, 0., ray, +1);
    if (get_importance)
      RefineAndPropagateImportance(src, us, vs, P, up, vp, Q, uq, vq, src_prob, 0., ray, +1);
  }
}

#ifdef NEVER

static unsigned *NextRandomInRange(unsigned *index, int dir,
				   int nmsb, unsigned msb1, unsigned rmsb2)
{
  static unsigned xi[4];
  unsigned bits = (1U<<NBITS)-1;
  
  {static int wgiv=FALSE;
  if (!wgiv) fprintf(stderr, "%s %d: WARNING: RANDOM SAMPLING!!\n", __FILE__, __LINE__);
  wgiv = TRUE;
  }
  
  Assert(!hierarchy.do_h_meshing, "Don't use random samples with hierarchical refinement!");
  xi[0] = (unsigned)lrand48() & bits;
  xi[1] = (unsigned)lrand48() & bits;
  xi[2] = (unsigned)lrand48() & bits;
  xi[3] = (unsigned)lrand48() & bits;
  return xi;
}
#endif

static double *NextSample(ELEMENT *elem,
			  int nmsb, niedindex msb1, niedindex rmsb2,
			  double *zeta)
{
  niedindex *xi = NULL, u, v;
  
  niedindex *ray_index = get_radiance ? &elem->ray_index : &elem->imp_ray_index;

  xi = NextNiedInRange(ray_index, +1, nmsb, msb1, rmsb2);
#ifdef NEVER
  switch (mcr.sequence) {
  case S4D_NIEDERREITER:
    xi = NextNiedInRange(ray_index, +1, nmsb, msb1, rmsb2);
    break;
  case S4D_RANDOM:
    xi = NextRandomInRange(ray_index, +1, nmsb, msb1, rmsb2);
    break;
  default:
    if (hierarchy.do_h_meshing)		
      Fatal(-1, "NextSample", "Sampling sequence not yet implemented for stochastic Jacobi radiosity with hierarchical refinement");
    else {
      double *zeta = Sample4D(*ray_index);
      static unsigned xi1[4];
      xi1[0] = (unsigned)(zeta[0] * RECIP1);
      xi1[1] = (unsigned)(zeta[1] * RECIP1);
      xi1[2] = (unsigned)(zeta[2] * RECIP1);
      xi1[3] = (unsigned)(zeta[3] * RECIP1);
      xi = xi1;
    }
  }
#endif

  (*ray_index)++;
  u = (xi[0] & ~3) | 1;		
  v = (xi[1] & ~3) | 1;
  if (elem->nrvertices == 3) FoldSample(&u, &v);
  zeta[0] = (double)u * RECIP;
  zeta[1] = (double)v * RECIP;
  zeta[2] = (double)xi[2] * RECIP;
  zeta[3] = (double)xi[3] * RECIP;
  return zeta;
}


static void UniformHitCoordinates(HITREC *hit, double *uhit, double *vhit)
{
  if (hit->flags & HIT_UV) {	
    *uhit = hit->uv.u; *vhit = hit->uv.v;
    if (hit->patch->jacobian)
      BilinearToUniform(hit->patch, uhit, vhit);
  } else
    PatchUniformUV(hit->patch, &hit->point, uhit, vhit);

  
  if (*uhit < EPSILON) *uhit = EPSILON;
  if (*vhit < EPSILON) *vhit = EPSILON;
  if (*uhit > 1.-EPSILON) *uhit = 1.-EPSILON;
  if (*vhit > 1.-EPSILON) *vhit = 1.-EPSILON;
}


static void ElementShootRay(ELEMENT *src,
			    int nmsb, niedindex msb1, niedindex rmsb2)
{
  RAY ray;
  HITREC *hit, hitstore;
  double zeta[4];

  if (get_radiance) mcr.traced_rays++;
  if (get_importance) mcr.imp_traced_rays++;

  ray = McrGenerateLocalLine(src->pog.patch,
			     NextSample(src, nmsb, msb1, rmsb2, zeta));
  hit = McrShootRay(src->pog.patch, &ray, &hitstore);
  if (hit) {
    double uhit=0., vhit=0.;
    UniformHitCoordinates(hit, &uhit, &vhit);
    RefineAndPropagate(TOPLEVEL_ELEMENT(src->pog.patch), zeta[0], zeta[1],
		       TOPLEVEL_ELEMENT(hit->patch), uhit, vhit, &ray);
  } else
    mcr.nrmisses++;
}

static void InitPushRayIndex(ELEMENT *elem)
{
  elem->ray_index = elem->parent->ray_index;
  elem->imp_ray_index = elem->parent->imp_ray_index;
  ForAllChildrenElements(elem, InitPushRayIndex);
}


static void ElementShootRays(ELEMENT *elem, int rays_this_elem)
{
  int i;
  int nmsb; 		
  niedindex msb1, rmsb2;	

  
  ElementRange(elem, &nmsb, &msb1, &rmsb2);

  
  for (i=0; i<rays_this_elem; i++)
    ElementShootRay(elem, nmsb, msb1, rmsb2);

  if (!ElementIsLeaf(elem)) {
    
    ForAllChildrenElements(elem, InitPushRayIndex);
  }
}


static void ShootRays(void)
{
  double rnd = drand48();
  long ray_count = 0;
  double p_cumul = 0.0;

  
  ForAllPatches(P, Patches) {
    REC_ForAllSurfaceLeafs(leaf, TOPLEVEL_ELEMENT(P)) {
      double p = leaf->prob / sum_probs;
      long rays_this_leaf =
	(long)floor((p_cumul+p) * (double)nr_rays + rnd) - ray_count;

      if (rays_this_leaf > 0)
	ElementShootRays(leaf, rays_this_leaf);

      p_cumul += p;
      ray_count += rays_this_leaf;
    } REC_EndForAllSurfaceLeafs;
  } EndForAll;

  fprintf(stderr, "\n");
}


static void UpdateElement(ELEMENT *elem)
{
  if (get_radiance) {
    if (do_control_variate) {
      
      COLORADD(elem->received_rad[0], mcr.control_radiance, elem->received_rad[0]);
    }
    
    MULTCOEFFICIENTS(elem->Rd, elem->received_rad, elem->basis);
  }

  reflect(elem, (double)nr_rays/sum_probs);

  COLORADDSCALED(mcr.unshot_flux, M_PI*elem->area, elem->unshot_rad[0], mcr.unshot_flux);
  COLORADDSCALED(mcr.total_flux, M_PI*elem->area, elem->rad[0], mcr.total_flux);
#ifdef IDMCR
  COLORADDSCALED(mcr.imp_unshot_flux, M_PI*elem->area*(elem->imp - elem->source_imp), elem->unshot_rad[0], mcr.imp_unshot_flux);
  mcr.unshot_ymp += elem->area * fabs(elem->unshot_imp);
  mcr.total_ymp += elem->area * elem->imp;
#endif
}

static void Push(ELEMENT *parent, ELEMENT *child)
{
  if (get_radiance) {
    COLOR Rd; COLORCLEAR(Rd);

    if (parent->iscluster && !child->iscluster) {
      
      COLOR rad = parent->received_rad[0];
      Rd = child->Rd;
      COLORPROD(Rd, rad, rad);
      PushRadiance(parent, child, &rad, child->received_rad);
    } else
      PushRadiance(parent, child, parent->received_rad, child->received_rad);
  }

#ifdef IDMCR
  if (get_importance)
    PushImportance(parent, child, &parent->received_imp, &child->received_imp);
#endif
}

static void Pull(ELEMENT *parent, ELEMENT *child)
{
  if (get_radiance) {
    PullRadiance(parent, child, parent->rad, child->rad);
    PullRadiance(parent, child, parent->unshot_rad, child->unshot_rad);
  }
#ifdef IDMCR
  if (get_importance) {
    PullImportance(parent, child, &parent->imp, &child->imp);
    PullImportance(parent, child, &parent->unshot_imp, &child->unshot_imp);
  }
#endif
}


static void ClearElement(ELEMENT *parent)
{
  if (get_radiance) {
    CLEARCOEFFICIENTS(parent->rad, parent->basis);
    CLEARCOEFFICIENTS(parent->unshot_rad, parent->basis);
  }
#ifdef IDMCR
  if (get_importance) {
    parent->imp = parent->unshot_imp = 0.;
  }
#endif
}

static void PushUpdatePull(ELEMENT *elem);
static void PushUpdatePullChild(ELEMENT *child)
{
  ELEMENT *parent = child->parent;
  Push(parent, child);
  PushUpdatePull(child);
  Pull(parent, child);
}

static void PushUpdatePull(ELEMENT *elem)
{
  if (ElementIsLeaf(elem)) {
    UpdateElement(elem);
  } else {	
    ClearElement(elem);
    ForAllChildrenElements(elem, PushUpdatePullChild);
  }
}

static void PullRdEd(ELEMENT *elem);
static void PullRdEdFromChild(ELEMENT *child)
{
  ELEMENT *parent = child->parent;

  PullRdEd(child);

  COLORADDSCALED(parent->Ed, child->area/parent->area, child->Ed, parent->Ed);
  COLORADDSCALED(parent->Rd, child->area/parent->area, child->Rd, parent->Rd);
  if (parent->iscluster)
    COLORSETMONOCHROME(parent->Rd, 1.);
}

static void PullRdEd(ELEMENT *elem)
{
  if (ElementIsLeaf(elem) || (!elem->iscluster && !ElementIsTextured(elem)))
    return;

  COLORCLEAR(elem->Ed); COLORCLEAR(elem->Rd);
  ForAllChildrenElements(elem, PullRdEdFromChild);
}

static void PushUpdatePullSweep(void)
{
  
  COLORCLEAR(mcr.unshot_flux);	mcr.unshot_ymp = 0.;
  COLORCLEAR(mcr.total_flux);	mcr.total_ymp = 0.;
  COLORCLEAR(mcr.imp_unshot_flux);

  
  PullRdEd(hierarchy.topcluster);

  PushUpdatePull(hierarchy.topcluster);
}


void DoStochasticJacobiIteration(long nr_rays,
				 COLOR *(*GetRadiance)(ELEMENT *),
				 float (*GetImportance)(ELEMENT *),
				 void (*Update)(ELEMENT *P, double w))
{
  InitGlobals(nr_rays, GetRadiance, GetImportance, Update);
  PrintMessage(nr_rays);
  if (!Setup()) return;
  ShootRays();
  PushUpdatePullSweep();
}
