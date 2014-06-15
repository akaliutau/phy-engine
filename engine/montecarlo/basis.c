

#include <stdio.h>
#include <string.h>

#include "Boolean.h"
#include "basis.h"
#include "element.h"
#include "volume.h"
#include "error.h"

BASIS basis[NR_ELEMENT_TYPES][NR_APPROX_TYPES];
static int inited = FALSE;

BASIS dummyBasis = {
  "dummy basis",
  0, NULL, NULL
};

static double c(double u, double v)
{
  return 1;
}

static double (*f[1])(double, double) = { c };

BASIS clusterBasis = {
  "cluster basis",
  1, f, f
};

struct APPROXDESC approxdesc[NR_APPROX_TYPES] = {
  {"constant"		,  1},
  {"linear"		,  3},
  {"bilinear"		,  4},
  {"quadratic"		,  6},
  {"cubic"		, 10}
};

static BASIS MakeBasis(ELEMENT_TYPE et, APPROX_TYPE at)
{
  BASIS basis = quadBasis;
  char desc[100], *elem=NULL;

  switch (et) {
  case ET_TRIANGLE: 	basis = triBasis; elem="triangles"; break;
  case ET_QUAD: 	basis = quadBasis; elem = "quadrilaterals"; break;
  default: Fatal(-1, "MakeBasis", "Invalid element type %d", et);
  }

  basis.size = approxdesc[at].basis_size;

  sprintf(desc, "%s orthonormal basis for %s", approxdesc[at].name, elem);
  basis.description = strdup(desc);

  return basis;
}

void PrintBasis(BASIS *basis)
{
  int i;
  double u=0.5, v=0.5;

  fprintf(stderr, "%s, size=%d, samples at (%g,%g): ",
	  basis->description, basis->size, u, v);
  for (i=0; i<basis->size; i++)
    fprintf(stderr, "%g ", basis->function[i](u,v));
  fprintf(stderr, "\n");
}

#ifdef DEBUG
static void PlotBasis(BASIS *basis, int tri)
{
  FILE *fp = stdout;
  int i,j,k;
  for (i=0; i<=20; i++) {
    double u = (double)i/20.;
    for (j=0; j<=20; j++) {
      double v = (double)j/20.;
      fprintf(fp, "%g %g  ", u, v);
      for (k=0; k<basis->size; k++) {
	if (tri!=3 || u+v<=1+EPSILON)
	  fprintf(fp, "%g ", basis->function[k](u,v));
	else
	  fprintf(fp, "%g ", 0.); 
      }
      fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
  }
}

static void PlotBases(void)
{
  PlotBasis(&triBasis, 3);
}
#endif


static void ComputeFilterCoefficients(BASIS *parent_basis, int parent_size,
				      BASIS *child_basis, int child_size,
				      TRANSFORM2D *upxfm, CUBARULE *cr,
				      FILTER *filter)
{
  int a, b, k;
  double x;

  for (a=0; a<parent_size; a++) {
    for (b=0; b<child_size; b++) {
      x = 0.;
      for (k=0; k<cr->nrnodes; k++) {
	POINT2D up;
	up.u = cr->u[k];
	up.v = cr->v[k];
	TRANSFORM_POINT_2D((*upxfm), up, up);
	x += cr->w[k] * parent_basis->function[a](up.u, up.v) * 
	                  child_basis->function[b](cr->u[k], cr->v[k]);
      }
      (*filter)[a][b] = x;
    }
  }
}


static void ComputeRegularFilterCoefficients(BASIS *basis, TRANSFORM2D *upxfm, 
					     CUBARULE *cr)
{
  int s;

  for (s=0; s<4; s++) {
    ComputeFilterCoefficients(basis, basis->size, basis, basis->size, 
			      &upxfm[s], cr, &(*basis->regular_filter)[s]);
  }
}

#ifdef TEST

static void TestRegularFilterCoefficients(BASIS *basis, int size)
{
  int a1, a2, b, s;
  double x;

  fprintf(stderr, "basis '%s', size %d:\n", basis->description, size);
  for (a1=0; a1<size; a1++) {
    for (a2=0; a2<size; a2++) {
      x = 0.;
      for (s=0; s<4; s++) {
	for (b=0; b<size; b++) {
	  x += (*basis->regular_filter)[s][a1][b] *
	                    (*basis->regular_filter)[s][a2][b];
	}
      }
      fprintf(stderr, "%+.3f ", x/4.);
    }
    fprintf(stderr, "\n");
  }
}
#endif 

void InitBasis(void)
{
  int et, at;

  if (inited) return;

  ComputeRegularFilterCoefficients(&triBasis, triupxfm, &CRT8);
  ComputeRegularFilterCoefficients(&quadBasis, quadupxfm, &CRQ8);
#ifdef TEST
{ int size;
  for (size=1; size<=triBasis.size; size++)
    TestRegularFilterCoefficients(&triBasis, size);
  for (size=1; size<=quadBasis.size; size++)
     TestRegularFilterCoefficients(&quadBasis, size);
}
#endif 

  for (et=0; et<NR_ELEMENT_TYPES; et++)
    for (at=0; at<NR_APPROX_TYPES; at++)
      basis[et][at] = MakeBasis((ELEMENT_TYPE)et, (APPROX_TYPE)at);
  
  inited = TRUE;
}

COLOR ColorAtUV(BASIS *basis, COLOR *rad, double u, double v)
{
  int i;
  COLOR res; COLORCLEAR(res);
  for (i=0; i<basis->size; i++) {
    double s = basis->function[i](u,v);
    COLORADDSCALED(res, s, rad[i], res);
  }
  return res;
}


void FilterColorDown(COLOR *parent, FILTER *h, COLOR *child, int n)
{
  int a, b;

  for (b=0; b<n; b++) {
    for (a=0; a<n; a++) {
      COLORADDSCALED(child[b], (*h)[a][b], parent[a], child[b]);
    }
  }
}

void FilterColorUp(COLOR *child, FILTER *h, COLOR *parent, int n, double areafactor)
{
  int a, b;

  for (a=0; a<n; a++) {
    for (b=0; b<n; b++) {
      double H = (*h)[a][b] * areafactor;
      COLORADDSCALED(parent[a], H, child[b], parent[a]);
    }
  }
}
