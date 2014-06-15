

#ifndef lint
static const char SCCSid[] = "@(#)xf.c 1.12 11/29/95 LBL";
#endif



#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "parser.h"

#define  d2r(a)		((PI/180.)*(a))

#define  checkarg(a,l)	if (av[i][a] || badarg(ac-i-1,av+i+1,l)) goto done

MAT4  m4ident = MAT4IDENT;

static MAT4  m4tmp;		

XF_SPEC	*xf_context;		
char	**xf_argend;		
static char	**xf_argbeg;	


int
xf_handler(int ac, char **av)		
   	   
    	     
{
	register XF_SPEC	*spec;
	register int	n;
	int	rv;

	if (ac == 1) {			
		if ((spec = xf_context) == NULL)
			return(MG_ECNTXT);
		n = -1;
		if (spec->xarr != NULL) {	
			register struct xf_array	*ap = spec->xarr;

			(void)xf_aname((struct xf_array *)NULL);
			n = ap->ndim;
			while (n--) {
				if (++ap->aarg[n].i < ap->aarg[n].n)
					break;
				(void)strcpy(ap->aarg[n].arg, "0");
				ap->aarg[n].i = 0;
			}
			if (n >= 0) {
				if ((rv = mg_fgoto(&ap->spos)) != MG_OK)
					return(rv);
				sprintf(ap->aarg[n].arg, "%d", ap->aarg[n].i);
				(void)xf_aname(ap);
			}
		}
		if (n < 0) {			
			xf_context = spec->prev;
			free_xf(spec);
			return(MG_OK);
		}
	} else {			
		if ((spec = new_xf(ac-1, av+1)) == NULL)
			return(MG_EMEM);
		if (spec->xarr != NULL)
			(void)xf_aname(spec->xarr);
		spec->prev = xf_context;	
		xf_context = spec;
	}
					
	n = xf_ac(spec);
	n -= xf_ac(spec->prev);		
	if (xf(&spec->xf, n, xf_av(spec)) != n)
		return(MG_ETYPE);
					
	if ((spec->rev = (spec->xf.sca < 0.)))
		spec->xf.sca = -spec->xf.sca;
					
	if (spec->prev != NULL) {
		multmat4(spec->xf.xfm, spec->xf.xfm, spec->prev->xf.xfm);
		spec->xf.sca *= spec->prev->xf.sca;
		spec->rev ^= spec->prev->rev;
	}
	spec->xid = comp_xfid(spec->xf.xfm);	
	return(MG_OK);
}


XF_SPEC *
new_xf(int ac, char **av)			
   	   
    	     
{
	register XF_SPEC	*spec;
	register int	i;
	char	*cp;
	int	n, ndim;

	ndim = 0;
	n = 0;				
	for (i = 0; i < ac; i++)
		if (!strcmp(av[i], "-a")) {
			ndim++;
			i++;
		} else
			n += strlen(av[i]) + 1;
	if (ndim > XF_MAXDIM)
		return(NULL);
	spec = (XF_SPEC *)malloc(sizeof(XF_SPEC) + n);
	if (spec == NULL)
		return(NULL);
	if (ndim) {
		spec->xarr = (struct xf_array *)malloc(sizeof(struct xf_array));
		if (spec->xarr == NULL)
			return(NULL);
		mg_fgetpos(&spec->xarr->spos);
		spec->xarr->ndim = 0;		
	} else
		spec->xarr = NULL;
	spec->xac = ac + xf_argc;
					
	if (xf_argbeg == NULL || xf_av(spec) < xf_argbeg) {
		register char	**newav =
				(char **)malloc((spec->xac+1)*sizeof(char *));
		if (newav == NULL)
			return(NULL);
		for (i = xf_argc; i-- > 0; )
			newav[ac+i] = xf_argend[i-xf_context->xac];
		*(xf_argend = newav + spec->xac) = NULL;
		if (xf_argbeg != NULL)
			free((MEM_PTR)xf_argbeg);
		xf_argbeg = newav;
	}
	cp = (char *)(spec + 1);	
	for (i = 0; i < ac; i++)
		if (!strcmp(av[i], "-a")) {
			xf_av(spec)[i++] = "-i";
			xf_av(spec)[i] = strcpy(
					spec->xarr->aarg[spec->xarr->ndim].arg,
					"0");
			spec->xarr->aarg[spec->xarr->ndim].i = 0;
			spec->xarr->aarg[spec->xarr->ndim++].n = atoi(av[i]);
		} else {
			xf_av(spec)[i] = strcpy(cp, av[i]);
			cp += strlen(av[i]) + 1;
		}
	return(spec);
}


void
free_xf(register XF_SPEC *spec)			
                	      
{
	if (spec->xarr != NULL)
		free((MEM_PTR)spec->xarr);
	free((MEM_PTR)spec);
}


int
xf_aname(register struct xf_array *ap)			
                        	    
{
	static char	oname[10*XF_MAXDIM];
	static char	*oav[3] = {mg_ename[MG_E_OBJECT], oname};
	register int	i;
	register char	*cp1, *cp2;

	if (ap == NULL)
		return(mg_handle(MG_E_OBJECT, 1, oav));
	cp1 = oname;
	*cp1 = 'a';
	for (i = 0; i < ap->ndim; i++) {
		for (cp2 = ap->aarg[i].arg; *cp2; )
			*++cp1 = *cp2++;
		*++cp1 = '.';
	}
	*cp1 = '\0';
	return(mg_handle(MG_E_OBJECT, 2, oav));
}


long
comp_xfid(register double (*xfm)[4])			
             	    
{
	static char	shifttab[64] = { 15, 5, 11, 5, 6, 3,
				9, 15, 13, 2, 13, 5, 2, 12, 14, 11,
				11, 12, 12, 3, 2, 11, 8, 12, 1, 12,
				5, 4, 15, 9, 14, 5, 13, 14, 2, 10,
				10, 14, 12, 3, 5, 5, 14, 6, 12, 11,
				13, 9, 12, 8, 1, 6, 5, 12, 7, 13,
				15, 8, 9, 2, 6, 11, 9, 11 };
	register int	i;
	register long	xid;

	xid = 0;			
	for (i = 0; i < sizeof(MAT4)/sizeof(unsigned short); i++)
		xid ^= (long)(((unsigned short *)xfm)[i]) << shifttab[i&63];
	return(xid);
}


void
xf_clear(void)			
{
	register XF_SPEC	*spec;

	if (xf_argbeg != NULL) {
		free((MEM_PTR)xf_argbeg);
		xf_argbeg = xf_argend = NULL;
	}
	while ((spec = xf_context) != NULL) {
		xf_context = spec->prev;
		free_xf(spec);
	}
}


void
xf_xfmpoint(double *v1, double *v2)		
     	       
{
	if (xf_context == NULL) {
		VCOPY(v1, v2);
		return;
	}
	multp3(v1, v2, xf_context->xf.xfm);
}


void
xf_xfmvect(double *v1, double *v2)		
     	       
{
	if (xf_context == NULL) {
		VCOPY(v1, v2);
		return;
	}
	multv3(v1, v2, xf_context->xf.xfm);
}


void
xf_rotvect(double *v1, double *v2)		
     	       
{
	xf_xfmvect(v1, v2);
	if (xf_context == NULL)
		return;
	v1[0] /= xf_context->xf.sca;
	v1[1] /= xf_context->xf.sca;
	v1[2] /= xf_context->xf.sca;
}


double
xf_scale(double d)			
      	  
{
	if (xf_context == NULL)
		return(d);
	return(d*xf_context->xf.sca);
}


void
multmat4(double (*m4a)[4], register double (*m4b)[4], register double (*m4c)[4])		
          
                        
{
	register int  i, j;
	
	for (i = 4; i--; )
		for (j = 4; j--; )
			m4tmp[i][j] = m4b[i][0]*m4c[0][j] +
				      m4b[i][1]*m4c[1][j] +
				      m4b[i][2]*m4c[2][j] +
				      m4b[i][3]*m4c[3][j];
	
	copymat4(m4a, m4tmp);
}


void
multv3(double *v3a, register double *v3b, register double (*m4)[4])	
           
                    
                  
{
	m4tmp[0][0] = v3b[0]*m4[0][0] + v3b[1]*m4[1][0] + v3b[2]*m4[2][0];
	m4tmp[0][1] = v3b[0]*m4[0][1] + v3b[1]*m4[1][1] + v3b[2]*m4[2][1];
	m4tmp[0][2] = v3b[0]*m4[0][2] + v3b[1]*m4[1][2] + v3b[2]*m4[2][2];
	
	v3a[0] = m4tmp[0][0];
	v3a[1] = m4tmp[0][1];
	v3a[2] = m4tmp[0][2];
}


void
multp3(register double *p3a, double *p3b, register double (*m4)[4])		
                    
           
                  
{
	multv3(p3a, p3b, m4);	
	p3a[0] += m4[3][0];	
	p3a[1] += m4[3][1];
	p3a[2] += m4[3][2];
}


int
xf(register XF *ret, int ac, char **av)			
                  
        
           
{
	MAT4  xfmat, m4;
	double  xfsca, dtmp;
	int  i, icnt;

	setident4(ret->xfm);
	ret->sca = 1.0;

	icnt = 1;
	setident4(xfmat);
	xfsca = 1.0;

	for (i = 0; i < ac && av[i][0] == '-'; i++) {
	
		setident4(m4);
		
		switch (av[i][1]) {
	
		case 't':			
			checkarg(2,"fff");
			m4[3][0] = atof(av[++i]);
			m4[3][1] = atof(av[++i]);
			m4[3][2] = atof(av[++i]);
			break;

		case 'r':			
			switch (av[i][2]) {
			case 'x':
				checkarg(3,"f");
				dtmp = d2r(atof(av[++i]));
				m4[1][1] = m4[2][2] = cos(dtmp);
				m4[2][1] = -(m4[1][2] = sin(dtmp));
				break;
			case 'y':
				checkarg(3,"f");
				dtmp = d2r(atof(av[++i]));
				m4[0][0] = m4[2][2] = cos(dtmp);
				m4[0][2] = -(m4[2][0] = sin(dtmp));
				break;
			case 'z':
				checkarg(3,"f");
				dtmp = d2r(atof(av[++i]));
				m4[0][0] = m4[1][1] = cos(dtmp);
				m4[1][0] = -(m4[0][1] = sin(dtmp));
				break;
			default:
			  { float x, y, z, a, c, s, t, A, B;
			  checkarg(2,"ffff");
			  x = atof(av[++i]);
			  y = atof(av[++i]);
			  z = atof(av[++i]);
			  a = d2r(atof(av[++i]));
			  s = sqrt(x*x + y*y + z*z);
			  x /= s; y /= s; z /= s;
			  c = cos(a);
			  s = sin(a);
			  t = 1-c;
			  m4[0][0] = t*x*x + c;
			  m4[1][1] = t*y*y + c;
			  m4[2][2] = t*z*z + c;
			  A = t*x*y;
			  B = s*z;
			  m4[0][1] = A+B;
			  m4[1][0] = A-B;
			  A = t*x*z;
			  B = s*y;
			  m4[0][2] = A-B;
			  m4[2][0] = A+B;
			  A = t*y*z;
			  B = s*x;
			  m4[1][2] = A+B;
			  m4[2][1] = A-B;
			  }
			}
			break;

		case 's':			
		  switch (av[i][2]) {
		  case 'x':
		    checkarg(3,"f");
		    dtmp = atof(av[i+1]);
		    if (dtmp == 0.0) goto done;
		    m4[0][0] = dtmp;
		    break;
		  case 'y':
		    checkarg(3,"f");
		    dtmp = atof(av[i+1]);
		    if (dtmp == 0.0) goto done;
		    m4[1][1] = dtmp;
		    break;
		  case 'z':
		    checkarg(3,"f");
		    dtmp = atof(av[i+1]);
		    if (dtmp == 0.0) goto done;
		    m4[2][2] = dtmp;
		    break;
		  default:
		    checkarg(2,"f");
		    dtmp = atof(av[i+1]);
		    if (dtmp == 0.0) goto done;
		    xfsca *=
		      m4[0][0] = 
		      m4[1][1] = 
		      m4[2][2] = dtmp;
		    break;
		  }
		  i++;
		  break;

		case 'm':			
			switch (av[i][2]) {
			case 'x':
				checkarg(3,"");
				xfsca *=
				m4[0][0] = -1.0;
				break;
			case 'y':
				checkarg(3,"");
				xfsca *=
				m4[1][1] = -1.0;
				break;
			case 'z':
				checkarg(3,"");
				xfsca *=
				m4[2][2] = -1.0;
				break;
			default:
				goto done;
			}
			break;

		case 'i':			
			checkarg(2,"i");
			while (icnt-- > 0) {
				multmat4(ret->xfm, ret->xfm, xfmat);
				ret->sca *= xfsca;
			}
			icnt = atoi(av[++i]);
			setident4(xfmat);
			xfsca = 1.0;
			continue;

		default:
			goto done;

		}
		multmat4(xfmat, xfmat, m4);
	}
done:
	while (icnt-- > 0) {
		multmat4(ret->xfm, ret->xfm, xfmat);
		ret->sca *= xfsca;
	}
	return(i);
}
