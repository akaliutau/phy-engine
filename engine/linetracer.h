/* llinetracer.h: global line tracing engine */

#ifndef _GLOBALLINES_H_
#define _GLOBALLINES_H_

#include "hitlist.h"

/* default global line hit point comparsion routine (for sorting
 * global line hits. Use this one if you want to use ProcessSpans()
 * for handling global line hit points in spans of mutually visible
 * points.
 * Returns -1 if hit1 comes before hit2, +1 if hit1 should come after
 * hit2 and 0 if both hits are coincident are are both frontal hits
 * or hits from the back (a situation that occurs e.g. for coplanar
 * polygons) */
extern int DefCompareHits(HITREC *hit1, HITREC *hit2);

/* Determines "spans" connecting a hit from the back with the next frontal
 * hit. Calls do_span for each found span. Requires that the hitlist has
 * been sorted using the default hitlist sorting routine. 
 * Returns number of spans processed. */
extern int ProcessSpans(HITLIST *hits, void (*do_span)(HITREC *start, HITREC *end));

/* example of a span processing routine for ProcessSpans */
extern void PrintSpan(HITREC *start, HITREC *end);

/* default hit processing routine: calles ProcessSpans() with
 * PrintSpan as a span processing routine. */
extern void DefProcessHits(HITLIST *hits);

/* Initialises global line tracing.
 * - sample4d is a routine returning a 4D sample vector with given
 * index, used to generate a global line. This parameter can be 
 * NULL, in which case 4D Niederreiter sampling is used.
 * - compare_hits is a routine that compares to hit records. Use
 * 'DefCompareHits' in order to sort global line hit points in such
 * a way that ProcessSpans() can be used for processing the hits.
 * Also this parameter can be NULL, in which case the hits are
 * not sorted.
 * - process_hits is a pointer to a routine for processing the 
 * sorted global line hits. If sorted using DefCompareHits, process_hits
 * can call ProcessSpans with a routine for handling spans of mutually
 * visible points. */
extern void InitGlobalLines(double* (*sample4d)(int index),
			    int (*compare_hits)(HITREC *hit1, HITREC *hit2),
			    void (*process_hits)(HITLIST *hits));

/* Generates and traces a global line with given index number using
 * the two-points-on-a-bounding-sphere method, using sample4d()
 * for sampling the points on the sphere. If the global line 
 * intersects surfaces in the scene, the hit points are sorted using
 * compare_hits(). Eventually, process_hits() is called for processing
 * the sorted hit points. 
 * Returns FALSE is the generated global line did not hit the scene and
 * returns TRUE if it did. */
extern int DoGlobalLine(int index);

/* World bounding sphere, computed as a side result of InitGlobalLines() */
extern VECTOR glin_Center;
extern float glin_Radius;

#endif /*_GLOBALLINES_H_*/
