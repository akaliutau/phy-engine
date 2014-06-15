


#include "volume.h"




CUBARULE CRQ1 = {
	"quads degree 1, 1 point",
	1, 1,
        { 0.0	},
        { 0.0	},
	{ 0.0   },
        { 4.0 	}
};


#define w	4./3.
#define u 	0.81649658092772603272 	
#define c	-0.5			
#define s	0.86602540378443864676	
CUBARULE CRQ2 = {
	"quads degree 2, 3 points",
	2, 3,
        { u	, u*c	, u*c	},
        { 0.0	, u*s	,-u*s	},
	{ 0.0	, 0.0	, 0.0 	},
        { w	, w	, w	}
};
#undef s
#undef c
#undef u
#undef w


#define u	0.81649658092772603272 	
CUBARULE CRQ3 = {
	"quads degree 3, 4 points",
	3, 4,
        { u	, 0.	,-u	, 0. 	},
        { 0.	, u	, 0.	,-u 	},
	{ 0.	, 0.	, 0.	, 0. 	},
        { 1.0	, 1.0	, 1.0	, 1.0	}
};
#undef u


#define	u 0.57735026918962576450 	
CUBARULE CRQ3PG = {
	"quads degree 3, 4 points, product Gauss formula",
	3 , 4 ,	
        { u	, u	,-u	,-u 	},  
        { u	,-u	, u	,-u 	},  
	{ 0.	, 0.	, 0.	, 0. 	},  
        { 1.0	, 1.0	, 1.0	, 1.0	}   
};
#undef u


CUBARULE CRQ4 = {
	"quads degree 4, 6 points",
	4 , 6 , 
        { 0.0			, 0.0			, 0.774596669241483	,-0.774596669241483	, 0.774596669241483	,-0.774596669241483	},
        {-0.356822089773090	, 0.934172358962716	, 0.390885162530071	, 0.390885162530071	,-0.852765377881771	,-0.852765377881771	},
	{ 0.0			, 0.0			, 0.0			, 0.0			, 0.0			, 0.0                   },
        { 1.286412084888852	, 0.491365692888926	, 0.761883709085613	, 0.761883709085613	, 0.349227402025498	, 0.349227402025498	}
};


#define w1	8./7.
#define w2	5./9.
#define w3	20./63.
#define r		0.96609178307929588492
#define s		0.57735026918962573106
#define t		0.77459666924148340428
CUBARULE CRQ5 = {
	"quads degree 5, 7 points, Radon's rule",
	5 , 7 , 
        { 0.0	, s	, s	,-s	,-s	, r	,-r	},
        { 0.0	, t	,-t	, t	,-t	, 0.0	, 0.0	},
	{ 0.0	, 0.0	, 0.0	, 0.0	, 0.0	, 0.0	, 0.0   },
        { w1	, w2	, w2	, w2	, w2	, w3	, w3	}
};
#undef w1
#undef w2
#undef w3
#undef r
#undef s
#undef t


#define x0	0.0
#define w0	8./9.
#define x1	0.7745966692414834
#define w1	5./9.
CUBARULE CRQ5PG = {
	"quads degree 5, 9 points product Gauss rule",
	5, 9,
        {-x1	,-x1	,-x1	, x0	, x0	, x0	, x1	, x1	, x1	},
        {-x1	, x0	, x1	,-x1	, x0	, x1	,-x1	, x0	, x1	},
	{ 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0. },
        { w1*w1	, w1*w0	, w1*w1	, w0*w1	, w0*w0	, w0*w1	, w1*w1	, w1*w0	, w1*w1	}
};
#undef w1
#undef x1
#undef w0
#undef x0


CUBARULE CRQ6 = {
	"quads degree 6, 10 points",
	6 , 10 , 
        { 0.0			, 0.0			, 0.863742826346154	,-0.863742826346154	, 
	  0.518690521392592	,-0.518690521392592	, 0.933972544972849	,-0.933972544972849	,
	  0.608977536016356	,-0.608977536016356	},
        { 0.869833375250059	,-0.479406351612111	, 0.802837516207657	, 0.802837516207657	,
	  0.262143665508058	, 0.262143665508058	,-0.363096583148066	,-0.363096583148066	,
	 -0.896608632762453	, -0.896608632762453	},
	{ 0.			, 0.			, 0.			, 0.			, 
	  0.			, 0.			, 0.			, 0.			,
	  0.			, 0.                    },
        { 0.392750590964348	, 0.754762881242610	, 0.206166050588279	, 0.206166050588279	,
	  0.689992138489864 	, 0.689992138489864	, 0.260517488732317	, 0.260517488732317	,
	  0.269567586086061	, 0.269567586086061	}
};


#define r	0.92582009977255141919	
#define s 	0.38055443320831561227	
#define t	0.80597978291859884159	
#define w1	0.24197530864197530631	
#define w2	0.52059291666739448967	
#define w3	0.23743177469063023177	
CUBARULE CRQ7 = {
	"quads degree 7, 12 points",
	7 , 12 , 
        { r	,-r	, 0.0	, 0.0	, s	, s	,-s	,-s	, t	, t	,-t	,-t	},
        { 0.0	, 0.0	, r	,-r	, s	,-s	, s	,-s	, t	,-t	, t	,-t	},
	{ 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0.    },
        { w1	, w1	, w1 	, w1	, w2	, w2	, w2	, w2	, w3	, w3	, w3	, w3	}
};
#undef r
#undef s
#undef t
#undef w1
#undef w2
#undef w3


#define x1	0.86113631159405257522
#define x2	0.33998104358485626480	
#define w1	0.34785484513745385737
#define w2	0.65214515486254614263
CUBARULE CRQ7PG = {
	"quads degree 7, 16 points product Gauss rule",
	7, 16, 
        {-x1	,-x1	,-x1	,-x1	,-x2	,-x2	,-x2	,-x2	, x2	, x2	, x2	, x2	, x1	, x1	, x1	, x1	},
        {-x1	,-x2	, x2	, x1	,-x1	,-x2	, x2	, x1	,-x1	,-x2	, x2	, x1	,-x1	,-x2	, x2	, x1	},
	{ 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0. },
        { w1*w1	, w1*w2	, w1*w2	, w1*w1	, w2*w1	, w2*w2	, w2*w2	, w2*w1	, w2*w1	, w2*w2	, w2*w2	, w2*w1	, w1*w1	, w1*w2	, w1*w2	, w1*w1	}
};
#undef w2
#undef w1
#undef x2
#undef x1
 

CUBARULE CRQ8 = {
	"quads degree 8, 16 points",
	8, 16, 
        { 0.0			, 0.0			, 0.952509466071562	,-0.952509466071562	, 
	  0.532327454074206	,-0.532327454074206	, 0.684736297951735	,-0.684736297951735	, 
          0.233143240801405	,-0.233143240801405	, 0.927683319306117	,-0.927683319306117	,
          0.453120687403749	,-0.453120687403749	, 0.837503640422812	,-0.837503640422812	},
        { 0.659560131960342	,-0.949142923043125	, 0.765051819557684	, 0.765051819557684	,
          0.936975981088416	, 0.936975981088416	, 0.333656717735747	, 0.333656717735747	, 
         -0.079583272377397	,-0.079583272377397	,-0.272240080612534	,-0.272240080612534	,
         -0.613735353398028	,-0.613735353398028	,-0.888477650535971	,-0.888477650535971	},
	{ 0.			, 0.			, 0.			, 0.			,
	  0.			, 0.			, 0.			, 0.			,
	  0.			, 0.			, 0.			, 0.			,
	  0.			, 0.			, 0.			, 0. },
        { 0.450276776305590	, 0.166570426777813	, 0.098869459933431	, 0.098869459933431	,
          0.153696747140812	, 0.153696747140812	, 0.396686976072903	, 0.396686976072903	, 
          0.352014367945695	, 0.352014367945695	, 0.189589054577798	, 0.189589054577798	, 
          0.375101001147587	, 0.375101001147587	, 0.125618791640072	, 0.125618791640072	}
};


#define b1	0.96884996636197772072
#define b2	0.75027709997890053354
#define b3	0.52373582021442933604
#define b4	0.07620832819261717318
#define c1	0.63068011973166885417
#define c2	0.92796164595956966740
#define c3	0.45333982113564719076
#define c4	0.85261572933366230775
#define w0	0.52674897119341563786
#define w1	0.08887937817019870697
#define w2	0.11209960212959648528
#define w3	0.39828243926207009528
#define w4	0.26905133763978080301
CUBARULE CRQ9 = {
	"quads degree 9, 17 points",
	9, 17, 
        { 0.0	,
	  b1	,-b1	,-c1	, c1	, 
	  b2	,-b2	,-c2	, c2	, 
	  b3	,-b3	,-c3	, c3	, 
	  b4	,-b4	,-c4	, c4	},
        { 0.0	,
          c1	,-c1	, b1	,-b1	,
          c2	,-c2	, b2	,-b2	,
          c3	,-c3	, b3	,-b3	,
          c4	,-c4	, b4	,-b4	},
	{ 0.	,
	  0.	, 0.	, 0.	, 0.	,
	  0.	, 0.	, 0.	, 0.	,
	  0.	, 0.	, 0.	, 0.	,
	  0.	, 0.	, 0.	, 0.    },
        { w0	,
	  w1	, w1	, w1	, w1	,
	  w2	, w2	, w2	, w2	,
	  w3	, w3	, w3	, w3	,
	  w4	, w4	, w4	, w4	}
};
#undef w4
#undef w3
#undef w2
#undef w1
#undef w0
#undef c4
#undef c3
#undef c2
#undef c1
#undef b4
#undef b3
#undef b2
#undef b1





CUBARULE CRT1 = {
	"triangles degree 1, 1 points",
	1, 1,
        { 1./3. },
        { 1./3. },
	{ 0.    },
        { 1.0	}
};


CUBARULE CRT2 = {
	"triangles degree 2, 3 points",
	2, 3,
        { 1./6.	, 1./6.	, 2./3.	},
        { 1./6.	, 2./3.	, 1./6.	},
	{ 0.	, 0.	, 0. },
        { 1./3.	, 1./3.	, 1./3.	}
};


CUBARULE CRT3 = {
	"triangles degree 3, 4 points",
	3, 4,
        {   1./3.,   0.2  ,   0.2  ,   0.6   },
        {   1./3.,   0.2  ,   0.6  ,   0.2   },
	{ 0.	 , 0.	  , 0.	   , 0. },
        { -9./16., 25./48., 25./48., 25./48. }
};




#define w1	3.298552309659655e-1/3.
#define a1	8.168475729804585e-1
#define b1	9.157621350977073e-2
#define c1	b1
#define w2	6.701447690340345e-1/3.
#define a2	1.081030181680702e-1
#define b2	4.459484909159649e-1
#define c2	b2
CUBARULE CRT4 = {
	"triangles degree 4, 6 points",
	4, 6, 
        { a1	, b1	, c1	, a2	, b2	, c2	},
        { b1	, c1	, a1	, b2	, c2	, a2	},
	{ 0.	, 0.	, 0.	, 0.	, 0.	, 0. },
        { w1	, w1	, w1	, w2	, w2	, w2	}
};
#undef c2
#undef b2
#undef a2
#undef w2
#undef c1
#undef b1
#undef a1
#undef w1


#define r	0.1012865073234563
#define s	0.7974269853530873
#define t	1./3.
#define u	0.4701420641051151
#define v	0.05971587178976981
#define A	0.225
#define B	0.1259391805448271
#define C	0.1323941527885062
CUBARULE CRT5 = {
	"triangles degree 5, 7 points",
	5, 7, 
        { t	, r	, r	, s	, u	, u	, v	},
        { t	, r	, s	, r	, u	, v	, u	},
	{ 0.	, 0.	, 0.	, 0.	, 0.	, 0.	, 0. },
        { A	, B	, B	, B	, C	, C	, C	}
};
#undef C
#undef B
#undef A
#undef v
#undef u
#undef t
#undef s
#undef r






#define w1	0.2651702815743450e-1 * 2.
#define a1	0.6238226509439084e-1
#define b1	0.6751786707392436e-1
#define c1	0.8700998678316848
#define w2	0.4388140871444811e-1 * 2.
#define a2	0.5522545665692000e-1
#define b2	0.3215024938520156
#define c2	0.6232720494910644
#define w3	0.2877504278497528e-1 * 2.
#define a3	0.3432430294509488e-1
#define b3	0.6609491961867980
#define c3	0.3047265008681072
#define w4	0.6749318700980879e-1 * 2.
#define a4	0.5158423343536001
#define b4	0.2777161669764050
#define c4	0.2064414986699949
CUBARULE CRT7 = {
	"triangles degree 7, 12 points",
	7, 12, 
        { a1	, b1	, c1	, a2	, b2	, c2	, 
	  a3	, b3	, c3	, a4	, b4	, c4	},
        { b1	, c1	, a1	, b2	, c2	, a2	,
	  b3	, c3	, a3	, b4	, c4	, a4	},
	{ 0.	, 0.	, 0.	, 0.	, 0.	, 0.	,
	  0.	, 0.	, 0.	, 0.	, 0.	, 0. },
        { w1	, w1	, w1	, w2	, w2	, w2	,
	  w3	, w3	, w3	, w4	, w4	, w4	}
};
#undef c4
#undef b4
#undef a4
#undef w4
#undef c3
#undef b3
#undef a3
#undef w3
#undef c2
#undef b2
#undef a2
#undef w2
#undef c1
#undef b1
#undef a1
#undef w1




#define w0	1.443156076777862e-1	
#define a0	3.333333333333333e-1
#define b0	3.333333333333333e-1
#define	c0	b0
#define w1	2.852749028018549e-1/3.
#define a1	8.141482341455413e-2
#define b1	4.592925882927229e-1
#define c1	b1
#define w2	9.737549286959440e-2/3.
#define a2	8.989055433659379e-1
#define b2	5.054722831703103e-2
#define c2	b2
#define w3	3.096521116041552e-1/3.
#define a3	6.588613844964797e-1
#define b3	1.705693077517601e-1
#define c3	b3
#define w4	1.633818850466092e-1/6.
#define a4	8.394777409957211e-3
#define b4	7.284923929554041e-1
#define c4 	2.631128296346387e-1
CUBARULE CRT8 = {
	"triangles degree 8, 16 points",
	8, 16, 
        { a0	, 
	  a1	, b1	, c1	,
	  a2	, b2	, c2	,
	  a3	, b3	, c3	,
	  a4	, a4	, b4	, b4	, c4	, c4	},
        { b0	, 
	  b1	, c1	, a1	,
	  b2	, c2	, a2	,
	  b3	, c3	, a3	,
	  b4	, c4	, c4	, a4	, a4	, b4	},
	{ 0.	,
	  0.	, 0.	, 0.	,
	  0.	, 0.	, 0.	,
	  0.	, 0.	, 0.	,
	  0.	, 0.	, 0.	, 0.	, 0.	, 0. },
        { w0	,
	  w1	, w1	, w1	,
	  w2	, w2	, w2	,
	  w3	, w3	, w3	,
	  w4	, w4	, w4	, w4	, w4	, w4   }
};
#undef c4
#undef b4
#undef a4
#undef w4
#undef c3
#undef b3
#undef a3
#undef w3
#undef c2
#undef b2
#undef a2
#undef w2
#undef c1
#undef b1
#undef a1
#undef w1
#undef c0
#undef b0
#undef a0
#undef w0


#define w0	9.713579628279610e-2   
#define a0	3.333333333333333e-1
#define b0	3.333333333333333e-1
#define c0	b0
#define w1	9.400410068141950e-2/3.
#define a1	2.063496160252593e-2
#define b1	4.896825191987370e-1
#define c1	b1
#define w2	2.334826230143263e-1/3.
#define a2	1.258208170141290e-1
#define b2	4.370895914929355e-1
#define c2	b2
#define w3	2.389432167816273e-1/3.
#define a3	6.235929287619356e-1
#define b3	1.882035356190322e-1
#define c3	b3
#define w4	7.673302697609430e-2/3.
#define a4	9.105409732110941e-1
#define b4	4.472951339445297e-2
#define c4	b4
#define w5	2.597012362637364e-1/6.
#define a5	3.683841205473626e-2
#define b5	7.411985987844980e-1	
#define c5	2.219629891607657e-1
CUBARULE CRT9 = {
	"triangles degree 9, 19 points",
	9, 19, 
        { a0	, 
	  a1	, b1	, c1	,
	  a2	, b2	, c2	,
	  a3	, b3	, c3	,
	  a4	, b4	, c4	,
	  a5	, a5	, b5	, b5	, c5	, c5	},
        { b0	, 
	  b1	, c1	, a1	,
	  b2	, c2	, a2	,
	  b3	, c3	, a3	,
	  b4	, c4	, a4	,
	  b5	, c5	, c5	, a5	, a5	, b5	},
	{ 0.	,
	  0.	, 0.	, 0.	,
	  0.	, 0.	, 0.	,
	  0.	, 0.	, 0.	,
	  0.	, 0.	, 0.	,
	  0.	, 0.	, 0.	, 0.	, 0.	, 0. },
        { w0	,
	  w1	, w1	, w1	,
	  w2	, w2	, w2	,
	  w3	, w3	, w3	,
	  w4	, w4	, w4	,
	  w5	, w5	, w5	, w5	, w5	, w5   }
};
#undef c5
#undef b5
#undef a5
#undef w5
#undef c4
#undef b4
#undef a4
#undef w4
#undef c3
#undef b3
#undef a3
#undef w3
#undef c2
#undef b2
#undef a2
#undef w2
#undef c1
#undef b1
#undef a1
#undef w1
#undef c0
#undef b0
#undef a0
#undef w0




#define	u 1.
#define w 8./9.
CUBARULE CRV1 = {
	"boxes degree 1, 9 points (the corners + center)",
	1 , 9 ,	
        { u	, u	, u	, u	,-u	,-u	,-u	,-u 	, 0.  }, 
        { u	, u	,-u	,-u	, u	, u	,-u	,-u 	, 0.  },
	{ u	,-u	, u	,-u	, u	,-u	, u	,-u     , 0.  },
        {  w 	,  w 	,  w 	,  w 	,  w 	,  w 	,  w 	,  w 	,  w  }
};
#undef u
#undef w


#define	u 0.57735026918962576450 	
CUBARULE CRV3PG = {
	"boxes degree 3, 8 points, product Gauss formula",
	3 , 8 ,	
        { u	, u	, u	, u	,-u	,-u	,-u	,-u 	}, 
        { u	, u	,-u	,-u	, u	, u	,-u	,-u 	},
	{ u	,-u	, u	,-u	, u	,-u	, u	,-u     },
        { 1.0	, 1.0	, 1.0	, 1.0	, 1.0	, 1.0	, 1.0	, 1.0	}
};
#undef u


CUBARULE *quadrule[9] = {&CRQ1, &CRQ2, &CRQ3, &CRQ4, &CRQ5, &CRQ6, &CRQ7, &CRQ8, &CRQ9};


CUBARULE *quadprodrule[3] = {&CRQ3PG, &CRQ5PG, &CRQ7PG};


CUBARULE *trianglerule[9] = {&CRT1, &CRT2, &CRT3, &CRT4, &CRT5, &CRT7, &CRT7, &CRT8, &CRT9};


CUBARULE *boxesrule[1] = {&CRV1};


CUBARULE *boxesprodrule[1] = {&CRV3PG};


static void TransformQuadRule(CUBARULE *rule)
{
	int k;

	for (k=0; k<rule->nrnodes; k++) {
		rule->u[k] = (rule->u[k]+1.)/2.;
		rule->v[k] = (rule->v[k]+1.)/2.;
		rule->w[k] /= 4.;
	}
}


static void TransformCubeRule(CUBARULE *rule)
{
	int k;

	for (k=0; k<rule->nrnodes; k++) {
		rule->u[k] = (rule->u[k]+1.)/2.;
		rule->v[k] = (rule->v[k]+1.)/2.;
		rule->t[k] = (rule->t[k]+1.)/2.;
		rule->w[k] /= 8.;
	}
}


void FixCubatureRules(void)
{
	int i;

	for (i=0; i<9; i++)
		TransformQuadRule(quadrule[i]);
	for (i=0; i<3; i++)
		TransformQuadRule(quadprodrule[i]);
	for (i=0; i<1; i++)
		TransformCubeRule(boxesrule[i]);
	for (i=0; i<1; i++)
		TransformCubeRule(boxesprodrule[i]);
}

#ifdef TEST
#include <stdio.h>


double power(double x, unsigned n)
{
	double pow = 1.;
	while (n>0) {
		pow *= x;
		n--;
	}
	return pow;
}


double f(double u, double v, int a, int b)
{
	return power(u, a) * power(v, b);
}


double integrate(CUBARULE *rule, int a, int b)
{
	int k;
	double res = 0.;

	for (k=0; k<rule->nrnodes; k++)
		res += rule->w[k] * f(rule->u[k], rule->v[k], a, b);

	return res;
}


void testmono(CUBARULE *rule, int maxdegree)
{
	int i, j;

	printf("a \\ b\t");
	for (j=0; j<=maxdegree; j++) 
		printf("\t     %d\t", j);
	printf("\n\n");
	
	for (i=0; i<=maxdegree; i++) {
		printf("%2d\t", i);
		for (j=0; j<=maxdegree; j++) {
			if (i+j > maxdegree)
				printf("\t     .\t");
			else
				printf("%14.14g\t", integrate(rule, i, j));
		}
		printf("\n");
	}
	printf("\n");
}


void testprod(CUBARULE *rule, int maxdegree)
{
	int i, j;

	printf("a \\ b\t");
	for (j=0; j<=maxdegree; j++) 
		printf("\t     %d\t", j);
	printf("\n\n");
	
	for (i=0; i<=maxdegree; i++) {
		printf("%2d\t", i);
		for (j=0; j<=maxdegree; j++) {
			printf("%14.14g\t", integrate(rule, i, j));
		}
		printf("\n");
	}
	printf("\n");
}

int main(int argc, char **argv)
{
	int i;


	for (i=0; i<9; i++) {
		printf("%s\n", quadrule[i]->description);
		testmono(quadrule[i], i+1);
	}


	for (i=0; i<3; i++) {
		printf("%s\n", quadprodrule[i]->description);
		testprod(quadprodrule[i], 2*(i+2)-1);
	}


	for (i=0; i<9; i++) {
		printf("%s\n", trianglerule[i]->description);
		testmono(trianglerule[i], i+1);
	}

	return 0;
}

#endif 
