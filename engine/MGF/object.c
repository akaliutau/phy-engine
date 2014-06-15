

#ifndef lint
static const char SCCSid[] = "@(#)object.c 1.5 11/29/95 LBL";
#endif



#include <stdio.h>
#include <string.h>
#include "parser.h"


int	obj_nnames;			
char	**obj_name;			

static int	obj_maxname;		

#define ALLOC_INC	16		


int
obj_handler(int ac, char **av)		
   	   
    	     
{
	if (ac == 1) {				
		if (obj_nnames < 1)
			return(MG_ECNTXT);
		free((MEM_PTR)obj_name[--obj_nnames]);
		obj_name[obj_nnames] = NULL;
		return(MG_OK);
	}
	if (ac != 2)
		return(MG_EARGC);
	if (!isname(av[1]))
		return(MG_EILL);
	if (obj_nnames >= obj_maxname-1) {	
		if (!obj_maxname)
			obj_name = (char **)malloc(
				(obj_maxname=ALLOC_INC)*sizeof(char *));
		else
			obj_name = (char **)realloc((MEM_PTR)obj_name,
				(obj_maxname+=ALLOC_INC)*sizeof(char *));
		if (obj_name == NULL)
			return(MG_EMEM);
	}
						
	obj_name[obj_nnames] = (char *)malloc(strlen(av[1])+1);
	if (obj_name[obj_nnames] == NULL)
		return(MG_EMEM);
	strcpy(obj_name[obj_nnames++], av[1]);
	obj_name[obj_nnames] = NULL;
	return(MG_OK);
}


void
obj_clear(void)			
{
	while (obj_nnames)
		free((MEM_PTR)obj_name[--obj_nnames]);
	if (obj_maxname) {
		free((MEM_PTR)obj_name);
		obj_maxname = 0;
	}
}
