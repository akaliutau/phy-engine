

#ifndef lint
static const char SCCSid[] = "@(#)fvect.c 1.3 3/18/97 LBL";
#endif



#include <stdio.h>
#include <math.h>
#include "parser.h"


double
normalize(register double *v)			
                  
{
	static double  len;
	
	len = DOT(v, v);
	
	if (len <= 0.0)
		return(0.0);
	
	if (len <= 1.0+FTINY && len >= 1.0-FTINY)
		len = 0.5 + 0.5*len;	
	else
		len = sqrt(len);

	v[0] /= len;
	v[1] /= len;
	v[2] /= len;

	return(len);
}


void
fcross(register double *vres, register double *v1, register double *v2)		
                             
{
	vres[0] = v1[1]*v2[2] - v1[2]*v2[1];
	vres[1] = v1[2]*v2[0] - v1[0]*v2[2];
	vres[2] = v1[0]*v2[1] - v1[1]*v2[0];
}
