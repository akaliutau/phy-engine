





#include <removal_queue.h>
#include <simplify.h>
#include <stdio.h>
#include <stdlib.h>




















void removal_queue_init(RemovalQueue *queue, int size)
{
    int i;
    
    ALLOCN(queue->queue, int, size);
    ALLOCN(queue->in_queue, char, size);
    queue->max_size = size;
    
    for (i=0; i<size; i++)
    {
	queue->in_queue[i] = FALSE;
	queue->queue[i] = -1;
    }
    queue->first = 0;
    queue->last = queue->max_size - 1;
    queue->size = 0;
    return;
} 


void removal_queue_destroy(RemovalQueue *queue)
{
    FREE(queue->in_queue);
    FREE(queue->queue);
    queue->first = queue->last = queue->size = queue->max_size = 0;
    return;
} 


int removal_queue_extract(RemovalQueue *queue)
{
    int retval;
    
    if (queue->size <= 0)
	return -1;

    retval = queue->queue[queue->first];
    queue->first = (queue->first + 1) % queue->max_size;
    queue->size--;
    queue->in_queue[retval] = FALSE;
    return retval;
} 


void removal_queue_touch(RemovalQueue *queue, int id)
{
    if ((id < 0) || (id >= queue->max_size))
	return;
    if (queue->in_queue[id] == TRUE)
	return;

    queue->last = (queue->last + 1) % queue->max_size;
    queue->queue[queue->last] = id;
    queue->in_queue[id] = TRUE;
    queue->size++;
} 



