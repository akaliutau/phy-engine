/* array.h: rijen */

#ifndef _ARRAY_H_
#define _ARRAY_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ARRAY {
	int	length,	/* aantal voorziene plaatsen */
	        nrels;	/* aantal elementen ingevuld */
	void	**pel;	/* array van pointers naar de elementen */
} ARRAY;

/* creeert een array voor gegevens aantal elementen */
extern ARRAY	*ArrayCreate(int length);

/* geeft een array terug vrij. Vooraleer de array vrij te geven is
 * het aan te raden eerst de elementen zelf vrij te geven. Dat kan met
 * de iterator. */
extern void	ArrayDestroy(ARRAY *array);

/* maakt de array extral elementen groter */
extern void 	ArrayGrow(ARRAY *array, int extral);

/* voegt een element toe aan de array, de array wordt groter
 * gemaakt indien nodig. Geeft de index van het toegevoegde element terug. 
 * De indices beginnen te tellen vanaf nul */
extern int	ArrayAdd(ARRAY *array, void *element);

/* iterator: voer de gegeven procedure uit op ieder element van de array, 
 * bijvoorbeeld om de elementen te vernietigen vooraleer de array te
 * vernietigen. Het tweede argument is de te herhalen procedure. */
extern void	ArrayIterate(ARRAY *array, void (*elmethod)(void *));

#ifdef __cplusplus
}
#endif

#endif /* _ARRAY_H_ */
