/* transform.h: all about transformations */

#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "./math/Float.h"
#include "vectortype.h"

/* 
 * |u'|   |m[0][0]  m[0][1]|   | u |   |t[0]|
 * |  | = |                | * |   | + |    |
 * |v'|   |m[1][0]  m[1][1]|   | v |   |t[1]|
 */
typedef struct TRANSFORM2D {
  float m[2][2], t[2];
} TRANSFORM2D;

#define TRANSFORM_POINT_2D(trans, src, dst) {POINT2D _d_;		\
  _d_.u = (trans).m[0][0] * (src).u + (trans).m[0][1] * (src).v + (trans).t[0];\
  _d_.v = (trans).m[1][0] * (src).u + (trans).m[1][1] * (src).v + (trans).t[1];\
  (dst) = _d_;}

#define PRINT_TRANSFORM2D(fp, trans) {					\
  fprintf(fp, "\t%f %f    %f\n", (trans).m[0][0], (trans).m[0][1], (trans).t[0]); \
  fprintf(fp, "\t%f %f    %f\n", (trans).m[0][1], (trans).m[1][1], (trans).t[1]);}

/* xf(p) = xf2(xf1(p)) */
#define PRECONCAT_TRANSFORM2D(xf2, xf1, xf) {TRANSFORM2D _xf_;		\
  _xf_.m[0][0] = (xf2).m[0][0] * (xf1).m[0][0] + (xf2).m[0][1] * (xf1).m[1][0];	\
  _xf_.m[0][1] = (xf2).m[0][0] * (xf1).m[0][1] + (xf2).m[0][1] * (xf1).m[1][1];	\
  _xf_.m[1][0] = (xf2).m[1][0] * (xf1).m[0][0] + (xf2).m[1][1] * (xf1).m[1][0];	\
  _xf_.m[1][1] = (xf2).m[1][0] * (xf1).m[0][1] + (xf2).m[1][1] * (xf1).m[1][1];	\
  _xf_.t[0]  = (xf2).m[0][0] * (xf1).t[0]  + (xf2).m[0][1] * (xf1).t[1] + (xf2).t[0];\
  _xf_.t[1]  = (xf2).m[1][0] * (xf1).t[0]  + (xf2).m[1][1] * (xf1).t[1] + (xf2).t[1];\
  (xf) = _xf_;}


/* 4x4 transform */
typedef struct TRANSFORM {
  float m[4][4];
} TRANSFORM;

extern TRANSFORM IdentityTransform;

#define SET_3X3MATRIX(m, a, b, c, d, e, f, g, h, i) {		\
m[0][0] = a; m[0][1] = b; m[0][2] = c;				\
m[1][0] = d; m[1][1] = e; m[1][2] = f;				\
m[2][0] = g; m[2][1] = h; m[2][2] = i;				\
}

extern void PrintTransform(FILE *out, TRANSFORM xf);

/* xf(p) = xf2(xf1(p)) */
extern TRANSFORM TransCompose(TRANSFORM xf2, TRANSFORM xf1);

/* Create scaling, ... transform. The transforms behave identically as the
 * corresponding transforms in OpenGL. */
extern TRANSFORM Scale(VECTOR s);
extern TRANSFORM RotateX(float angle /*radians*/), RotateY(float angle), RotateZ(float angle);
extern TRANSFORM Rotate(float angle /* radians */, VECTOR axis);
extern TRANSFORM Translate(VECTOR t);
extern TRANSFORM LookAt(VECTOR eye, VECTOR centre, VECTOR up);
extern TRANSFORM Perspective(float fov /*radians*/, float aspect, float near, float far);
extern TRANSFORM Ortho(float left, float right, float bottom, float top, float near, float far);

/* Recovers the rotation axis and angle from the given rotation matrix.
 * There is no check whether the transform really is a rotation. */
extern void RecoverRotation(TRANSFORM xf, float *angle, VECTOR *axis);

#define TRANSFORM_POINT_3D(trans, src, dst) {POINT _d_;			\
_d_.x = (trans).m[0][0] * (src).x + (trans).m[0][1] * (src).y + (trans).m[0][2] * (src).z + (trans).m[0][3];	\
_d_.y = (trans).m[1][0] * (src).x + (trans).m[1][1] * (src).y + (trans).m[1][2] * (src).z + (trans).m[1][3];	\
_d_.z = (trans).m[2][0] * (src).x + (trans).m[2][1] * (src).y + (trans).m[2][2] * (src).z + (trans).m[2][3];   	\
(dst) = _d_;}

#define TRANSFORM_VECTOR_3D(trans, src, dst) {VECTOR _d_;			\
_d_.x = (trans).m[0][0] * (src).x + (trans).m[0][1] * (src).y + (trans).m[0][2] * (src).z;	\
_d_.y = (trans).m[1][0] * (src).x + (trans).m[1][1] * (src).y + (trans).m[1][2] * (src).z;	\
_d_.z = (trans).m[2][0] * (src).x + (trans).m[2][1] * (src).y + (trans).m[2][2] * (src).z;   	\
(dst) = _d_;}

#define TRANSFORM_POINT_4D(trans, src, dst) {POINT4D _d_;			\
_d_.x = (trans).m[0][0] * (src).x + (trans).m[0][1] * (src).y + (trans).m[0][2] * (src).z + (trans).m[0][3] * (src).w;	\
_d_.y = (trans).m[1][0] * (src).x + (trans).m[1][1] * (src).y + (trans).m[1][2] * (src).z + (trans).m[1][3] * (src).w;	\
_d_.z = (trans).m[2][0] * (src).x + (trans).m[2][1] * (src).y + (trans).m[2][2] * (src).z + (trans).m[2][3] * (src).w; 	\
_d_.w = (trans).m[3][0] * (src).x + (trans).m[3][1] * (src).y + (trans).m[3][2] * (src).z + (trans).m[3][3] * (src).w; 	\
(dst) = _d_;}

#ifdef __cplusplus
}
#endif

#endif /*_TRANSFORM_H_*/
