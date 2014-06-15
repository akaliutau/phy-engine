/* sobol.h: Sobol QMC sequence */

#ifndef _SOBOL_H_
#define _SOBOL_H_

extern double *NextSobol(void);
extern double *Sobol(int seed);
extern void InitSobol(int idim);

#endif /*_SOBOL_H_*/

