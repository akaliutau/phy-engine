

#include "Array.h"
#include "pools.h"
#include "private.h"


ARRAY	*ArrayCreate(int length)
{
	ARRAY *array;

	array = (ARRAY *)Alloc(sizeof(ARRAY));
	array->length = length;
	if (length > 0) 
		array->pel = (void **)Alloc(length * sizeof(void *));
	else
		array->pel = (void **)NULL;
	array->nrels = 0;

	return array;
}


void	ArrayDestroy(ARRAY *array)
{
	if (array->length > 0)
		Free((char *)array->pel, array->length * sizeof(void *));
	Free((char *)array, sizeof(ARRAY));
}


void 	ArrayGrow(ARRAY *array, int extral)
{
	array->pel = (void *)Realloc((char *)array->pel, 
			     array->length * sizeof(void *),
			     extral * sizeof(void *));
	array->length += extral;
}


int	ArrayAdd(ARRAY *array, void *element)
{

	if (array->nrels > array->length)
		GdtFatal("ArrayAdd", "more elements in array than possible");


	if (array->nrels == array->length)
		ArrayGrow(array, 1);


	array->pel[array->nrels] = element;
	array->nrels++;

	return array->nrels-1;
}


void	ArrayIterate(ARRAY *array, void (*elmethod)(void *))
{
	int i;
	
	for (i=0; i<array->nrels; i++)
		elmethod(array->pel[i]);
}




