/* halton.h: Halton quasi Monte Carlo sample generator */

#ifndef _HALTON_H_
#define _HALTON_H_

/* returns the i-th Halton number with given base */
extern double Halton(int i, int base);
extern double Halton2(int i), Halton3(int i), Halton5(int i), Halton7(int i);

#endif /*_HALTON_H_*/
