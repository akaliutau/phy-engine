

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readmgf.h"
#include "scene.h"
#include "geom.h"
#include "compound.h"
#include "error.h"
#include "cie.h"
#include "vectoroctree.h"
#include "namedvertex.h"
#include "vertex.h"
#include "patch.h"
#include "phong.h"
#include "splitbsdf.h"
#include "options.h"
#include "defaults.h"
#include "fileopts.h"

#include "MGF/parser.h"

void MgfDefaults(void)
{
}

void ParseMgfOptions(int *argc, char **argv)
{
  
}

void PrintMgfOptions(FILE *fp)
{
  
}


static VECTOROCTREE *globalPoints, 	
                    *globalNormals;	
static NAMEDVERTEXTREE *globalVertices;	


static VECTORLIST *currentPointList, *currentNormalList;
static VERTEXLIST *currentVertexList, *autoVertexList;
static PATCHLIST *currentFaceList;
static GEOMLIST	*currentGeomList;
static MATERIAL *currentMaterial;

#ifndef MAXGEOMSTACKDEPTH
#define MAXGEOMSTACKDEPTH	100	
#endif


static GEOMLIST *geomStack[MAXGEOMSTACKDEPTH], **geomStackPtr;
static VERTEXLIST *autoVertexListStack[MAXGEOMSTACKDEPTH], **autoVertexListStackPtr;

#ifndef MAXFACEVERTICES
#define MAXFACEVERTICES	       100	
#endif

static int incomplex = FALSE, 		
           insurface = FALSE,		
           all_surfaces_sided = FALSE;	



void MgfSetNrQuartCircDivs(int divs)
{
  if (divs <= 0) {
    Error(NULL, "Number of quarter circle divisions (%d) should be positive", divs);
    return;
  }

  mg_nqcdivs = divs;
}


void MgfSetIgnoreSidedness(int yesno)
{
  all_surfaces_sided = yesno;
}

void MgfSetMonochrome(int yesno)
{
  monochrome = yesno;
}

static void do_error(char *errmsg)
{
  Error(NULL, "%s line %d: %s", mg_file->fname, mg_file->lineno, errmsg);
}

static void do_warning(char *errmsg)
{
  Warning(NULL, "%s line %d: %s", mg_file->fname, mg_file->lineno, errmsg);
}

static void NewSurface(void)
{
  currentPointList = VectorListCreate();
  currentNormalList = VectorListCreate();
  currentVertexList = VertexListCreate();	
  currentFaceList = PatchListCreate();
  insurface = TRUE;
}

static void SurfaceDone(void)
{
  GEOM *thegeom;

  if (currentFaceList) {
    thegeom = GeomCreate((void *)SurfaceCreate(currentMaterial, currentPointList, currentNormalList, (VECTORLIST*)NULL, currentVertexList, currentFaceList, NO_COLORS), SurfaceMethods()); 
    currentGeomList = GeomListAdd(currentGeomList, thegeom);
  }
  insurface = FALSE;
}


static int MaterialChanged(void)
{
  char *matname;

  matname = c_cmname;
  if (!matname || *matname == '\0')	
    matname = "unnamed";

  
  if (strcmp(matname, currentMaterial->name) == 0 && c_cmaterial->clock == 0)
    return FALSE;

  return TRUE;
}


static void MgfGetColor(C_COLOR *cin, double intensity, COLOR *cout)
{
  float xyz[3], rgb[3];

  c_ccvt(cin, C_CSXY);
  if (cin->cy > EPSILON) {
    xyz[0] = cin->cx / cin->cy * intensity;
    xyz[1] = 1. * intensity;
    xyz[2] = (1.- cin->cx - cin->cy) / cin->cy * intensity;
  } else {
    do_warning("invalid color specification (Y<=0) ... setting to black");
    xyz[0] = xyz[1] = xyz[2] = 0.;
  }

  if (xyz[0]<0. || xyz[1] <0. || xyz[2] < 0.) {
    do_warning("invalid color specification (negative CIE XYZ componenets) ... clipping to zero");
    if (xyz[0] < 0.) xyz[0] = 0.;
    if (xyz[1] < 1.) xyz[1] = 0.;
    if (xyz[2] < 2.) xyz[2] = 0.;
  }

#ifdef RGBCOLORS
  xyz_rgb(xyz, rgb);
  if (clipgamut(rgb))
    do_warning("color desaturated during gamut clipping");
  COLORSET(*cout, rgb[0], rgb[1], rgb[2]);
#else
  if (finite(xyz[0]) && finite(xyz[1]) && finite(xyz[2])) {
    COLORSET(*cout, xyz[0], xyz[1], xyz[2]);
  }
  else {
    do_warning("nonsense colour specified, setting it to black");
    COLORCLEAR(*cout);
  }
#endif
}

float ColorMax(COLOR col)
{
  
#define NUMSMPLS 3
  float samples[NUMSMPLS], mx;
  int i;

#ifdef RGBCOLORS
#define SPEC_SAMPLES(col, rgb) \
  rgb[0] = col.spec[0]; rgb[1] = col.spec[1]; rgb[2] = col.spec[2];
#else
#define SPEC_SAMPLES(col, rgb) \
  xyz_rgb(col.spec, rgb);
#endif
  SPEC_SAMPLES(col, samples);

  mx = -HUGE;
  for (i=0; i<NUMSMPLS; i++)
    if (samples[i] > mx) mx = samples[i];

  return mx;
#undef SPEC_SAMPLES
#undef NUMSMPLS
}


static int GetCurrentMaterial(void)
{
  COLOR Ed, Es, Rd, Td, Rs, Ts, A;
  float Ne, Nr, Nt, a;
  MATERIAL *thematerial;
  char *matname;

  matname = c_cmname;
  if (!matname || *matname == '\0')	
    matname = "unnamed";

  
  if (strcmp(matname, currentMaterial->name) == 0 && c_cmaterial->clock == 0)
    return FALSE;

  if ((thematerial = MaterialLookup(MaterialLib, matname))) {
    if (c_cmaterial->clock == 0) {
      currentMaterial = thematerial;
      return TRUE;
    }
  }

  
  MgfGetColor(&c_cmaterial->ed_c, c_cmaterial->ed, &Ed);
  MgfGetColor(&c_cmaterial->rd_c, c_cmaterial->rd, &Rd);
  MgfGetColor(&c_cmaterial->td_c, c_cmaterial->td, &Td);
  MgfGetColor(&c_cmaterial->rs_c, c_cmaterial->rs, &Rs);
  MgfGetColor(&c_cmaterial->ts_c, c_cmaterial->ts, &Ts);

  
  COLORADD(Rd, Rs, A);
  if ((a = ColorMax(A)) > 1.-EPSILON) {
    do_warning("invalid material specification: total reflectance shall be < 1");
    a = (1.-EPSILON) / a;
    COLORSCALE(a, Rd, Rd);
    COLORSCALE(a, Rs, Rs);
  }

  COLORADD(Td, Ts, A);
  if ((a = ColorMax(A)) > 1.-EPSILON) {
    do_warning("invalid material specification: total transmittance shall be < 1");
    a = (1.-EPSILON) / a;
    COLORSCALE(a, Td, Td);
    COLORSCALE(a, Ts, Ts);
  }

  
  COLORSCALE((1./WHITE_EFFICACY), Ed, Ed);

  COLORCLEAR(Es);
  Ne = 0.;

  
  if (c_cmaterial->rs_a != 0.0) {
    Nr = 0.6/c_cmaterial->rs_a;    
    Nr *= Nr;
  } else
    Nr = 0.0;

  if (c_cmaterial->ts_a != 0.0) {
    Nt = 0.6/c_cmaterial->ts_a;    
    Nt *= Nt;
  } else
    Nt = 0.0;

  if (monochrome) {
    COLORSETMONOCHROME(Ed, ColorGray(Ed));
    COLORSETMONOCHROME(Es, ColorGray(Es));
    COLORSETMONOCHROME(Rd, ColorGray(Rd));
    COLORSETMONOCHROME(Rs, ColorGray(Rs));
    COLORSETMONOCHROME(Td, ColorGray(Td));
    COLORSETMONOCHROME(Ts, ColorGray(Ts));
  }

  thematerial = MaterialCreate(matname, 
			       (COLORNULL(Ed) && COLORNULL(Es)) ? (EDF *)NULL : EdfCreate(PhongEdfCreate(&Ed, &Es, Ne), &PhongEdfMethods),
			       BsdfCreate(SplitBSDFCreate(
			       (COLORNULL(Rd) && COLORNULL(Rs)) ? (BRDF *)NULL : BrdfCreate(PhongBrdfCreate(&Rd, &Rs, Nr), &PhongBrdfMethods),
			       (COLORNULL(Td) && COLORNULL(Ts)) ? (BTDF *)NULL : BtdfCreate(PhongBtdfCreate(&Td, &Ts, Nt, c_cmaterial->nr, c_cmaterial->ni), &PhongBtdfMethods), (TEXTURE*)NULL), &SplitBsdfMethods),
			       all_surfaces_sided ? 1 : c_cmaterial->sided);
  
  MaterialLib = MaterialListAdd(MaterialLib, thematerial); 
  currentMaterial = thematerial;

  
  c_cmaterial->clock = 0;

  return TRUE;
}

static VECTOR *InstallPoint(float x, float y, float z)
{
  VECTOR *coord = VectorCreate(x, y, z);
  currentPointList = VectorListAdd(currentPointList, coord); 
  return coord;
}

static VECTOR *InstallNormal(float x, float y, float z)
{
  VECTOR *norm = VectorCreate(x, y, z);
  currentNormalList = VectorListAdd(currentNormalList, norm);    
  return norm;
}

static VERTEX *InstallVertex(VECTOR *coord, VECTOR *norm, char *name)
{
  VERTEX *v = VertexCreate(coord, norm, (VECTOR*)NULL, PatchListCreate());
  currentVertexList = VertexListAdd(currentVertexList, v);
  return v;
}

static VERTEX *GetVertex(char *name)
{
  C_VERTEX *vp;
  VERTEX *thevertex;

  if ((vp = c_getvert(name)) == NULL)
    return (VERTEX *)NULL;

  thevertex = (VERTEX *)(vp->client_data);
  if (!thevertex || vp->clock>=1 || vp->xid != xf_xid(xf_context) || is0vect(vp->n)) {
    
    FVECT  vert, norm;
    VECTOR *thenormal;
    VECTOR *thepoint;

    xf_xfmpoint(vert, vp->p);
    thepoint = InstallPoint(vert[0], vert[1], vert[2]);
    if (is0vect(vp->n)) {
      thenormal = (VECTOR *)NULL;
    } else {
      xf_xfmvect(norm, vp->n);
      thenormal = InstallNormal(norm[0], norm[1], norm[2]);
    }
    thevertex = InstallVertex(thepoint, thenormal, name);
    vp->client_data = (void *)thevertex;
    vp->xid = xf_xid(xf_context);
  }
  vp->clock = 0;

  return thevertex;
}


static VERTEX *GetBackFaceVertex(VERTEX *v, char *name)
{
  VERTEX *back = v->back;

  if (!back) {
    VECTOR *the_point, *the_normal;  

    the_point = v->point;
    the_normal = v->normal;
    if (the_normal) 
      the_normal = InstallNormal(-the_normal->x, -the_normal->y, -the_normal->z);

    back = v->back = InstallVertex(the_point, the_normal, name);
    back->back = v;
  }

  return back;
}

static PATCH *NewFace(VERTEX *v1, VERTEX *v2, VERTEX *v3, VERTEX *v4, VECTOR *normal)
{
  PATCH *theface;

  if (xf_context && xf_context->rev) {
    theface = PatchCreate(v4 ? 4 : 3, v3, v2, v1, v4);
  } else {
    theface = PatchCreate(v4 ? 4 : 3, v1, v2, v3, v4);
  }

  if (theface)
    currentFaceList = PatchListAdd(currentFaceList, theface);

  return theface;
}


static VECTOR *FaceNormal(int nrvertices, VERTEX **v, VECTOR *normal)
{
  double norm;
  DVECTOR prev, cur;
  VECTOR n;
  int i;

  VECTORSET(n,0,0,0);
  VECTORSUBTRACT(*(v[nrvertices -1]->point),*(v[0]->point),cur);
  for (i=0; i<nrvertices; i++) {
    prev = cur;
    VECTORSUBTRACT(*(v[i]->point),*(v[0]->point),cur);
    n.x += (prev.y - cur.y) * (prev.z + cur.z);
    n.y += (prev.z - cur.z) * (prev.x + cur.x);
    n.z += (prev.x - cur.x) * (prev.y + cur.y);
  }
  norm = VECTORNORM(n);

  if (norm < EPSILON) {
    
    return (VECTOR *)NULL;
  }
  VECTORSCALEINVERSE(norm, n, n);
  *normal = n;

  return normal;
}


static int FaceIsConvex(int nrvertices, VERTEX **v, VECTOR *normal)
{
  DVEC2D v2d[MAXFACEVERTICES+1], p, c;
  int i, index, sign;
  
  index = VectorDominantCoord(normal);
  for (i=0; i<nrvertices; i++)
    VECTORPROJECT(v2d[i], *(v[i]->point), index);
  
  p.u = v2d[3].u - v2d[2].u;
  p.v = v2d[3].v - v2d[2].v;
  c.u = v2d[0].u - v2d[3].u;
  c.v = v2d[0].v - v2d[3].v;
  sign = (p.u * c.v > c.u * p.v) ? 1 : -1;

  for (i=1; i<nrvertices; i++) {
    p.u = c.u;
    p.v = c.v;
    c.u = v2d[i].u - v2d[i-1].u;
    c.v = v2d[i].v - v2d[i-1].v;
    if (((p.u * c.v > c.u * p.v) ? 1 : -1) != sign)
      return FALSE;
  }
  
  return TRUE;
}


static int PointInsideTriangle2D(VEC2D *p, VEC2D *p1, VEC2D *p2, VEC2D *p3)
{
  double u0, v0, u1, v1, u2, v2, a, b;

  
  u0 = p->u - p1->u;
  v0 = p->v - p1->v;
  u1 = p2->u - p1->u;
  v1 = p2->v - p1->v;
  u2 = p3->u - p1->u;
  v2 = p3->v - p1->v;
  
  a=10.; b=10.;		
  if (fabs(u1) < EPSILON) {
    if (fabs(u2)>EPSILON && fabs(v1)>EPSILON) {
      b = u0/u2;
      if (b<EPSILON || b>1.-EPSILON)
	return FALSE;
      else
	a = (v0 - b*v2)/v1;
    }
  } else {
    b = v2*u1 - u2*v1;
    if (fabs(b)>EPSILON) {
      b = (v0*u1 - u0*v1) / b;
      if (b<EPSILON || b>1.-EPSILON)
	return FALSE;
      else
	a = (u0 - b*u2)/u1;
    }
  }

  return (a>=EPSILON && a<=1.-EPSILON && (a+b)<=1.-EPSILON);	
}


static int SegmentsIntersect2D(VEC2D *p1, VEC2D *p2, VEC2D *p3, VEC2D *p4)
{
  double a, b, c, du, dv, r1, r2, r3, r4;
  int colinear = FALSE;

  
  du = fabs(p2->u - p1->u);
  dv = fabs(p2->v - p1->v);
  if (du > EPSILON || dv > EPSILON) {
    if (dv > du) {
      a = 1.0;
      b = - (p2->u - p1->u) / (p2->v - p1->v);
      c = - (p1->u + b * p1->v);
    } else {
      a = - (p2->v - p1->v) / (p2->u - p1->u);
      b = 1.0;
      c = - (a * p1->u + p1->v);
    }
    
    r3 = a * p3->u + b * p3->v + c;
    r4 = a * p4->u + b * p4->v + c;
    
    if (fabs(r3) < EPSILON && fabs(r4) < EPSILON)
      colinear = TRUE;
    else if ((r3 > -EPSILON && r4 > -EPSILON) || (r3 < EPSILON && r4 < EPSILON))
      return FALSE;
  }

  if (!colinear) {
    du = fabs(p4->u - p3->u);
    dv = fabs(p4->v - p3->v);
    if (du > EPSILON || dv > EPSILON) {
      if (dv > du) {
	a = 1.0;
	b = - (p4->u - p3->u) / (p4->v - p3->v);
	c = - (p3->u + b * p3->v);
      } else {
	a = - (p4->v - p3->v) / (p4->u - p3->u);
	b = 1.0;
	c = - (a * p3->u + p3->v);
      }
      
      r1 = a * p1->u + b * p1->v + c;
      r2 = a * p2->u + b * p2->v + c;
      
      if (fabs(r1) < EPSILON && fabs(r2) < EPSILON)
	colinear = TRUE;
      else if ((r1 > -EPSILON && r2 > -EPSILON) || (r1 < EPSILON && r2 < EPSILON))
	return FALSE;		
    }
  }
  
  if (!colinear)
    return TRUE;
  
  return FALSE;	
}


static int do_complex_face(int n, VERTEX **v, VECTOR *normal, VERTEX **backv, VECTOR *backnormal)
{
  int i, j, max, p0, p1, p2, corners, start, good, index;
  double maxd, d, a;
  VECTOR center;
  char out[MAXFACEVERTICES+1];
  VEC2D q[MAXFACEVERTICES+1];
  VECTOR nn;

  VECTORSET(center, 0., 0., 0.);
  for (i=0; i<n; i++)
    VECTORSUM(center, *(v[i]->point), center);
  VECTORSCALEINVERSE((float)n, center, center);

  VECTORDIST(center, *(v[0]->point), maxd);
  max = 0;
  for (i=1; i<n; i++) {
    VECTORDIST(center, *(v[i]->point), d);
    if (d > maxd) {
      maxd = d;
      max = i;
    }
  }

  for (i=0; i<n; i++)
    out[i] = FALSE;

  p1 = max;
  p0 = p1 - 1;
  if (p0 < 0)
    p0 = n-1;
  p2 = (p1+1) % n;
  VECTORTRIPLECROSSPRODUCT(*(v[p0]->point), *(v[p1]->point), *(v[p2]->point), *normal);
  VECTORNORMALIZE(*normal);
  index = VectorDominantCoord(normal);

  for (i=0; i<n; i++)
    VECTORPROJECT(q[i], *(v[i]->point), index);

  corners = n;
  p0 = -1;
  a = 0.;	
	
  while (corners >= 3) {
    start = p0;

    do {
      p0 = (p0 + 1) % n;
      while (out[p0])
	p0 = (p0 + 1) % n;
      
      p1 = (p0 + 1) % n;
      while (out[p1])
	p1 = (p1 + 1) % n;
      
      p2 = (p1 + 1) % n;
      while (out[p2])
	p2 = (p2 + 1) % n;
      
      if (p0 == start)
	break;
      
      VECTORTRIPLECROSSPRODUCT(*(v[p0]->point), *(v[p1]->point), *(v[p2]->point), nn);
      a = VECTORNORM(nn);
      VECTORSCALEINVERSE(a, nn, nn);
      VECTORDIST(nn, *normal, d);
      
      good = TRUE;
      if (d <= 1.0) {
	for (i=0; i<n && good; i++) {
	  if (out[i] || v[i]==v[p0] || v[i]==v[p1] || v[i]==v[p2]) 
	    continue;
	  
	  if (PointInsideTriangle2D(&q[i], &q[p0], &q[p1], &q[p2])) 
	    good = FALSE;
	  
	  j = (i+1) % n;
	  if (out[j] || v[j]==v[p0]) 
	    continue;
	  
	  if (SegmentsIntersect2D(&q[p2], &q[p0], &q[i], &q[j])) 
	    good = FALSE;
	}
      }
    } while (d > 1.0 || !good);
    
    if (p0 == start) {
      do_error("misbuilt polygonal face");
      return MG_OK; 	
    }

    if (fabs(a) > EPSILON) {	
      PATCH *face, *twin;
      face = NewFace(v[p0], v[p1], v[p2], (VERTEX *)NULL, normal);
      if (!currentMaterial->sided) {
	twin = NewFace(backv[p2], backv[p1], backv[p0], (VERTEX *)NULL, backnormal);
	face->twin = twin;
	twin->twin = face;
      }
    }
    
    out[p1] = TRUE;
    corners--;
  }

  return MG_OK;
}

static int do_face(int argc, char **argv)
{
  VERTEX *v[MAXFACEVERTICES+1], *backv[MAXFACEVERTICES+1];
  VECTOR normal, backnormal;
  PATCH *face, *twin;
  int i, errcode;

  if (argc < 4) {
    do_error("too few vertices in face");
    return MG_OK; 	
  }

  if (argc-1 > MAXFACEVERTICES) {
    do_warning("too many vertices in face. Recompile the program with larger MAXFACEVERTICES constant in readmgf.c");
    return MG_OK;	
  }

  if (!incomplex) {
    if (MaterialChanged()) {
      if (insurface) SurfaceDone();	
      NewSurface();
      GetCurrentMaterial();
    }
  }

  for (i=0; i<argc-1; i++) {
    if ((v[i] = GetVertex(argv[i+1])) == (VERTEX *)NULL)
      return MG_EUNDEF;	
    backv[i] = (VERTEX *)NULL;
    if (!currentMaterial->sided)
      backv[i] = GetBackFaceVertex(v[i], argv[i+1]);
  }

  if (!FaceNormal(argc-1, v, &normal)) {
    do_warning("degenerate face");
    return MG_OK;	
  }
  if (!currentMaterial->sided)
    VECTORSCALE(-1., normal, backnormal);

  errcode = MG_OK;
  if (argc == 4) {		
    face = NewFace(v[0], v[1], v[2], (VERTEX *)NULL, &normal);
    if (!currentMaterial->sided) {
      twin = NewFace(backv[2], backv[1], backv[0], (VERTEX *)NULL, &backnormal);
      face->twin = twin;
      twin->twin = face;
    }
  } else if (argc == 5) {	
    if (incomplex || FaceIsConvex(argc-1, v, &normal)) {
      face = NewFace(v[0], v[1], v[2], v[3], &normal);
      if (!currentMaterial->sided) {
	twin = NewFace(backv[3], backv[2], backv[1], backv[0], &backnormal);
	face->twin = twin;
	twin->twin = face;
      }
    } else 
      errcode = do_complex_face(argc-1, v, &normal, backv, &backnormal);
  } else  			
    errcode = do_complex_face(argc-1, v, &normal, backv, &backnormal);
  
  return errcode;
}


static double FVECT_DistanceSquared(FVECT *v1, FVECT *v2)
{
  FVECT d;

  d[0] = (*v2)[0] - (*v1)[0];
  d[1] = (*v2)[1] - (*v1)[1];
  d[2] = (*v2)[2] - (*v1)[2];
  return (d[0]*d[0] + d[1]*d[1] + d[2]*d[2]);
}


static int do_face_with_holes(int argc, char **argv)
{
  FVECT v[MAXFACEVERTICES+1];	  
  char *nargv[MAXFACEVERTICES+1], 
       copied[MAXFACEVERTICES+1]; 
  int newcontour[MAXFACEVERTICES];
  int i, ncopied;		  

  if (argc-1 > MAXFACEVERTICES) {
    do_warning("too many vertices in face. Recompile the program with larger MAXFACEVERTICES constant in readmgf.c");
    return MG_OK;	
  }

  
  for (i=1; i<argc; i++) {
    C_VERTEX *vp;

    if (*argv[i] == '-') 	
      continue;

    vp = c_getvert(argv[i]);
    if (!vp) 			
      return MG_EUNDEF;
    xf_xfmpoint(v[i], vp->p);	

    copied[i] = FALSE;		
  }

  
  ncopied = 0;
  for (i=1; i<argc && *argv[i]!='-'; i++) {
    newcontour[ncopied++] = i;
    copied[i] = TRUE;
  }

  
  for (; i<argc; i++) {
    if (*argv[i]=='-')
      continue;		
    if (!copied[i])
      break;		
  }

  while (i < argc) {
    
    int nearestcopied, nearestother, first, last, num, j, k;
    double mindist;

    
    nearestcopied = nearestother = 0;
    mindist = HUGE;
    for (j=i; j<argc; j++) {
      if (*argv[j] == '-' || copied[j])
	continue;	
      for (k=0; k<ncopied; k++) {
	double d = FVECT_DistanceSquared(&v[j], &v[newcontour[k]]);
	if (d < mindist) {
	  mindist = d;
	  nearestcopied = k;
	  nearestother = j;
	}
      }
    }

    
    for (first=nearestother; *argv[first]!='-'; first--) {}
    first++;

    
    for (last=nearestother; last<argc && *argv[last]!='-'; last++) {}
    last--;

    
    num = last-first+1;

    
    if (ncopied+num+2 > MAXFACEVERTICES) {
      do_warning("too many vertices in face. Recompile the program with larger MAXFACEVERTICES constant in readmgf.c");
      return MG_OK;	
    }

    
    for (k=ncopied-1; k>=nearestcopied; k--)
      newcontour[k+num+2] = newcontour[k];
    ncopied += num+2;

    
    k = nearestcopied+1;
    for (j=nearestother; j<=last; j++) {
      newcontour[k++] = j;
      copied[j] = TRUE;
    }
    for (j=first; j<=nearestother; j++) {
      newcontour[k++] = j;
      copied[j] = TRUE;
    }

    
    for (; i<argc; i++) {
      if (*argv[i]=='-')
	continue;	
      if (!copied[i])
	break;		
    }
  }

  
  nargv[0] = "f";
  for (i=0; i<ncopied; i++)
    nargv[i+1] = argv[newcontour[i]];

  
  return do_face(ncopied+1, nargv);
}


static int do_discretize(int argc, char **argv)
{
  int en = mg_entity(argv[0]);

  switch (en) {
  case MG_E_SPH: 	return e_sph(argc, argv);
  case MG_E_TORUS: 	return e_torus(argc, argv);
  case MG_E_CYL: 	return e_cyl(argc, argv);
  case MG_E_RING: 	return e_ring(argc, argv);
  case MG_E_CONE: 	return e_cone(argc, argv);
  case MG_E_PRISM: 	return e_prism(argc, argv);
  default:
    Fatal(4, "mgf.c: do_discretize", "Unsupported geometry entity number %d", en);
  }

  return MG_EILL;	
}

static int do_surface(int argc, char **argv)
{
  int errcode;

  if (incomplex) 
    return do_discretize(argc, argv);

  else {
    incomplex = TRUE;
    if (insurface) SurfaceDone();
    NewSurface();
    GetCurrentMaterial();
    
    errcode = do_discretize(argc, argv);
    
    SurfaceDone();
    incomplex = FALSE;
    
    return errcode;
  }
}

static void PushCurrentGeomList(void)
{
  if (geomStackPtr - geomStack >= MAXGEOMSTACKDEPTH) {
    do_error("Objects are nested too deep for this program. Recompile with larger MAXGEOMSTACKDEPTH constant in readmgf.c");
    return;
  } else {
    *geomStackPtr = currentGeomList;
    geomStackPtr ++;
    currentGeomList = GeomListCreate();
    
    *autoVertexListStackPtr = autoVertexList;
    autoVertexListStackPtr ++;
    autoVertexList = VertexListCreate();
  }
}

static void PopCurrentGeomList(void)
{
  if (geomStackPtr <= geomStack) {
    do_error("Object stack underflow ... unbalanced 'o' contexts?");
    currentGeomList = GeomListCreate();
    return;
  } else {
    geomStackPtr --;
    currentGeomList = *geomStackPtr;
    
    VertexListDestroy(autoVertexList);
    autoVertexListStackPtr --;
    autoVertexList = *autoVertexListStackPtr;
  }
}

static int do_object(int argc, char **argv)
{
  int i;

  if (argc > 1) { 	
    for (i=0; i<geomStackPtr - geomStack; i++)
      fprintf(stderr, "\t");
    fprintf(stderr, "%s ...\n", argv[1]);
    
    if (insurface) SurfaceDone();

    PushCurrentGeomList();

    NewSurface();
  } else {		
    GEOM *thegeom = (GEOM *)NULL;

    if (insurface) SurfaceDone();

    if (GeomListCount(currentGeomList) > 0) 
      thegeom = GeomCreate((void *)CompoundCreate(currentGeomList), CompoundMethods());
		
    PopCurrentGeomList();

    if (thegeom)
      currentGeomList = GeomListAdd(currentGeomList, thegeom); 

    NewSurface();
  }

  return obj_handler(argc, argv);
}

static int unknown_count;	

static int do_unknown(int argc, char **argv)
{
  do_warning("unknown entity");
  unknown_count++;

  return MG_OK;
}

static void InitMgf(void)
{
  mg_ehand[MG_E_FACE] = do_face;
  mg_ehand[MG_E_FACEH] = do_face_with_holes;

  mg_ehand[MG_E_VERTEX] = c_hvertex;
  mg_ehand[MG_E_POINT] = c_hvertex;
  mg_ehand[MG_E_NORMAL] = c_hvertex;
  
  mg_ehand[MG_E_COLOR] = c_hcolor;
  mg_ehand[MG_E_CXY] = c_hcolor;
  mg_ehand[MG_E_CMIX] = c_hcolor;


  mg_ehand[MG_E_MATERIAL] = c_hmaterial;
  mg_ehand[MG_E_ED] = c_hmaterial;
  mg_ehand[MG_E_IR] = c_hmaterial;
  mg_ehand[MG_E_RD] = c_hmaterial;
  mg_ehand[MG_E_RS] = c_hmaterial;
  mg_ehand[MG_E_SIDES] = c_hmaterial;
  mg_ehand[MG_E_TD] = c_hmaterial;
  mg_ehand[MG_E_TS] = c_hmaterial;
  
  mg_ehand[MG_E_OBJECT] = do_object;	
  
  mg_ehand[MG_E_XF] = xf_handler;
  
  mg_ehand[MG_E_SPH] = do_surface;
  mg_ehand[MG_E_TORUS] = do_surface;
  mg_ehand[MG_E_RING] = do_surface;
  mg_ehand[MG_E_CYL] = do_surface;
  mg_ehand[MG_E_CONE] = do_surface;
  mg_ehand[MG_E_PRISM] = do_surface;
  
  unknown_count = 0;
  mg_uhand = do_unknown;
  
  mg_init();
}

void ReadMgf(char *filename)
{
  MG_FCTXT fctxt;
  int err;

  MgfSetNrQuartCircDivs(nqcdivs);
  MgfSetIgnoreSidedness(force_onesided_surfaces);
  MgfSetMonochrome(monochrome);

  InitMgf();

  globalPoints = VectorOctreeCreate();
  globalNormals = VectorOctreeCreate();
  globalVertices = NamedVertexTreeCreate();	
  currentGeomList = GeomListCreate();

  MaterialLib = MaterialListCreate();
  currentMaterial = &defaultMaterial;

  geomStackPtr = geomStack;
  autoVertexListStackPtr = autoVertexListStack;
  autoVertexList = VertexListCreate();

  incomplex = FALSE;
  insurface = FALSE;

  NewSurface();

  if (filename[0] == '#') 
    err = mg_open(&fctxt, NULL);
  else
    err = mg_open(&fctxt, filename);
  if (err) 
    do_error(mg_err[err]);
  else {
    while (mg_read() > 0 && !err) {
      err = mg_parse();
      if (err)
	do_error(mg_err[err]);
    }
    mg_close();
  }
  mg_clear();

  if (insurface) SurfaceDone();
  World = currentGeomList;

  VertexListDestroy(autoVertexList);
  VectorOctreeDestroy(globalPoints);
  VectorOctreeDestroy(globalNormals);
  NamedVertexTreeDestroy(globalVertices);
}

