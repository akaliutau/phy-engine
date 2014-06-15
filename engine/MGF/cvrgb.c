

#ifndef lint
static char SCCSid[] = "@(#)cvrgb.c 1.1 12/1/95 LBL";
#endif



#include <stdio.h>

#include "parser.h"

			
#define  CIE_x_r		0.640		
#define  CIE_y_r		0.330
#define  CIE_x_g		0.290
#define  CIE_y_g		0.600
#define  CIE_x_b		0.150
#define  CIE_y_b		0.060
#define  CIE_x_w		0.3333		
#define  CIE_y_w		0.3333

#define CIE_C_rD	( (1./CIE_y_w) * \
				( CIE_x_w*(CIE_y_g - CIE_y_b) - \
				  CIE_y_w*(CIE_x_g - CIE_x_b) + \
				  CIE_x_g*CIE_y_b - CIE_x_b*CIE_y_g	) )
#define CIE_C_gD	( (1./CIE_y_w) * \
				( CIE_x_w*(CIE_y_b - CIE_y_r) - \
				  CIE_y_w*(CIE_x_b - CIE_x_r) - \
				  CIE_x_r*CIE_y_b + CIE_x_b*CIE_y_r	) )
#define CIE_C_bD	( (1./CIE_y_w) * \
				( CIE_x_w*(CIE_y_r - CIE_y_g) - \
				  CIE_y_w*(CIE_x_r - CIE_x_g) + \
				  CIE_x_r*CIE_y_g - CIE_x_g*CIE_y_r	) )


float	xyz2rgbmat[3][3] = {		
	{(CIE_y_g - CIE_y_b - CIE_x_b*CIE_y_g + CIE_y_b*CIE_x_g)/CIE_C_rD,
	 (CIE_x_b - CIE_x_g - CIE_x_b*CIE_y_g + CIE_x_g*CIE_y_b)/CIE_C_rD,
	 (CIE_x_g*CIE_y_b - CIE_x_b*CIE_y_g)/CIE_C_rD},
	{(CIE_y_b - CIE_y_r - CIE_y_b*CIE_x_r + CIE_y_r*CIE_x_b)/CIE_C_gD,
	 (CIE_x_r - CIE_x_b - CIE_x_r*CIE_y_b + CIE_x_b*CIE_y_r)/CIE_C_gD,
	 (CIE_x_b*CIE_y_r - CIE_x_r*CIE_y_b)/CIE_C_gD},
	{(CIE_y_r - CIE_y_g - CIE_y_r*CIE_x_g + CIE_y_g*CIE_x_r)/CIE_C_bD,
	 (CIE_x_g - CIE_x_r - CIE_x_g*CIE_y_r + CIE_x_r*CIE_y_g)/CIE_C_bD,
	 (CIE_x_r*CIE_y_g - CIE_x_g*CIE_y_r)/CIE_C_bD}
};


mgf2rgb(register C_COLOR *cin, double intensity, register float *cout)		
                	     	
      	          		
              	        	
{
	static double	cie[3];
					
	c_ccvt(cin, C_CSXY);
	cie[0] = intensity*cin->cx/cin->cy;
	cie[1] = intensity;
	cie[2] = intensity*(1./cin->cy - 1.) - cie[0];
					
	cout[0] = xyz2rgbmat[0][0]*cie[0] + xyz2rgbmat[0][1]*cie[1]
			+ xyz2rgbmat[0][2]*cie[2];
	if(cout[0] < 0.) cout[0] = 0.;
	cout[1] = xyz2rgbmat[1][0]*cie[0] + xyz2rgbmat[1][1]*cie[1]
			+ xyz2rgbmat[1][2]*cie[2];
	if(cout[1] < 0.) cout[1] = 0.;
	cout[2] = xyz2rgbmat[2][0]*cie[0] + xyz2rgbmat[2][1]*cie[1]
			+ xyz2rgbmat[2][2]*cie[2];
	if(cout[2] < 0.) cout[2] = 0.;
}
