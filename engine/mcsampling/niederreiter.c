

static niedindex nied[DIMEN] = {0,0,0,0}, count = 0;

#ifdef NEVER

#define GRAY(n)	(n ^ (n>>1))

void Nied(long n, double *xi1, double *xi2, double *xi3, double *xi4)
{
  niedindex gray;
  niedindex *cj0=cj[0], *cj1=cj[1], *cj2=cj[2], *cj3=cj[3];

  nied[0] = nied[1] = nied[2] = nied[3] = 0;
  n += SKIP;
  gray =  n;
  while (gray) {
    if (gray&1) {
      nied[0] ^= *cj0;
      nied[1] ^= *cj1;
      nied[2] ^= *cj2;
      nied[3] ^= *cj3;
    }

    cj0++; cj1++; cj2++; cj3++;
    gray >>= 1;
  }

  count = n;

  *xi1 = (double)nied[0] * RECIP;
  *xi2 = (double)nied[1] * RECIP;
  *xi3 = (double)nied[2] * RECIP;
  *xi4 = (double)nied[3] * RECIP;
}

void NextNied(double *xi1, double *xi2, double *xi3, double *xi4)
{
  niedindex i=count++, r=0;

  while (i&1) {i >>= 1; r++;}	      		

  *xi1 = (double)(nied[0] ^= cj[0][r]) * RECIP;
  *xi2 = (double)(nied[1] ^= cj[1][r]) * RECIP;
  *xi3 = (double)(nied[2] ^= cj[2][r]) * RECIP;
  *xi4 = (double)(nied[3] ^= cj[3][r]) * RECIP;
}
#endif 


niedindex *Nied(niedindex n)
{
  niedindex diff;
  niedindex *cj0=cj[0], *cj1=cj[1], *cj2=cj[2], *cj3=cj[3];

  n += SKIP;
  diff = n ^ count;	
  while (diff) {
    if (diff&1) {
      nied[0] ^= *cj0;
      nied[1] ^= *cj1;
      nied[2] ^= *cj2;
      nied[3] ^= *cj3;
    }

    cj0++; cj1++; cj2++; cj3++;    
    diff >>= 1;
  }

  count = n;
  return nied;
}


niedindex *NextNiedInRange(niedindex *idx, int dir,
			  int nmsb, niedindex msb1, niedindex rmsb2)
{
  niedindex mask, rmask, diff, c, i, step;
  niedindex *cj0, *cj1, *cj2, *cj3;

  step = 1<<nmsb;
  mask = step-1;
  rmask = mask << (NBITS-nmsb);
  msb1 &= mask;
  rmsb2 &= rmask;

  i = *idx + SKIP;
  if (dir >= 0) {
    i = (((i&mask) <= msb1 ? i : i+mask) & ~mask) | msb1;
  } else {
    i = (((i&mask) >= msb1 ? i : i-mask) & ~mask) | msb1;
    step = -step;
  }

  c = count;
  diff = (i^c) & mask;
  cj1 = cj[1];
  while (diff) {
    if (diff&1) nied[1] ^= *cj1;
    diff >>= 1;
    cj1++;
  }

  do {
    diff = (i^c) >> nmsb;
    cj1 = cj[1] + nmsb;
    while (diff) {
      if (diff&1) nied[1] ^= *cj1;
      diff >>= 1;
      cj1++;
    }
    c = i;
    i += step;
    if (i >= NBITS_POW) {
      fprintf(stderr, "\nOverflow in Niederreiter sequence. A %d-bit sequence is not enough???\n", NBITS);
      
      abort();
    }
  } while ((nied[1] & rmask) != rmsb2);

  cj0 = cj[0]; cj2 = cj[2]; cj3 = cj[3];
  diff = c ^ count;
  while (diff) {
    if (diff&1) {
      nied[0] ^= *cj0;
      nied[2] ^= *cj2;
      nied[3] ^= *cj3;
    }
    diff >>= 1;
    cj0++; cj2++; cj3++;
  }
  count = c;

  *idx = count - SKIP;
  return nied;
}


niedindex RadicalInverse(niedindex n)
{
  niedindex inv = 0, f = NBITS_POW1;
  while (n) {
    if (n&1) inv |= f;
    f>>=1;
    n>>=1;
    if (n&1) inv |= f;
    f>>=1;
    n>>=1;
    if (n&1) inv |= f;
    f>>=1;
    n>>=1;
    if (n&1) inv |= f;
    f>>=1;
    n>>=1;
  }
  return inv;
}


void FoldSample(niedindex *xi1, niedindex *xi2)
{
  niedindex u=*xi1, v=*xi2, d, m;

  u = (u & ~3)|1;		
  v = (v & ~3)|1;		

  d = (u & v) & ~1;		

  m = NBITS_POW;		
  while (d) {
    if (d&NBITS_POW1) {		
      u = (u & m) | (~(u-1) & ~m);
      v = (v & m) | (~(v-1) & ~m);
    }
    m |= m>>1;
    d <<= 1;
  }

  *xi1 = u; *xi2 = v;
}

#ifdef TEST2

#include "time.h"

int main(int argc, char **argv)
{
  int i, j, n, count, c1, c2, m1, m2;
  double xi1, xi2, xi3, xi4, min1, max1, min2, max2;
  clock_t t;

  fprintf(stderr, "x interval code? y interval code?\n");
  scanf("%u %u", &c1, &c2);
  for (n=0, m1=1, i=c1; i; i>>=1, m1<<=1, n++); m1>>=1; c1 ^= m1; m1--; n--;
  
  for (m2=1, i=c2; i; i>>=1, m2<<=1); m2>>=1; c2 ^= m2; m2--;
  c2 = RadicalInverse(c2);
  
  fprintf(stderr, "c1=%08x, m1=%08x, c2=%08x, m2=%08x, n=%d\n", 
	  c1, m1, c2, m2, n);

  fprintf(stderr, "count?\n");
  scanf("%u", &count);

  t = clock();

  for (i=0, j=0; i<count; i++, j++) {
    
    NextNiedInRange(&j, +1, 1, n, c1, c2, &xi1, &xi2, &xi3, &xi4);
    
    
  }

  t = clock() - t;
  fprintf(stderr, "%g secs.\n", (float)t/(float)CLOCKS_PER_SEC);

#ifdef NEVER
  fprintf(stderr, "count?\n"); scanf("%d", &count);

  t = clock();
  for (i=0; i<count; i++) {
    double xi5, xi6, xi7, xi8;
    IncrementalNied(i, &xi5, &xi6, &xi7, &xi8);
    Nied(i, &xi1, &xi2, &xi3, &xi4);
    
    if (xi1 != xi5 || xi2 != xi6 || xi3 != xi7 || xi4 != xi8)
      fprintf(stderr, "%d differs.\n", i);
  }
  t = clock() - t;
  fprintf(stderr, "%g secs.\n", (float)t/(float)CLOCKS_PER_SEC);
#endif

#ifdef NEVER
  fprintf(stderr, "count?\n"); scanf("%d", &count);
  fprintf(stderr, "min1, max1, min2, max2?\n");
  scanf("%lf %lf %lf %lf", &min1, &max1, &min2, &max2);

  Nied(-1, &xi1, &xi2, &xi3, &xi4);
  t = clock(); n=-1;
  for (i=0; i<count; i++) {
    niedindex k = n+1;
    NextNiedInRange(min1, max1, min2, max2, &k, &xi1, &xi2, &xi3, &xi4);
    printf("%d %g %g %g %g\n", k, xi1, xi2, xi3, xi4);

    do {
      n++;
      Nied(n, &xi1, &xi2, &xi3, &xi4);
    } while (xi1<min1 || xi1>=max1 || xi2<min2 || xi2>=max2);
    printf("%d %g %g %g %g\n", n, xi1, xi2, xi3, xi4);
    
  }
  t = clock() - t;
  fprintf(stderr, "%g secs.\n", (float)t/(float)CLOCKS_PER_SEC);
#endif

  return 0;
}
#endif 

#ifdef TEST3


int main(int argc, char **argv)
{
#define N 16
  int i, j, k, m[N][N];

  for (i=0; i<N; i++)
    for (j=0; j<N; j++)
      m[i][j] = 0;

  for (k=0; k<N*N; k++) {
    niedindex *xi = Nied(k-SKIP);
    i = xi[0] >> (NBITS-4);
    j = xi[1] >> (NBITS-4);
    if (m[i][j] != 0)
      printf("Sorry ... positie (%d,%d) al bezet met nr. %d\n", 
	     i, j, m[i][j]);
    else
      m[i][j] = k;
  }

  for (i=0; i<N; i++) {
    printf("\t{");
    for (j=0; j<N; j++)
      printf("%3d,", m[i][j]);
    printf("}\n");
  }

  return 0;
}

#endif 

#ifdef TEST
int main(int argc, char **argv)
{
  int i, n;

  fprintf(stderr, "Aantal getallen?\n");
  scanf("%d", &n);

  for (i=0; i<n; i++) {
    niedindex *xi = Nied(i);
    niedindex xi1=xi[0], xi2=xi[1];
    printf("%g %g   ", (double)xi1*RECIP, (double)xi2*RECIP);
    FoldSample(&xi1, &xi2);
    printf("\t%g %g  \t%g\n", (double)xi1*RECIP, (double)xi2*RECIP,
	   (double)(xi1+xi2)*RECIP);
  }

  return 0;
}

#endif 
