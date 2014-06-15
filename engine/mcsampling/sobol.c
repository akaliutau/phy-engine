

#include <stdio.h>
#include <math.h> 
#include "sobol.h"

#define MAXDIM 5 
#define VMAX 30

static int dim, nextn, x[MAXDIM], v[MAXDIM][VMAX], skip;
static double RECIPD;

double *NextSobol(void)
{
  static double xx[MAXDIM];
  int c, i, save;
  
  c=1;  save=nextn;
  while((save%2)==1){c+=1; save=save/2;};
  for(i=0; i<dim; i++)
  {
    x[i] = x[i] ^ (v[i][c-1]<<(VMAX-c));
    xx[i]=x[i]*RECIPD;
  }
  nextn+=1;

  return xx;
}


#define GRAY(n)	(n ^ (n>>1))

double *Sobol(int seed)
{
  static double xx[MAXDIM];
  
  int c, i, gray;

  seed += skip + 1;
  for(i=0; i<dim; i++)
  {
    x[i] = 0;
    c=1;
    gray = GRAY(seed);
    while (gray) {
      if (gray&1) {
	x[i] = x[i] ^ (v[i][c-1]<<(VMAX-c));
      }
      c++;
      gray >>= 1;
    }

    xx[i]=x[i]*RECIPD;
  }

  return xx;
}

void InitSobol(int idim)
{
  int i, j, k, m, save;
  int d[MAXDIM], POLY[MAXDIM];
  
  
  nextn=0;
  dim=idim;
  RECIPD=1./pow(2.0, VMAX);
  
    
  POLY[0]=3;  d[0]=1;  
  POLY[1]=7;  d[1]=2;  
  POLY[2]=11; d[2]=3;  
  POLY[3]=19; d[3]=4;  
  POLY[4]=37; d[4]=5;  

  

  
  
  for(i=0; i<dim; i++)
    for(j=0; j<d[i]; j++) v[i][j]=1;

  

  
  for(i=0; i<dim; i++)
    for(j=d[i]; j<VMAX; j++)
    {
      v[i][j]=v[i][j-d[i]];
      save=POLY[i];
      m=pow(2, d[i]);
      for(k=d[i]; k>0; k--)
      {
        v[i][j] = v[i][j] ^ m*(save%2)*v[i][j-k];
        save=save/2; m=m/2;
      }
    }

  for(i=0; i<dim; i++) x[i]=0;
  skip=pow(2, 6); 
  for(i=1; i<=skip; i++) NextSobol();
    
}

#ifdef TEST
int main(int argc, char **argv)
{
  int i;

  InitSobol(4);
  for (i=0; i<10; i++) {
    double *x1, *x2;
    x1 = NextSobol();
    x2 = Sobol(i, 4);
    printf("%f %f   %f %f   %f %f   %f %f\n",
	   x1[0], x2[0], x1[1], x2[1], x1[2], x2[2], x1[3], x2[3]);
  }

  return 0;
}
#endif 
