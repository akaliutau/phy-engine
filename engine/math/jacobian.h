/* jacobian.h */

#ifndef _JACOBIAN_H_
#define _JACOBIAN_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Jacobian for a quadrilateral patch is J(u,v) = A + B.u + C.v */
typedef struct JACOBIAN {
	float	A, B, C;	
} JACOBIAN;

extern JACOBIAN *JacobianCreate(float A, float B, float C);
extern void JacobianDestroy(JACOBIAN *jacobian);

#ifdef __cplusplus
}
#endif

#endif /*_JACOBIAN_H_*/
