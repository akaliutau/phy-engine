/* materiallist.h: linear lists of MATERIALs */

#ifndef _MATERIALLIST_H_
#define _MATERIALLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "material.h"
#include "List.h"

/* this struct should have the same layout as the struct LIST in generics/List.h, so
 * all generic list routines can be used for lists of MATERIALs */
typedef struct MATERIALLIST {
	struct MATERIAL *material;
	struct MATERIALLIST *next;
} MATERIALLIST;

#define MaterialListCreate	(MATERIALLIST *)ListCreate

#define MaterialListAdd(materiallist, material)	\
        (MATERIALLIST *)ListAdd((LIST *)materiallist, (void *)material)

#define MaterialListCount(materiallist) \
        ListCount((LIST *)materiallist)

#define MaterialListGet(materiallist, index) \
        (MATERIAL *)ListGet((LIST *)materiallist, index)

#define MaterialListNext(pmateriallist) \
        (MATERIAL *)ListNext((LIST **)pmateriallist)

#define MaterialListRemove(materiallist, material) \
        (MATERIALLIST *)ListRemove((LIST *)materiallist, (void *)material)

#define MaterialListIterate(materiallist, proc) \
        ListIterate((LIST *)materiallist, (void (*)(void *))proc)

#define MaterialListIterate1B(materiallist, proc, data) \
        ListIterate1B((LIST *)materiallist, (void (*)(void *, void *))proc, (void *)data)

#define MaterialListDestroy(materiallist) \
        ListDestroy((LIST *)materiallist)

#define MaterialListPrint(out, materiallist) \
        MaterialListIterate1B(materiallist, MaterialPrint, out)

#define ForAllMaterials(mat, matlist) ForAllInList(MATERIAL, mat, matlist)

/* Looks up a material with given name in the given material list. Returns
 * a pointer to the material if found, or (MATERIAL *)NULL if not found. */
extern MATERIAL *MaterialLookup(MATERIALLIST *MaterialLib, char *name);

#ifdef __cplusplus
}
#endif

#endif /* _MATERIALLIST_H_ */
