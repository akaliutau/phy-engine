

#include "pnm.h"
#include "pnm_private.h"
#include "common.h"
#include "pools.h"


static int ReadPBMHeader(FILE *fp, IMAGE *pnm)
{
#ifdef DEBUG
  fprintf(stderr, "ReadPBMHeader:\n");
#endif 

  skipspace(fp);

  if ((pnm->width = getnatural(fp)) < 0)
    return 1;

  skipspace(fp);
  
  if ((pnm->height = getnatural(fp)) < 0)
    return 2;
  
  skip_comment(fp);
  if (skip_one_space(fp) != 0)
    return 3;
  
  pnm->nrchannels = 1;
  pnm->maxval = 1;
  
  return 0;
}


static int ReadPGMHeader(FILE *fp, IMAGE *pnm)
{
#ifdef DEBUG
  fprintf(stderr, "ReadPGMHeader:\n");
#endif 

  skipspace(fp);
  
  if ((pnm->width = getnatural(fp)) < 0)
    return 1;
  
  skipspace(fp);
  
  if ((pnm->height = getnatural(fp)) < 0)
    return 2;
  
  skipspace(fp);
  
  if ((pnm->maxval = getnatural(fp)) < 0)
    return 3;
  if (pnm->maxval > MAXCVAL) {
    PnmError(NULL, "Can't process this pgm file: max. grayvalue too large");
    return 4;
  }
  
  skip_comment(fp);
  if (skip_one_space(fp) != 0)
    return 5;
  
  pnm->nrchannels = 1;
  
  return 0;
}


static int ReadPPMHeader(FILE *fp, IMAGE *pnm)
{
#ifdef DEBUG
  fprintf(stderr, "ReadPPMHeader:\n");
#endif 

  skipspace(fp);
  
  if ((pnm->width = getnatural(fp)) < 0)
    return 1;
  
  skipspace(fp);
  
  if ((pnm->height = getnatural(fp)) < 0)
    return 2;
  
  skipspace(fp);
  
  if ((pnm->maxval = getnatural(fp)) < 0)
    return 3;
  if (pnm->maxval > MAXCVAL) {
    PnmError(NULL, "Can't process this ppm file: max. colorvalue too large");
    return 4;
  }
  
  skip_comment(fp);
  if (skip_one_space(fp) != 0)
    return 5;
  
  pnm->nrchannels = 3;
  
  return 0;
}


static IMAGE *ReadPBM(FILE *fp, IMAGE *pnm)
{
  int row, col, bit;
  
  if (ReadPBMHeader(fp, pnm) != 0 ||
      AllocPixmap(pnm) != 0) 
    return (IMAGE *)NULL;
  
  for (row = 0; row < pnm->height; row++)
    for (col = 0; col < pnm->width; col++) {
      skipspace(fp);
      if ((bit = getbit(fp)) < 0)
	return (IMAGE *)NULL;
      pnm->pix[0][row][col] = bit;
    }
  
  return pnm;
}


static IMAGE *ReadPGM(FILE *fp, IMAGE *pnm)
{
  int row, col, gray;
  
  if (ReadPGMHeader(fp, pnm) != 0 ||
      AllocPixmap(pnm) != 0) 
    return (IMAGE *)NULL;
  
  for (row = 0; row < pnm->height; row++)
    for (col = 0; col < pnm->width; col++) {
      skipspace(fp);
      if ((gray = getnatural(fp)) < 0)
	return (IMAGE *)NULL;
      pnm->pix[0][row][col] = gray;
    }
  
  return pnm;
}


static IMAGE *ReadPPM(FILE *fp, IMAGE *pnm)
{
  int row, col, red, green, blue;
  
  if (ReadPPMHeader(fp, pnm) != 0 ||
      AllocPixmap(pnm) != 0) 
    return (IMAGE *)NULL;
  
  for (row = 0; row < pnm->height; row++)
    for (col = 0; col < pnm->width; col++) {
      skipspace(fp);
      if ((red = getnatural(fp)) < 0)
	return (IMAGE *)NULL;
      pnm->pix[0][row][col] = red;
      
      skipspace(fp);
      if ((green = getnatural(fp)) < 0)
	return (IMAGE *)NULL;
      pnm->pix[1][row][col] = green;
			
      skipspace(fp);
      if ((blue = getnatural(fp)) < 0)
	return (IMAGE *)NULL;
      pnm->pix[2][row][col] = blue;
    }

  return pnm;
}


static IMAGE *ReadRawPBM(FILE *fp, IMAGE *pnm)
{
  int row, col, count, c=0;
  
  if (ReadPBMHeader(fp, pnm) != 0 ||
      AllocPixmap(pnm) != 0) 
    return (IMAGE *)NULL;
  
  count = 0;
  for (row = 0; row < pnm->height; row++)
    for (col = 0; col < pnm->width; col++) {
      if ((count & 7) == 0) { 
	if ((c = myfgetc(fp)) == EOF) {
	  PnmError(NULL, "Unexpected end of input");
	  return (IMAGE *)NULL;
	}
	count = 0;
      }
      
      pnm->pix[0][row][col] = (c & 0x80) ? 0 : 1;
      c <<= 1; count++;
    }
  
  return pnm;
}

static IMAGE *ReadRawPGM(FILE *fp, IMAGE *pnm)
{
  int row, col, c;
  
  if (ReadPGMHeader(fp, pnm) != 0 ||
      AllocPixmap(pnm) != 0) 
    return (IMAGE *)NULL;
  
  for (row = 0; row < pnm->height; row++)
    for (col = 0; col < pnm->width; col++) {
      if ((c = myfgetc(fp)) == EOF) {
	PnmError(NULL, "Unexpected end of input");
	return (IMAGE *)NULL;
      }
      pnm->pix[0][row][col] = c;
    }
  
  return pnm;
}

static IMAGE *ReadRawPPM(FILE *fp, IMAGE *pnm)
{
  int row, col=0, red=0, green=0, blue=0;
  
  if (ReadPPMHeader(fp, pnm) != 0 ||
      AllocPixmap(pnm) != 0) 
    return (IMAGE *)NULL;
  
  for (row = 0; row < pnm->height; row++)
    for (col = 0; col < pnm->width; col++) {
      if ((red = myfgetc(fp)) == EOF ||
	  (green = myfgetc(fp)) == EOF ||
	  (blue = myfgetc(fp)) == EOF) {
	PnmError(NULL, "Unexpected end of input");
	return (IMAGE *)NULL;
      }
      pnm->pix[0][row][col] = red;
      pnm->pix[1][row][col] = green;
      pnm->pix[2][row][col] = blue;
    }
  
  return pnm;
}


IMAGE *PnmRead(FILE *fp)
{
  IMAGE *pnm = (IMAGE *)NULL; 
  int c;
  
  pnm = (IMAGE *)Alloc(sizeof(IMAGE));
  
  
  c = myfgetc(fp);
  if (c != 'P') {
    PnmError(NULL, "Input not a pnm file");
    return (IMAGE *)NULL;
  }
  
  switch ((c = myfgetc(fp))) {
  case '1':
    return ReadPBM(fp, pnm);
  case '2':
    return ReadPGM(fp, pnm);
  case '3':
    return ReadPPM(fp, pnm);
  case '4':
    return ReadRawPBM(fp, pnm);
  case '5':
    return ReadRawPGM(fp, pnm);
  case '6':
    return ReadRawPPM(fp, pnm);
  default:
    PnmError(NULL, "Input not a pnm file");
  }
  
  return (IMAGE *)NULL;
}
