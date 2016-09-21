#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>
#include <iostream>

typedef float Matrix[16];
typedef float Vector4[4];
typedef float Vector3[3];

#define A(row,col)  a[(col<<2)+row]
#define B(row,col)  b[(col<<2)+row]
#define P(row,col)  product[(col<<2)+row]

inline	void
add_v3_v3(Vector3 result,const Vector3 s1,const Vector3 s2) {
	result[0]	=	s1[0] + s2[0];
	result[1]	=	s1[1] + s2[1];
	result[2]	=	s1[2] + s2[2];
}


inline void
initV3(float a, float b, float c, Vector3 v)
{
	v[0] = a;
	v[1] = b;
	v[2] = c;
}

inline void
init0V3(Vector3 v)
{
	v[0] = 0.;
	v[1] = 0.;
	v[2] = 0.;
}

inline void
initV3(const Vector3 i, Vector3 v)
{
	v[0] = i[0];
	v[1] = i[1];
	v[2] = i[2];
}

inline void
initV4(float a, float b, float c, float d, Vector4 v)
{
	v[0] = a;
	v[1] = b;
	v[2] = c;
	v[3] = d;
}

inline	void
cross_v3_v3(Vector3 result,const Vector3 s1,const Vector3 s2) {
	result[0]	=	(s1[1]*s2[2] - s1[2]*s2[1]);
	result[1]	=	(s1[2]*s2[0] - s1[0]*s2[2]);
	result[2]	=	(s1[0]*s2[1] - s1[1]*s2[0]);
}

inline float
dot_v3_v3(const Vector3 s1,const Vector3 s2) {
	return (s1[0]*s2[0] + s1[1]*s2[1] + s1[2]*s2[2]);
}

inline	float
length_v3(const Vector3 v) {
	return sqrtf(dot_v3_v3(v,v));
}

inline void
mul_v3_v3(Vector3 result,const Vector3 s1) {
	result[0]	*=	s1[0];
	result[1]	*=	s1[1];
	result[2]	*=	s1[2];
}

inline void
sub_v3_v3(Vector3 result,const Vector3 s1) {
	result[0]	-=	s1[0];
	result[1]	-=	s1[1];
	result[2]	-=	s1[2];
}

inline void
sub_v3_v3(Vector3 result,const Vector3 s1,const Vector3 s2) {
	result[0]	=	s1[0] - s2[0];
	result[1]	=	s1[1] - s2[1];
	result[2]	=	s1[2] - s2[2];
}

inline void
normalize_v3(Vector3 r,const Vector3 v) {
	const double	l	=	1 / sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

	r[0]	=	 (v[0]*l);
	r[1]	=	 (v[1]*l);
	r[2]	=	 (v[2]*l);
}

inline void
normalize_v3(Vector3 v) {
	const double	l	=	1 / sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

	v[0]	=	 (v[0]*l);
	v[1]	=	 (v[1]*l);
	v[2]	=	 (v[2]*l);
}

inline void
matrix4_identity(Matrix matrix)
{
	matrix[0] = 1.0f;
	matrix[4] = 0.0f;
	matrix[8] = 0.0f;
	matrix[12] =0.0f;

	matrix[1] = 0.0f;
	matrix[5] = 1.0f;
	matrix[9] = 0.0f;
	matrix[13] = 0.0f;

	matrix[2] = 0.0f;
	matrix[6] = 0.0f;
	matrix[10] = 1.0f;
    matrix[14] = 0.0f;

    matrix[3] = 0.0f;
    matrix[7] = 0.0f;
    matrix[11] = 0.0f;
    matrix[15] = 1.0f;
}

inline void
matrix4_translate(Matrix matrix, float x, float y, float z = 0.)
{
	matrix[0] = 1.0f;
	matrix[4] = 0.0f;
	matrix[8] = 0.0f;
	matrix[12] =x;

	matrix[1] = 0.0f;
	matrix[5] = 1.0f;
	matrix[9] = 0.0f;
	matrix[13] = y;

	matrix[2] = 0.0f;
	matrix[6] = 0.0f;
	matrix[10] = 1.0f;
    matrix[14] = z;

    matrix[3] = 0.0f;
    matrix[7] = 0.0f;
    matrix[11] = 0.0f;
    matrix[15] = 1.0f;
}

inline void
matrix4_mult( Matrix product, const Matrix a, const Matrix b )
{
   int i;
   for (i = 0; i < 4; i++) {
      const float ai0=A(i,0),  ai1=A(i,1),  ai2=A(i,2),  ai3=A(i,3);
      P(i,0) = ai0 * B(0,0) + ai1 * B(1,0) + ai2 * B(2,0) + ai3 * B(3,0);
      P(i,1) = ai0 * B(0,1) + ai1 * B(1,1) + ai2 * B(2,1) + ai3 * B(3,1);
      P(i,2) = ai0 * B(0,2) + ai1 * B(1,2) + ai2 * B(2,2) + ai3 * B(3,2);
      P(i,3) = ai0 * B(0,3) + ai1 * B(1,3) + ai2 * B(2,3) + ai3 * B(3,3);
   }
}

inline bool
matrix4_inverse(const Matrix m, Matrix invOut)
{
    Matrix inv;
	float det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] -
             m[5]  * m[11] * m[14] -
             m[9]  * m[6]  * m[15] +
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] -
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] +
              m[4]  * m[11] * m[14] +
              m[8]  * m[6]  * m[15] -
              m[8]  * m[7]  * m[14] -
              m[12] * m[6]  * m[11] +
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] -
             m[4]  * m[11] * m[13] -
             m[8]  * m[5] * m[15] +
             m[8]  * m[7] * m[13] +
             m[12] * m[5] * m[11] -
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] +
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] -
               m[8]  * m[6] * m[13] -
               m[12] * m[5] * m[10] +
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] +
              m[1]  * m[11] * m[14] +
              m[9]  * m[2] * m[15] -
              m[9]  * m[3] * m[14] -
              m[13] * m[2] * m[11] +
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] -
             m[0]  * m[11] * m[14] -
             m[8]  * m[2] * m[15] +
             m[8]  * m[3] * m[14] +
             m[12] * m[2] * m[11] -
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] +
              m[0]  * m[11] * m[13] +
              m[8]  * m[1] * m[15] -
              m[8]  * m[3] * m[13] -
              m[12] * m[1] * m[11] +
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] -
              m[0]  * m[10] * m[13] -
              m[8]  * m[1] * m[14] +
              m[8]  * m[2] * m[13] +
              m[12] * m[1] * m[10] -
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] -
             m[1]  * m[7] * m[14] -
             m[5]  * m[2] * m[15] +
             m[5]  * m[3] * m[14] +
             m[13] * m[2] * m[7] -
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] +
              m[0]  * m[7] * m[14] +
              m[4]  * m[2] * m[15] -
              m[4]  * m[3] * m[14] -
              m[12] * m[2] * m[7] +
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] -
              m[0]  * m[7] * m[13] -
              m[4]  * m[1] * m[15] +
              m[4]  * m[3] * m[13] +
              m[12] * m[1] * m[7] -
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] +
               m[0]  * m[6] * m[13] +
               m[4]  * m[1] * m[14] -
               m[4]  * m[2] * m[13] -
               m[12] * m[1] * m[6] +
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] +
              m[1] * m[7] * m[10] +
              m[5] * m[2] * m[11] -
              m[5] * m[3] * m[10] -
              m[9] * m[2] * m[7] +
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] -
             m[0] * m[7] * m[10] -
             m[4] * m[2] * m[11] +
             m[4] * m[3] * m[10] +
             m[8] * m[2] * m[7] -
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] +
               m[0] * m[7] * m[9] +
               m[4] * m[1] * m[11] -
               m[4] * m[3] * m[9] -
               m[8] * m[1] * m[7] +
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] -
              m[0] * m[6] * m[9] -
              m[4] * m[1] * m[10] +
              m[4] * m[2] * m[9] +
              m[8] * m[1] * m[6] -
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
}

#define	element(row, column)	(column<<2)+row

inline void
matrix4_mult_v3(Vector3 result,const Matrix mtx,const Vector3 vec) {
	const	float	x		=	mtx[element(0,0)]*vec[0]+mtx[element(0,1)]*vec[1]+mtx[element(0,2)]*vec[2]+mtx[element(0,3)];
	const	float	y		=	mtx[element(1,0)]*vec[0]+mtx[element(1,1)]*vec[1]+mtx[element(1,2)]*vec[2]+mtx[element(1,3)];
	const	float	z		=	mtx[element(2,0)]*vec[0]+mtx[element(2,1)]*vec[1]+mtx[element(2,2)]*vec[2]+mtx[element(2,3)];
	const	float	w		= 	mtx[element(3,0)]*vec[0]+mtx[element(3,1)]*vec[1]+mtx[element(3,2)]*vec[2]+mtx[element(3,3)];

	if (w != 1) {
		const double	l	=	1 / (double) w;

		result[0]	=	(x*l);
		result[1]	=	(y*l);
		result[2]	=	(z*l);
	} else {
		result[0]	=	x;
		result[1]	=	y;
		result[2]	=	z;
	}
}

inline void
matrix4_rotate(Matrix r,float x,float y,float z,const float angle) {
	double	s	=	cos(angle/2);
	double	sp	=	sin(angle/2);
	double	l	=	sqrt(x*x + y*y + z*z);

	// If this is a zero rotation, set the identity matrix
	if ((l == 0) || (angle==0)) matrix4_identity(r);
	else {
		double	a	=	x*sp/l;
		double	b	=	y*sp/l;
		double	c	=	z*sp/l;
		l			=	sqrt(a*a + b*b + c*c + s*s);
		a			/=	l;
		b			/=	l;
		c			/=	l;
		s			/=	l;

		r[element(0,0)] = (float) (1-2*b*b-2*c*c);
		r[element(0,1)] = (float) (2*a*b-2*s*c);
		r[element(0,2)] = (float) (2*a*c+2*s*b);
		r[element(0,3)] = 0;
		r[element(1,0)] = (float) (2*a*b+2*s*c);
		r[element(1,1)] = (float) (1-2*a*a-2*c*c);
		r[element(1,2)] = (float) (2*b*c-2*s*a);
		r[element(1,3)] = 0;
		r[element(2,0)] = (float) (2*a*c-2*s*b);
		r[element(2,1)] = (float) (2*b*c+2*s*a);
		r[element(2,2)] = (float) (1-2*a*a-2*b*b);
		r[element(2,3)] = 0;
		r[element(3,0)] = 0;
		r[element(3,1)] = 0;
		r[element(3,2)] = 0;
		r[element(3,3)] = 1;
	}
}

inline void
matrix4_scale(Matrix r,const float sx,const float sy,const float sz) {
	matrix4_identity(r);

	r[element(0,0)]	=	sx;
	r[element(1,1)]	=	sy;
	r[element(2,2)]	=	sz;
}

inline void
matrix4_mult_v4(Vector4 result,const Matrix s1,const Vector4 s2) {
	const	float	x		=	s1[element(0,0)]*s2[0]+s1[element(0,1)]*s2[1]+s1[element(0,2)]*s2[2]+s1[element(0,3)]*s2[3];
	const	float	y		=	s1[element(1,0)]*s2[0]+s1[element(1,1)]*s2[1]+s1[element(1,2)]*s2[2]+s1[element(1,3)]*s2[3];
	const	float	z		=	s1[element(2,0)]*s2[0]+s1[element(2,1)]*s2[1]+s1[element(2,2)]*s2[2]+s1[element(2,3)]*s2[3];
	const	float	w		= 	s1[element(3,0)]*s2[0]+s1[element(3,1)]*s2[1]+s1[element(3,2)]*s2[2]+s1[element(3,3)]*s2[3];

	result[0]	=	x;
	result[1]	=	y;
	result[2]	=	z;
	result[3]	=	w;
}

#undef element

std::ostream& operator << (std::ostream & out, Matrix m);

#endif
