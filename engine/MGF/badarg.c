

#ifndef lint
static const char SCCSid[] = "@(#)badarg.c 1.1 6/21/94 LBL";
#endif



#include <ctype.h>
#include "words.h"

#define NULL		0

int
badarg(int ac, register char **av, register char *fl)		
   	   
             	     
             	    
{
	register int  i;

	if (fl == NULL)
		fl = "";		
	for (i = 1; *fl; i++,av++,fl++) {
		if (i > ac || *av == NULL)
			return(-1);
		switch (*fl) {
		case 's':		
			if (**av == '\0' || isspace(**av))
				return(i);
			break;
		case 'i':		
			if (!isintd(*av, " \t\r\n"))
				return(i);
			break;
		case 'f':		
			if (!isfltd(*av, " \t\r\n"))
				return(i);
			break;
		default:		
			return(-1);
		}
	}
	return(0);		
}
