

#include <ctype.h>

#include "pnm_private.h"
#include "common.h"
#include "pools.h"


int skip_one_space(FILE *fp)
{
#ifdef DDEBUG
  fprintf(stderr, "\nskip_one_space:\n");
#endif 
  if (isspace(myfgetc(fp)))
    return 0;

  PnmError(
#ifdef DEBUG
	"skip_one_space",
#else
	NULL,
#endif
	"Input is not valid");
  return 1;
}


void skip_until_eol(FILE *fp)
{
  int c;
#ifdef DDEBUG
  fprintf(stderr, "\nskip_until_eol:\n");
#endif 

  while ((c = myfgetc(fp)) != '\n') {}
}


void skipspace(FILE *fp)
{
  int c;
#ifdef DDEBUG
  fprintf(stderr, "\nskipspace:\n");
#endif 

  while (isspace(c = myfgetc(fp))) {}
  if (c == '#') {	
    skip_until_eol(fp);
    skipspace(fp);
  }
  else	
    myungetc(c, fp);
}


void skip_comment(FILE *fp)
{
  int c;
#ifdef DDEBUG
  fprintf(stderr, "\nskip_comment:\n");
#endif 

  if ((c = myfgetc(fp)) == '#') 
    skip_until_eol(fp);
  else
    myungetc(c,fp);
}


unsigned int getnatural(FILE *fp)
{
  int c;
  unsigned int num = 0;
#ifdef DDEBUG
  fprintf(stderr, "\ngetnatural:\n");
#endif 
	
  if (!isdigit(c = myfgetc(fp))) {
    PnmError(
#ifdef DEBUG
	  "getnatural",
#else
	  NULL,
#endif
	  "Input is not valid");
    return -1;
  }
  while (isdigit(c)) {
    num *= 10;
    num += (c - '0');
    c = myfgetc(fp);
  }
  myungetc(c, fp);
  
  return num;
}


CVAL getbit(FILE *fp)
{
#ifdef DDEBUG
  fprintf(stderr, "\ngetbit:\n");
#endif 

  switch (myfgetc(fp)) {
  case '0': return 1;
  case '1': return 0;
  default:
    PnmError( 
#ifdef DEBUG
	  "getbit",
#else
	  NULL,
#endif
	  "Input is not valid");
  }

  return -1;
}


#define up_mult(a, b) (b*((a+b-1)/b))


int AllocPixmap(IMAGE *pnm)
{
  int chan, row, col, pad_height, pad_width;
#ifdef DEBUG
  fprintf(stderr, "AllocPixmap: pnm->nrchannels = %ld, pnm->height = %ld, pnm->width = %ld\n", pnm->nrchannels, pnm->height, pnm->width);
#endif 
  pad_height = up_mult(pnm->height, PAD_SIZE);
  pad_width = up_mult(pnm->width, PAD_SIZE);

  pnm->pix = (CVAL ***)Alloc(pnm->nrchannels * sizeof(CVAL **));
  for (chan=0; chan < pnm->nrchannels; chan++) {
    pnm->pix[chan] = (CVAL **)Alloc(pad_height * sizeof(CVAL *));
    for (row=0; row < pad_height; row++) {
      pnm->pix[chan][row] = (CVAL *)Alloc(pad_width * sizeof(CVAL));


      for (col = 0; col < pad_width; col++)
	pnm->pix[chan][row][col] = 0;
    }
  }

  return 0;
}


void PnmFree(IMAGE *pnm)
{
  int chan, row, col, pad_height, pad_width;

  pad_height = up_mult(pnm->height, PAD_SIZE);
  pad_width = up_mult(pnm->width, PAD_SIZE);

  for (chan=0; chan < pnm->nrchannels; chan++) {
    for (row=0; row < pad_height; row++) {
      Free((char *)(pnm->pix[chan][row]), pad_width * sizeof(CVAL));
    }
    Free((char *)(pnm->pix[chan]), pad_height * sizeof(CVAL *));
  }
  Free((char *)(pnm->pix), pnm->nrchannels * sizeof(CVAL **));

  Free((char *)pnm, sizeof(IMAGE));
}
