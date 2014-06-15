

#ifndef lint
static const char SCCSid[] = "@(#)words.c 1.2 5/10/95 LBL";
#endif



#include  <ctype.h>

#ifdef  BSD
#define  strchr		index
#endif

#define  NULL		0

extern char  *strchr(const char *, int);


char *
iskip(register char *s)			
                  
{
	while (isspace(*s))
		s++;
	if (*s == '-' || *s == '+')
		s++;
	if (!isdigit(*s))
		return(NULL);
	do
		s++;
	while (isdigit(*s));
	return(s);
}


char *
fskip(register char *s)			
                  
{
	register char  *cp;

	while (isspace(*s))
		s++;
	if (*s == '-' || *s == '+')
		s++;
	cp = s;
	while (isdigit(*cp))
		cp++;
	if (*cp == '.') {
		cp++; s++;
		while (isdigit(*cp))
			cp++;
	}
	if (cp == s)
		return(NULL);
	if (*cp == 'e' || *cp == 'E')
		return(iskip(cp+1));
	return(cp);
}


int
isint(char *s)			
         
{
	register char  *cp;

	cp = iskip(s);
	return(cp != NULL && *cp == '\0');
}


int
isintd(char *s, char *ds)			
              
{
	register char  *cp;

	cp = iskip(s);
	return(cp != NULL && strchr(ds, *cp) != NULL);
}


int
isflt(char *s)			
         
{
	register char  *cp;

	cp = fskip(s);
	return(cp != NULL && *cp == '\0');
}


int
isfltd(char *s, char *ds)			
              
{
	register char  *cp;

	cp = fskip(s);
	return(cp != NULL && strchr(ds, *cp) != NULL);
}


int
isname(register char *s)			
                  
{
	while (*s == '_')			
		s++;
	if (!isascii(*s) || !isalpha(*s))	
		return(0);
	while (isascii(*++s) && isgraph(*s))	
		;
	return(*s == '\0');			
}
