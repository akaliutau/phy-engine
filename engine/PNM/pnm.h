/* pnm.h: Portable aNy Map */

#ifndef _PNM_H_INCLUDED_
#define _PNM_H_INCLUDED_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL (char *)0
#endif /*NULL*/

/* ------------------------------------------------------------------ */
/* routines for communication of error
 * messages to the user of the library. The routines to be specified
 * have one char * parameter: the message being passed. */
typedef void (*PNM_MSG_CALLBACK_FUNC)(char *);

/* max. length of a message communicated by the pnm library. */
#define PNM_MAX_MESSAGE_LENGTH 200

/* default error callback function prints the message on stderr. */
extern PNM_MSG_CALLBACK_FUNC PnmSetErrorCallback(PNM_MSG_CALLBACK_FUNC func);

/* ------------------------------------------------------------------ */
/* used to store color values */
typedef unsigned char CVAL;	      
#define MAXCVAL 255		      /* max. pos value a CVAL can hold */

/* define RAW_PNM if you want raw pnm files to be written whenever possible.
 * Raw PNM files can be written and read much faster than their ascii
 * counterparts. */
#define RAW_PNM	1

/* image will be padded with black pixels to make width and height a
 * multiple of PAD_SIZE */
#define PAD_SIZE 1

/* ------------------------------------------------------------------ */
#include <stdio.h>

typedef struct IMAGE {
  long int	width, 		/* nr pixels in a row */
                height, 	/* nr. of rows */
                nrchannels,	/* nr of color channels */
                maxval;		/* max. color component value */
  CVAL		***pix;		/* indexed with channel, row and col */
} IMAGE;

/*
 * Read a pnm (i.e. ascii or raw ppm, pgm or ppm) file:
 *
 * Input: pointer to a Portable aNy Map file.
 *
 * Output: pointer to a filled in IMAGE struct if file was succesfully read
 *         NULL pointer if errors occur while reading the file.
 */
extern IMAGE *PnmRead(FILE *fp);

/*
 * Write a pnm-file:
 *
 * If the number of channels is 1 then 
 *	If the max. gray value is 1 then 
 *		a raw pbm file is written
 *	else if  the max. gray value <= 255 then
 *		a raw pgm file is written
 *	else
 *		an ascii pgm file is written
 * else if the number of channels is 3 then
 *	if the max. color value <= 255 then
 *		a raw ppm file is written
 *	else
 *		an ascii ppm file is written
 * else
 *	an error message is written to stderr 
 *
 * Input: pointer to a filled in IMAGE struct.
 *	  pointer to the file to be written.
 *
 * Output: 0 if succesful, nonzero if not.
 *
 * Note: - define RAW_PNM if you want raw pbm, pgm, ppm when possible. 
 *       - color values > 255 are not possible with the current definition
 *	   of CVAL as an unsigned char.
 */ 
extern int PnmWrite(IMAGE *pnm, FILE *fp);

/* deallocates the storage allocated for the image */
extern void PnmFree(IMAGE *image);

#ifdef __cplusplus
}
#endif

#endif /*_PNM_H_INCLUDED_*/ 
