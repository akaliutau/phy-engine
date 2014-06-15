

#include "pnm.h"
#include "pnm_private.h"
#include "pools.h"

#ifndef RAW_PNM

static int WritePBM(IMAGE *pnm, FILE *fp)
{
  int row, col, count;

  fprintf(fp, "P1\n");
  fprintf(fp, "%ld %ld\n", pnm->width, pnm->height);
  
  count = 0;
  for (row = 0; row < pnm->height; row++)
    for (col = 0; col < pnm->width; col++) {
      if (pnm->pix[0][row][col] == 0)
	fputc('1', fp);	
      else
	fputc('0', fp); 
      fputc(' ', fp);
      count++;
      
      if ((count % 32) == 0)	
	fputc('\n', fp);
    }
  
  return 0;
}	
#endif 


static int WritePGM(IMAGE *pnm, FILE *fp)
{
  int row, col, n, count;
  
  fprintf(fp, "P2\n");
  fprintf(fp, "%ld %ld\n", pnm->width, pnm->height);
  fprintf(fp, "%ld\n", pnm->maxval);
  
  count = 0;
  for (row = 0; row < pnm->height; row++)
    for (col = 0; col < pnm->width; col++) {
      fprintf(fp, "%3d %n", pnm->pix[0][row][col], &n);
      count += n;
      if (count > 64) {
	fputc('\n', fp);
	count = 0;
      }
    }
  
  return 0;
}


static int WritePPM(IMAGE *pnm, FILE *fp)
{
  int row, col, n, count;
  
  fprintf(fp, "P3\n");
  fprintf(fp, "%ld %ld\n", pnm->width, pnm->height);
  fprintf(fp, "%ld\n", pnm->maxval);
  
  count = 0;
  for (row = 0; row < pnm->height; row++)
    for (col = 0; col < pnm->width; col++) {
      fprintf(fp, "%3d %3d %3d  %n", 
	      pnm->pix[0][row][col], 
	      pnm->pix[1][row][col], 
	      pnm->pix[2][row][col], 
	      &n); count += n;
	      if (count > 50) {
		fputc('\n', fp);
		count = 0;
	      }
    }
  
  return 0;
}


static int WriteRawPBM(IMAGE *pnm, FILE *fp)
{
  int row, col, n, c;
  
  fprintf(fp, "P4\n");
  fprintf(fp, "%ld %ld\n", pnm->width, pnm->height);
  
  n = 0; c = 0;
  for (row = 0; row < pnm->height; row++)
    for (col = 0; col < pnm->width; col++) {
      c <<= 1; n++;
      c |= !(pnm->pix[0][row][col] & 1);
      
      if ((n % 8) == 0) {
	fputc(c, fp);	
	c = 0;
      }
    }
  
  return 0;
}


static int WriteRawPGM(IMAGE *pnm, FILE *fp)
{
  int row, col;
  
  fprintf(fp, "P5\n");
  fprintf(fp, "%ld %ld\n", pnm->width, pnm->height);
  fprintf(fp, "%ld\n", pnm->maxval);
  
  for (row = 0; row < pnm->height; row++)
    for (col = 0; col < pnm->width; col++) 
      fputc(pnm->pix[0][row][col], fp);
  
  return 0;
}


static int WriteRawPPM(IMAGE *pnm, FILE *fp)
{
  int row, col;
  
  fprintf(fp, "P6\n");
  fprintf(fp, "%ld %ld\n", pnm->width, pnm->height);
  fprintf(fp, "%ld\n", pnm->maxval);
  
  for (row = 0; row < pnm->height; row++)
    for (col = 0; col < pnm->width; col++) {
      fputc(pnm->pix[0][row][col], fp);
      fputc(pnm->pix[1][row][col], fp);
      fputc(pnm->pix[2][row][col], fp);
    }
  
  return 0;
}

 
int PnmWrite(IMAGE *pnm, FILE *fp)
{
  switch (pnm->nrchannels) {
  case 1:
    if (pnm->maxval == 1)
#ifndef RAW_PNM
      return WritePBM(pnm, fp);
#else 
    return WriteRawPBM(pnm, fp);
    else if (pnm->maxval <= 255)
      return WriteRawPGM(pnm, fp);
#endif 
    else 
      return WritePGM(pnm, fp);
  case 3:
#ifdef RAW_PNM
    if (pnm->maxval <= 255)
      return WriteRawPPM(pnm, fp);
    else
#endif 
      return WritePPM(pnm, fp);
  default:
    PnmError(NULL, "Can't write pnm file with number of channels not equal to 1 or 3");
    return 1;
  }

  return 0;
}




