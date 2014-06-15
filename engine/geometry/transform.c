

#include <math.h>
#include "transform.h"
#include "vector.h"
#include "error.h"

TRANSFORM IdentityTransform = {
 {{1.	 ,0.	,0.    ,0.},
  {0.	 ,1.	,0.    ,0.},
  {0.	 ,0.	,1.    ,0.},
  {0.	 ,0.	,0.    ,1.}}
};

TRANSFORM Scale(VECTOR s)
{
  TRANSFORM xf = IdentityTransform;
  xf.m[0][0] = s.x;
  xf.m[1][1] = s.y;
  xf.m[2][2] = s.z;
  return xf;
}

TRANSFORM Translate(VECTOR t)
{
  TRANSFORM xf = IdentityTransform;
  xf.m[0][3] = t.x;
  xf.m[1][3] = t.y;
  xf.m[2][3] = t.z;
  return xf;
}

TRANSFORM RotateX(float t)
{
  TRANSFORM xf = IdentityTransform;
  double c = cos(t), s = sin(t);
  SET_3X3MATRIX(xf.m,
		1.,	0.,	0.,
		0.,     c ,    -s  ,
		0.,	s ,	c );
  return xf;
}

TRANSFORM RotateY(float t)
{
  TRANSFORM xf = IdentityTransform;
  double c = cos(t), s = sin(t);
  SET_3X3MATRIX(xf.m,
		c ,	0.,	s ,
		0.,     1.,     0.  ,
	       -s ,	0. ,	c );
  return xf;
}

TRANSFORM RotateZ(float t)
{
  TRANSFORM xf = IdentityTransform;
  double c = cos(t), s = sin(t);
  SET_3X3MATRIX(xf.m,
		c ,    -s ,	0.,
		s ,     c ,     0.,
		0.,	0.,	1.);
  return xf;
}

TRANSFORM Rotate(float alpha, VECTOR d)
{
  TRANSFORM xf = IdentityTransform;
  double x,y,z,c,s,t;

  
  if ((s = VECTORNORM(d)) < EPSILON) {
    Error("Rotate", "Bad rotation axis");
    return xf;
  } else
    
    VECTORSCALEINVERSE(s, d, d); 

  x=d.x, y=d.y, z=d.z, c=cos(alpha), s=sin(alpha), t=1-c;
  SET_3X3MATRIX(xf.m,
		x*x*t + c	, x*y*t - z*s	, x*z*t + y*s	,
		x*y*t + z*s	, y*y*t + c	, y*z*t - x*s	,
		x*z*t - y*s	, y*z*t + x*s	, z*z*t + c	);
  return xf;
}


void RecoverRotation(TRANSFORM xf, float *angle, VECTOR *axis)
{
  double c, s;

  c = (xf.m[0][0] + xf.m[1][1] + xf.m[2][2] - 1.) * 0.5;
  if (c > 1. - EPSILON) {
    *angle = 0.;
    VECTORSET(*axis, 0., 0., 1.);
  } else if (c < -1. + EPSILON) {
    *angle = M_PI;
    axis->x = sqrt((xf.m[0][0] + 1.) * 0.5);
    axis->y = sqrt((xf.m[1][1] + 1.) * 0.5);
    axis->z = sqrt((xf.m[2][2] + 1.) * 0.5);

    
    if (xf.m[1][0] < 0.) axis->y = -axis->y;
    if (xf.m[2][0] < 0.) axis->z = -axis->z;
  } else {
    double r;
    *angle = acos(c);
    s = sqrt(1. - c*c);
    r = 1. / (2. * s);
    axis->x = (double)(xf.m[2][1] - xf.m[1][2]) * r;
    axis->y = (double)(xf.m[0][2] - xf.m[2][0]) * r;
    axis->z = (double)(xf.m[1][0] - xf.m[0][1]) * r;
  }
}


TRANSFORM TransCompose(TRANSFORM xf2, TRANSFORM xf1)
{
  TRANSFORM xf;

  xf.m[0][0] = xf2.m[0][0] * xf1.m[0][0] + xf2.m[0][1] * xf1.m[1][0] + xf2.m[0][2] * xf1.m[2][0] + xf2.m[0][3] * xf1.m[3][0];
  xf.m[0][1] = xf2.m[0][0] * xf1.m[0][1] + xf2.m[0][1] * xf1.m[1][1] + xf2.m[0][2] * xf1.m[2][1] + xf2.m[0][3] * xf1.m[3][1];
  xf.m[0][2] = xf2.m[0][0] * xf1.m[0][2] + xf2.m[0][1] * xf1.m[1][2] + xf2.m[0][2] * xf1.m[2][2] + xf2.m[0][3] * xf1.m[3][2];
  xf.m[0][3] = xf2.m[0][0] * xf1.m[0][3] + xf2.m[0][1] * xf1.m[1][3] + xf2.m[0][2] * xf1.m[2][3] + xf2.m[0][3] * xf1.m[3][3];

  xf.m[1][0] = xf2.m[1][0] * xf1.m[0][0] + xf2.m[1][1] * xf1.m[1][0] + xf2.m[1][2] * xf1.m[2][0] + xf2.m[1][3] * xf1.m[3][0];
  xf.m[1][1] = xf2.m[1][0] * xf1.m[0][1] + xf2.m[1][1] * xf1.m[1][1] + xf2.m[1][2] * xf1.m[2][1] + xf2.m[1][3] * xf1.m[3][1];
  xf.m[1][2] = xf2.m[1][0] * xf1.m[0][2] + xf2.m[1][1] * xf1.m[1][2] + xf2.m[1][2] * xf1.m[2][2] + xf2.m[1][3] * xf1.m[3][2];
  xf.m[1][3] = xf2.m[1][0] * xf1.m[0][3] + xf2.m[1][1] * xf1.m[1][3] + xf2.m[1][2] * xf1.m[2][3] + xf2.m[1][3] * xf1.m[3][3];

  xf.m[2][0] = xf2.m[2][0] * xf1.m[0][0] + xf2.m[2][1] * xf1.m[1][0] + xf2.m[2][2] * xf1.m[2][0] + xf2.m[2][3] * xf1.m[3][0];
  xf.m[2][1] = xf2.m[2][0] * xf1.m[0][1] + xf2.m[2][1] * xf1.m[1][1] + xf2.m[2][2] * xf1.m[2][1] + xf2.m[2][3] * xf1.m[3][1];
  xf.m[2][2] = xf2.m[2][0] * xf1.m[0][2] + xf2.m[2][1] * xf1.m[1][2] + xf2.m[2][2] * xf1.m[2][2] + xf2.m[2][3] * xf1.m[3][2];
  xf.m[2][3] = xf2.m[2][0] * xf1.m[0][3] + xf2.m[2][1] * xf1.m[1][3] + xf2.m[2][2] * xf1.m[2][3] + xf2.m[2][3] * xf1.m[3][3];

  xf.m[3][0] = xf2.m[3][0] * xf1.m[0][0] + xf2.m[3][1] * xf1.m[1][0] + xf2.m[3][2] * xf1.m[2][0] + xf2.m[3][3] * xf1.m[3][0];
  xf.m[3][1] = xf2.m[3][0] * xf1.m[0][1] + xf2.m[3][1] * xf1.m[1][1] + xf2.m[3][2] * xf1.m[2][1] + xf2.m[3][3] * xf1.m[3][1];
  xf.m[3][2] = xf2.m[3][0] * xf1.m[0][2] + xf2.m[3][1] * xf1.m[1][2] + xf2.m[3][2] * xf1.m[2][2] + xf2.m[3][3] * xf1.m[3][2];
  xf.m[3][3] = xf2.m[3][0] * xf1.m[0][3] + xf2.m[3][1] * xf1.m[1][3] + xf2.m[3][2] * xf1.m[2][3] + xf2.m[3][3] * xf1.m[3][3];

  return xf;
}


TRANSFORM LookAt(VECTOR eye, VECTOR centre, VECTOR up)
{
  TRANSFORM xf = IdentityTransform;
  VECTOR s, X, Y, Z;

  VECTORSUBTRACT(eye, centre, Z);	
  VECTORNORMALIZE(Z);

  VECTORCROSSPRODUCT(up, Z, X);		
  VECTORNORMALIZE(X);

  VECTORCROSSPRODUCT(Z, X, Y);		
  SET_3X3MATRIX(xf.m,			
	       X.x, X.y, X.z,
	       Y.x, Y.y, Y.z,
	       Z.x, Z.y, Z.z);

  VECTORSCALE(-1., eye, s);		
  return TransCompose(xf, Translate(s));
}

TRANSFORM Perspective(float fov , float aspect, float near, float far)
{
  TRANSFORM xf = IdentityTransform;
  double f = 1./tan(fov/2.);
  
  xf.m[0][0] = f/aspect;
  xf.m[1][1] = f;
  xf.m[2][2] = (near+far) / (near-far);
  xf.m[2][3] = (2*far*near) / (near-far);
  xf.m[3][2] = -1.; 
  xf.m[3][3] = 0.;

  return xf;
}

extern TRANSFORM Ortho(float left, float right, float bottom, float top, float near, float far)
{
  TRANSFORM xf = IdentityTransform;

  xf.m[0][0] = 2./(right-left);
  xf.m[0][3] = - (right+left) / (right-left);

  xf.m[1][1] = 2./(top-bottom);
  xf.m[1][3] = - (top+bottom) / (top-bottom);
  
  xf.m[2][2] = -2. / (far-near);
  xf.m[2][3] = - (far+near) / (far-near);

  return xf;
}

void PrintTransform(FILE *fp, TRANSFORM xf)
{
  int i, j;
  for (i=0; i<4; i++) {
    for (j=0; j<4; j++) 
      fprintf(fp, "%f ", xf.m[i][j]);
    fprintf(fp, "\n");
  }
}

#ifdef TEST
void TestTransform(TRANSFORM xf)
{
  POINT4D v;
  printf("Enter vector to transform:\n");
  scanf("%f %f %f", &v.x, &v.y, &v.z);
  v.w = 1.;
  VectorPrint(stdout, v); printf(" ----> ");
  TRANSFORM_POINT_4D(xf, v, v);
  if (v.w < -EPSILON || v.w > EPSILON) {
    v.x /= v.w; v.y /= v.w; v.z /= v.w;
  }
  VectorPrint(stdout, v); printf(", w = %g\n", v.w);
}

TRANSFORM MakeTransform(void)
{
  int c;
  VECTOR v; float t;
  TRANSFORM xf;

  printf("Make what transform?\n");
  printf("1) scale\n");
  printf("2) translate\n");
  printf("3) rotate X\n");
  printf("4) rotate Y\n");
  printf("5) rotate Z\n");
  printf("6) rotate arbitrary axis\n");
  printf("7) lookat\n");
  printf("8) perspective\n");
  printf("9) ortho\n");
  scanf("%d", &c);
  switch (c) {
  case 1:
    printf("Scaling vector?\n");
    scanf("%f %f %f", &v.x, &v.y, &v.z);
    xf = Scale(v);
    break;
  case 2:
    printf("Translation vector?\n");
    scanf("%f %f %f", &v.x, &v.y, &v.z);
    xf = Translate(v);
    break;
  case 3:
    printf("angle?\n");
    scanf("%f", &t);
    t *= M_PI / 180.;
    xf = RotateX(t);
    break;
  case 4:
    printf("angle?\n");
    scanf("%f", &t);
    t *= M_PI / 180.;
    xf = RotateY(t);
    break;
  case 5:
    printf("angle?\n");
    scanf("%f", &t);
    t *= M_PI / 180.;
    xf = RotateZ(t);
    break;
  case 6:
    printf("angle?\n");
    scanf("%f", &t);
    t *= M_PI / 180.;
    printf("Rotation axis?\n");
    scanf("%f %f %f", &v.x, &v.y, &v.z);
    xf = Rotate(t, v);
    RecoverRotation(xf, &t, &v);
    printf("Testing rotation axis and angle recovery: axis = %g %g %g, angle = %g\n",
	   v.x, v.y, v.z, t*180./M_PI);
    break;
  case 7:
    { VECTOR e, c, u;
    printf("Eye?\n");
    scanf("%f %f %f", &e.x, &e.y, &e.z);
    printf("Centre?\n");
    scanf("%f %f %f", &c.x, &c.y, &c.z);
    printf("Up?\n");
    scanf("%f %f %f", &u.x, &u.y, &u.z);
    xf = LookAt(e, c, u);
    }
    break;
  case 8:
    { double fov, aspect, near, far;
    printf("fov, aspect, near, far?\n");
    scanf("%lf %lf %lf %lf", &fov, &aspect, &near, &far);
    xf = Perspective(fov / 180. * M_PI, aspect, near, far);
    }
    break;
  case 9:
    { double left, right, bottom, top, near, far;
    printf("left, right, bottom, top, near, far?\n");
    scanf("%lf %lf %lf %lf %lf %lf", &left, &right, &bottom, &top, &near, &far);
    xf = Ortho(left, right, bottom, top, near, far);
    }
    break;
  default:
    printf("Invalid choice %d.\n", c);
    return IdentityTransform;
  }

  PrintTransform(stdout, xf);
  return xf;
}

int main(int argc, char **argv)
{
  TRANSFORM xf;

  xf = MakeTransform();
  while (1) {
    VECTOR v; float t;
    TRANSFORM xf2;
    int c;

    printf("Combine with second transform?\n");	
    printf("1) yes\n");
    printf("2) no\n");
    scanf("%d", &c);
    if (c!=1) 
      break;
    xf2 = MakeTransform();
    xf = TransCompose(xf2, xf);
    PrintTransform(stdout, xf);

    RecoverRotation(xf, &t, &v);
    printf("Testing combined rotation axis and angle recovery: axis = %g %g %g, angle = %g\n",
	   v.x, v.y, v.z, t*180./M_PI);
  }

  while (1)
    TestTransform(xf);

  return 0;
}

#endif
