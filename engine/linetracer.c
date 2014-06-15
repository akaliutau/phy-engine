

#include "linetracer.h"

#include "scene.h"
#include "extmath.h"
#include "render.h"
#include "mcsampling/nied31.h"
#include "error.h"


VECTOR glin_Center;
float glin_Radius;


static void GenerateBoundingSphere(void)
{
  float *bbx = WorldGrid->bounds;
  VECTOR diagonal;
  VECTORSET(glin_Center,
	    (bbx[MIN_X] +  bbx[MAX_X])/2,   
	    (bbx[MIN_Y] +  bbx[MAX_Y])/2,
	    (bbx[MIN_Z] +  bbx[MAX_Z])/2);
  VECTORSET(diagonal,
	    bbx[MAX_X] -  bbx[MIN_X],   
	    bbx[MAX_Y] -  bbx[MIN_Y],
	    bbx[MAX_Z] -  bbx[MIN_Z] );
  glin_Radius =  VECTORNORM(diagonal)/2;
}

static double *DefSample4D(int n)
{
  unsigned int *zeta = Nied31(n);
  static double xi[4];
  xi[0] = zeta[0] * RECIP;
  xi[1] = zeta[1] * RECIP;
  xi[2] = zeta[2] * RECIP;
  xi[3] = zeta[3] * RECIP;
  return xi;
}
static double* (*Sample4D)(int n) = DefSample4D;


static RAY GenerateGlobalLine(int n)
{
  RAY ray;
  VECTOR p1, p2;
  double alea;
  double direc1,direc2;
  double *xi;

  xi = Sample4D(n);

  alea = 1.0-2.0*xi[0];
  direc1 = 2.0*M_PI*xi[1];
  direc2 = acos(alea);

  VECTORSET(p1,cos(direc1)*sin(direc2),  
	       sin(direc1)*sin(direc2),
	       cos(direc2));
  VECTORSCALE(glin_Radius, p1,p1);
  VECTORADD(glin_Center,p1,p1);

  alea = 1.0-2.0*xi[2];
  direc1 = 2.0*M_PI*xi[3];
  direc2 = acos(alea);

  VECTORSET(p2,cos(direc1)*sin(direc2),  
	    sin(direc1)*sin(direc2),
	    cos(direc2));
  VECTORSCALE(glin_Radius, p2,p2);
  VECTORADD(glin_Center,p2,p2);

  ray.pos = p1; 
  VECTORSUBTRACT(p2, p1, ray.dir);
  
  
  return ray;
}

static HITLIST *TraceGlobalLine(RAY *ray)
{
  return AllGridIntersections((HITLIST *)NULL, WorldGrid, ray, -EPSILON, 1.+EPSILON, HIT_FRONT|HIT_BACK|HIT_PATCH|HIT_POINT);
  
}


#define IsFrontalHit(hit)	(hit->flags & HIT_FRONT)


int DefCompareHits(HITREC *hit1, HITREC *hit2)
{
  int code=0;

  if (hit1->dist < hit2->dist-EPSILON)
    code = -1;
  else if (hit1->dist > hit2->dist+EPSILON)
    code = +1;
  else {
    
    if (IsFrontalHit(hit1) && !IsFrontalHit(hit2))
      code = -1;	
    else if (!IsFrontalHit(hit1) && IsFrontalHit(hit2))
      code = +1;
    else
      code = 0;		
  }
  return code;
}
static int (*CompareHits)(HITREC*, HITREC*) = DefCompareHits;


int ProcessSpans(HITLIST *hits, void (*do_span)(HITREC *start, HITREC *end))
{
  int nrspans = 0;
  HITREC *start = NULL;
  ForAllHits(hit, hits) {
    if (IsFrontalHit(hit)) {
      if (start) {
	do_span(start, hit); nrspans++;
	start = (HITREC *)NULL;
      }
      
    } else
      start = hit;
  } EndForAll;
  return nrspans;
}


void PrintSpan(HITREC *start, HITREC *end)
{
  fprintf(stderr, "start = "); PrintHit(stderr, start);
  fprintf(stderr, "end   = "); PrintHit(stderr, end);
}


void DefProcessHits(HITLIST *hits)
{
  ProcessSpans(hits, PrintSpan);
}
static void (*ProcessHits)(HITLIST *hits) = DefProcessHits;

static int inited = FALSE;


void InitGlobalLines(double* (*sample4d)(int index),
		     int (*compare_hits)(HITREC *hit1, HITREC *hit2),
		     void (*process_hits)(HITLIST *hits))
{
  if (!World) {
    Error("InitGlobalLines", "No world!");
    inited = FALSE;
    return;
  }
  inited = TRUE;
  GenerateBoundingSphere();
  Sample4D = sample4d ? sample4d : DefSample4D;
  CompareHits = compare_hits;
  ProcessHits = process_hits;
}


int DoGlobalLine(int index)
{
  RAY ray;
  HITLIST *hits;

  if (!inited) {
    Fatal(-1, "DoGlobalLine", "Call InitGlobalLines() first before using DoGlobalLine()");
  }

  ray = GenerateGlobalLine(index);
  hits = TraceGlobalLine(&ray);
  if (!hits)
    return FALSE;

  if (CompareHits) {
    hits = HitListSort(hits, CompareHits);
  }

  if (ProcessHits) {
    ProcessHits(hits);
  }

  DestroyHitlist(hits);

  return TRUE;
}
