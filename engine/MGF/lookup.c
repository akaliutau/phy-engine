

#ifndef lint
static const char SCCSid[] = "@(#)lookup.c 1.5 4/11/95 LBL";
#endif



#include <stdlib.h>
#include <stdio.h>
#include "lookup.h"

#ifndef MEM_PTR
#define MEM_PTR		void *
#endif

extern MEM_PTR	calloc(size_t, size_t);


int
lu_init(register LUTAB *tbl, int nel)		
              	     
   	    
{
	static int  hsiztab[] = {
		31, 61, 127, 251, 509, 1021, 2039, 4093, 8191, 16381, 
		32749, 65521, 131071, 262139, 524287, 1048573, 2097143, 
		4194301, 8388593, 0
	};
	register int  *hsp;

	nel += nel>>1;			
	for (hsp = hsiztab; *hsp; hsp++)
		if (*hsp > nel)
			break;
	if (!(tbl->tsiz = *hsp))
		tbl->tsiz = nel*2 + 1;		
	tbl->tabl = (LUENT *)calloc(tbl->tsiz, sizeof(LUENT));
	if (tbl->tabl == NULL)
		tbl->tsiz = 0;
	tbl->ndel = 0;

#ifdef LU_STATS
	tbl->nent = 0;
	tbl->collisions = tbl->accesses = 0;
	{int i; LUENT *le;
	for (i=0, le=tbl->tabl; i<tbl->tsiz; i++, le++) {
	  le->accesses = le->collisions = 0;
	}
	}
#endif
	return(tbl->tsiz);
}

static unsigned char shuffle[256] = {
  0, 157,  58, 215, 116,  17, 174,  75, 232, 133,  34, 191,  92, 249, 150,  51, 
208, 109,  10, 167,  68, 225, 126,  27, 184,  85, 242, 143,  44, 201, 102,   3, 
160,  61, 218, 119,  20, 177,  78, 235, 136,  37, 194,  95, 252, 153,  54, 211, 
112,  13, 170,  71, 228, 129,  30, 187,  88, 245, 146,  47, 204, 105,   6, 163, 
 64, 221, 122,  23, 180,  81, 238, 139,  40, 197,  98, 255, 156,  57, 214, 115, 
 16, 173,  74, 231, 132,  33, 190,  91, 248, 149,  50, 207, 108,   9, 166,  67, 
224, 125,  26, 183,  84, 241, 142,  43, 200, 101,   2, 159,  60, 217, 118,  19, 
176,  77, 234, 135,  36, 193,  94, 251, 152,  53, 210, 111,  12, 169,  70, 227, 
128,  29, 186,  87, 244, 145,  46, 203, 104,   5, 162,  63, 220, 121,  22, 179, 
 80, 237, 138,  39, 196,  97, 254, 155,  56, 213, 114,  15, 172,  73, 230, 131, 
 32, 189,  90, 247, 148,  49, 206, 107,   8, 165,  66, 223, 124,  25, 182,  83, 
240, 141,  42, 199, 100,   1, 158,  59, 216, 117,  18, 175,  76, 233, 134,  35, 
192,  93, 250, 151,  52, 209, 110,  11, 168,  69, 226, 127,  28, 185,  86, 243, 
144,  45, 202, 103,   4, 161,  62, 219, 120,  21, 178,  79, 236, 137,  38, 195, 
 96, 253, 154,  55, 212, 113,  14, 171,  72, 229, 130,  31, 188,  89, 246, 147, 
 48, 205, 106,   7, 164,  65, 222, 123,  24, 181,  82, 239, 140,  41, 198,  99
};

long
lu_shash(register char *s)			
                  
{
	register int	i = 0;
	register long	h = 0;
	register unsigned char *t = (unsigned char *)s;

	while (*t)
	  h ^= (long)shuffle[*t++] << ((i+=11) & 0xf);

	return(h);
}

#ifdef LU_STATS
static void lu_dump_stats(LUTAB *tbl)
{
  char fname[100];
  FILE *fp;

  sprintf(fname, "htab%d.stats", tbl->tsiz);
  fprintf(stderr, "Dumping stats in '%s' ...\n", fname);
  fp = fopen(fname, "w");

  fprintf(fp, "# hashvalue accesses collisions/access \n");
  for (i=0, le=tbl->tabl; i<tbl->tsiz; i++, le++) {
    fprintf(fp, "%d %d %g\n", i, le->accesses, 
	    le->accesses ? (float)(le->collisions)/(float)(le->accesses) : 0.);
  }
  fclose(fp);

  fprintf(stderr, "table size %d, accesses = %d, av. collisions/access = %g\n",
	  tbl->tsiz, tbl->accesses, (float)(tbl->collisions) / (float)(tbl->accesses));
}
#endif

int lu_realloc(LUTAB *tbl, int nel)
{
	register int	i;
	register LUENT	*le;
	int	oldtsiz;
	LUENT	*oldtabl;

#ifdef LU_STATS
	lu_dump_stats(tbl);
#endif

	oldtabl = tbl->tabl;
	oldtsiz = tbl->tsiz;
	i = tbl->ndel;
	if (!lu_init(tbl, nel)) {	
		tbl->tabl = oldtabl;
		tbl->tsiz = oldtsiz;
		tbl->ndel = i;
		return 0;
	}

	
	for (i=0, le=oldtabl; i<oldtsiz; i++, le++) {
	  if (le->key != NULL)
	  {
	    if (le->data != NULL) {
	      *lu_find(tbl, le->key) = *le;
#ifdef STATS
	      tbl->nent++;
#endif
	    } 
	    else
	    {
	      if (tbl->freek != NULL)
		(*tbl->freek)(le->key);
	    }
	  }
	}
	free((MEM_PTR)oldtabl);

	return tbl->tsiz;
}

LUENT *
lu_find(register LUTAB *tbl, char *key)		
              	     
    	     
{
	long	hval;
	int	ndx;
	register int	i, n;
	register LUENT	*le;

					
	if (tbl->tsiz <= 0) lu_init(tbl, 1);
	hval = (*tbl->hashf)(key);

tryagain:
	ndx = hval % tbl->tsiz; le = &tbl->tabl[ndx]; i=0; n=-1;
	do {
		if (le->key == NULL) {
			le->hval = hval;
#ifdef LU_STATS
			break;
#else
			return le;
#endif
		}
		if (le->hval == hval && 
		    (tbl->keycmp == NULL || (*tbl->keycmp)(le->key, key)==0)) {
#ifdef LU_STATS
			break;
#else
			return le;
#endif
		}

		
		i++;
		n += 2;
		le += n;
		if ((ndx += n) >= tbl->tsiz) {	
		  ndx = ndx % tbl->tsiz;
		  le = &tbl->tabl[ndx];
		}
	} while (i < tbl->tsiz);
#ifdef LU_STATS
	tbl->collisions += i;
	tbl->accesses ++;
	tbl->tabl[hval % tbl->tsiz].collisions += i;
	tbl->tabl[hval % tbl->tsiz].accesses ++;
	if (i < tbl->size)
	        return(le);
#endif

					
	if (!lu_realloc(tbl, tbl->tsiz-tbl->ndel+1))
	        return (NULL);

	goto tryagain;			
}


void lu_assoc(LUTAB *tbl, LUENT *le, char *key, char *data)
{
  le->key = key;
  le->data = data;

#ifdef STATS
  tbl->nent ++;
#endif
#ifdef NEVER
  if (tbl->nent * 100 / tbl->tsiz > 90) 
    lu_realloc(tbl, tbl->nent);
#endif
}


void
lu_delete(register LUTAB *tbl, char *key)		
              	     
    	     
{
	register LUENT	*le;

	if ((le = lu_find(tbl, key)) == NULL)
		return;
	if (le->key == NULL || le->data == NULL)
		return;
	if (tbl->freed != NULL)
		(*tbl->freed)(le->data);
	le->data = NULL;
	tbl->ndel++;
#ifdef STATS
	tbl->nent--;
#endif
}


void
lu_done(register LUTAB *tbl)			
              	     
{
	register LUENT	*tp;

	if (!tbl->tsiz)
		return;
	for (tp = tbl->tabl + tbl->tsiz; tp-- > tbl->tabl; )
		if (tp->key != NULL) {
			if (tbl->freek != NULL)
				(*tbl->freek)(tp->key);
			if (tp->data != NULL && tbl->freed != NULL)
				(*tbl->freed)(tp->data);
		}
	free((MEM_PTR)tbl->tabl);
	tbl->tabl = NULL;
	tbl->tsiz = 0;
	tbl->ndel = 0;
#ifdef STATS
	tbl->nent = 0;
#endif
}
