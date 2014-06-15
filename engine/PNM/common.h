/* common.h */

#ifndef _COMMON_H_INCLUDED_
#define _COMMON_H_INCLUDED_

#include <stdio.h>
#include "pnm.h"

#ifndef TRUE
#define TRUE 1
#endif /*TRUE*/

#ifndef FALSE
#define FALSE 0
#endif /*FALSE*/

#ifdef DDEBUG
static int myfgetc(FILE *fp)
{
  int c;
  
  c = fgetc(fp);
  fputc(c, stderr);
  
  return c;
}

static int myungetc(int c, FILE *fp)
{
  fprintf(stderr, "\nungetc(%c)\n", c);
  return ungetc(c, fp);
}
#else
#define myfgetc(fp)	fgetc(fp)
#define myungetc(c, fp)	ungetc(c, fp)
#endif

/* reads one symbol from fp. If this symbol is a space, the function
 * returns 0, if it is not, it returns non-zero: used to skip one space
 * before reading raw ppm, pgm, pbm data */
extern int skip_one_space(FILE *fp);

/* skips characters up to and including the first newline */
extern void skip_until_eol(FILE *fp);

/* skips spaces and comments - does nothing if the first read character is 
 * not a space or a '#'. */
extern void skipspace(FILE *fp);

/* skips comments: if the first symbol read is a '#' all following symbols
 * up to and including the next newline character are skipped. If not,
 * nothing happens. */
extern void skip_comment(FILE *fp);

/* reads a series of successive digits until a non-digit is encountered,
 * converts the digits to number, returns that (non negative) number. 
 * returns negative if the first read symbol is not a digit. */
extern unsigned int getnatural(FILE *fp);

/*
 * returns 1 if reading a '0' (white) 
 *         0              '1' (black)
 *         negative     something else
 */
extern CVAL getbit(FILE *fp);

/* allocates space to hold the graphics data: nr. of channels as specified in
 * the IMAGE struct, height and width are rounded up to an integer multiple 
 * of PAD_SIZE defined in pnm.h - returns zero if succesful,
 * nonzero otherwise. */
extern int AllocPixmap(IMAGE *image);

#endif /* _COMMON_H_INCLUDED_ */












