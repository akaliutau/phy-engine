/******************************************************************/
/*
 * sampling.h : some useful routines for sampling things
 */
/******************************************************************/

#ifndef _SAMPLING_H_
#define _SAMPLING_H_

#ifdef __cplusplus
extern "C" {
#endif

  /* Sample from a discrete pdf. total is the sum of the 
     (non scaled) probabilities, x_1 is re-adjusted to [0,1] after
     sampling, pdf for choosing index is filled in */
int SampleDiscrete(float probabilities[], float total, double *x_1, double *pdf);
int DSampleDiscrete(double probabilities[], double total, double *x_1, double *pdf);

#ifdef __cplusplus
}
#endif

#endif
