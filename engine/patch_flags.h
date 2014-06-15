/* patch_flags.h: extra PATCH flags (stored in PATCH.flags field) */

#ifndef _PATCH_FAGS_H_
#define _PATCH_FLAGS_H_

#define PATCH_VISIBILITY	0x01
#define PATCH_IS_VISIBLE(patch)	(((patch)->flags & PATCH_VISIBILITY)!=0)
#define PATCH_SET_VISIBLE(patch)	{(patch)->flags |= PATCH_VISIBILITY;}
#define PATCH_SET_INVISIBLE(patch)	{(patch)->flags &= ~PATCH_VISIBILITY;}

#define IGNORE_THIS_PATCH	0x80
#define IGNORE_PATCH(patch)   (((patch)->flags & IGNORE_THIS_PATCH) != 0)
#define PATCH_SET_IGNORE(patch)	{(patch)->flags |= IGNORE_THIS_PATCH;}
#define PATCH_SET_DONT_IGNORE(patch)	{(patch)->flags &= ~IGNORE_THIS_PATCH;}

#define HAS_OVERLAP		0x10
#define PATCH_HAS_OVERLAP(patch)	(((patch)->flags & HAS_OVERLAP) != 0)
#define PATCH_SET_HAS_OVERLAP(patch)	{(patch)->flags |= HAS_OVERLAP;}
#define PATCH_UNSET_HAS_OVERLAP(patch)	{(patch)->flags &= ~HAS_OVERLAP;}

#define HAS_NO_OVERLAP		0x20
#define PATCH_HAS_NO_OVERLAP(patch)	(((patch)->flags & HAS_NO_OVERLAP) != 0)
#define PATCH_SET_HAS_NO_OVERLAP(patch)	{(patch)->flags |= HAS_NO_OVERLAP;}
#define PATCH_UNSET_HAS_NO_OVERLAP(patch)	{(patch)->flags &= ~HAS_NO_OVERLAP;}

#define IS_LARGEST	0x40
#define PATCH_IS_LARGEST(patch)   (((patch)->flags & IS_LARGEST) != 0)
#define PATCH_SET_IS_LARGEST(patch)	{(patch)->flags |= IS_LARGEST;}
#define PATCH_SET_DONT_IS_LARGEST(patch)	{(patch)->flags &= ~IS_LARGEST;}

#endif /*_PATCH_FLAGS_H_*/
