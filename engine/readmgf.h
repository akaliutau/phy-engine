/* readmgf.h */

#ifndef _READMGF_H_
#define _READMGF_H_

#ifdef __cplusplus
extern "C" {
#endif

/* sets the number of quarter circle divisions for discretizing cylinders,
 * sphere, cones ... */
extern void MgfSetNrQuartCircDivs(int divs);

/* sets a flag indicating whether all surfaces should be considered
 * 1-sided (the best for "good" models) or not. */
extern void MgfSetIgnoreSidedness(int yesno);

/* if yesno is TRUE, all materials will be converted to be monochrome. */
extern void MgfSetMonochrome(int yesno);

/* Reads in an MGF file. The result should be that the globals variables 
 * World and MaterialLib in scene.h are filled in. */
extern void ReadMgf(char *filename);


extern void MgfDefaults(void);
extern void ParseMgfOptions(int *argc, char **argv);
extern void PrintMgfOptions(FILE *fp);

#ifdef __cplusplus
}
#endif

#endif /*_READMGF_H_*/
