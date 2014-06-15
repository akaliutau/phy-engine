

#include <stdio.h>
#include <math.h>

#include "faure.h"

#define MAXDIM 10
#define PRDIM 30
#define MAXSEED 2147483647

static int ix[MAXDIM][PRDIM]; 
static int dim, PR, prime[MAXDIM]={2, 3, 5, 5, 7, 7, 11, 11, 11, 11};

static int nextn, skip, ndigits;
static int C[MAXDIM][PRDIM][PRDIM];  

static int setFaureC(void)
{
  int i, j, k;


  for(j=0; j<ndigits; j++)
    for(k=j; k<ndigits; k++)
    {
      if (j==0) C[0][j][k]=1;
      else
        if (j==k) C[0][j][k]=1;
	        else C[0][j][k] = (C[0][j][k-1]+C[0][j-1][k-1])%PR;
    }


  for(i=dim-1; i>=0; i--)
    for(j=0; j<ndigits; j++)
      for(k=j; k<ndigits; k++)
        C[i][j][k] = (C[0][j][k] * (int)pow(i, k-j)) % PR;

  return 0;
}

static int setGFaureC(void)
{
  int i, j, k;
  unsigned P[PRDIM][PRDIM];

  
  for (j=0; j<ndigits; j++) {
    P[j][0] = 1;
    P[j][j] = 1;
  }
  for (j=1; j<ndigits; j++) {
    for (k=1; k<j; k++)
      P[j][k] = (P[j-1][k-1] + P[j-1][k])%PR;
    for (k=j+1; k<ndigits; k++)
      P[j][k] = 0;
  }

  
  for (i=0; i<dim; i++) {
    
    int m, j;
    for (m=0; m<ndigits; m++) {
      for (j=0; j<ndigits; j++) {
	int q, Q = m<j ? m : j;
	C[i][m][j] = 0;
	for (q=0; q<=Q; q++)
	  C[i][m][j] = (C[i][m][j] + P[m][q] * P[j][q] * (int)pow(i, m+j-2*q))%PR;
      }
    }
  }

  return 0;
}

double *NextFaure(void)
{
  int i, j, k, save;
  static double x[MAXDIM];
  double xx;

  save=nextn;
  k=1;
  while((save%PR)==(PR-1))
  {
    k=k+1;
    save=save/PR;
  }
  for (i=0; i<dim; i++)
  {
    xx=0;
    for(j=ndigits-1; j>=0; j--)
    {
      ix[i][j]=(ix[i][j]+C[i][j][k-1])%PR;
      xx=xx/PR+ix[i][j];
    }
    x[i]=xx/PR;
  }
  nextn+=1;
  return x;
}

double *Faure(int seed)
{
  int i, j, k, save;
  static double x[MAXDIM];
  double xx;

  nextn = seed+skip+1;
#ifndef NO_GRAY
  
  i = nextn;
  j = nextn/PR;
  k = 1;
  nextn = 0;
  while (i) {
    nextn += (((i%PR) + PR - (j%PR))%PR) * k;
    k *= PR;
    i /= PR; j /= PR;
  }
#endif 

  for (i=0; i<dim; i++) {
    xx=0;
    for(j=ndigits-1; j>=0; j--) {
      save=nextn;
      ix[i][j] = 0;
      for (k=0; k<ndigits; k++) {
	ix[i][j]=(ix[i][j] + C[i][j][k] * save)%PR;
	save/=PR;
      }
      xx=xx/PR+ix[i][j];
    }
    x[i]=xx/PR;
  }
  return x;
}

void InitFaure(int idim)
{
  int i, j;

  dim=idim;
  nextn=0;
  PR=prime[dim-1];
  ndigits = log((double)MAXSEED) / log((double)PR) + 1;
  setFaureC();
  for(i=0; i<dim; i++)
    for(j=0; j<ndigits; j++) ix[i][j]=0;

  skip=pow(PR, 4)-1;
  for (i=1; i<=skip; i++) NextFaure(); 
}

void InitGFaure(int idim)
{
  int i, j;

  dim=idim;
  nextn=0;
  PR=prime[dim-1];
  ndigits = log((double)MAXSEED) / log((double)PR) + 1;
  setGFaureC();
  for(i=0; i<dim; i++)
    for(j=0; j<ndigits; j++) ix[i][j]=0;

  skip=pow(PR, 4)-1;
  for (i=1; i<=skip; i++) NextFaure(); 
}

#ifdef TEST
int main(int rgc, char **argv)
{
  int i;
  InitGFaure(4);
  for (i=0; i<10; i++) {
    double *x1, *x2;
    x1 = NextFaure();
    x2 = Faure(i);
    printf("%f %f   %f %f   %f %f   %f %f\n",
	   x1[0], x2[0],   x1[1], x2[1],   x1[2], x2[2],   x1[3], x2[3]);
  }
  return 0;
}
#endif 
