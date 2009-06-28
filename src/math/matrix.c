/* 
 * matrix.c
 * by WN @ Jun 20, 2009
 */

#include <common/math.h>
#include <math/matrix.h>
#include <math/trigon.h>
#include <math/sqrt.h>
#include <stdint.h>
#include <memory.h>

#define LBRA	{
#define RBRA	}

#ifdef __SSE__
# include <xmmintrin.h>
#endif

#define UL(x)		((uint32_t)(x))
#define ALIGN(x, n)	(void*)(((UL(x)) + (1UL << (n)) - 1) & (~((1UL << (n)) - 1)))
#define ALIGN16(x)	(void*)ALIGN(x, 4)

static mat4x4 E = {
	.m = {
		{1.0, 0.0, 0.0, 0.0},
		{0.0, 1.0, 0.0, 0.0},
		{0.0, 0.0, 1.0, 0.0},
		{0.0, 0.0, 0.0, 1.0},
	},
};

/* caller must ensure d and s are different */
#define cpy_obj(d, s)	memcpy(d, s, sizeof(*s))

static inline void
real_mulmv(vec4 * d, mat4x4 * m, vec4 * v)
{
#ifdef __SSE__
	if (cpu_cap.have_sse2) {
		__m128 m0, m1, m2, m3, v0, v1, v2, v3;
		m0 = _mm_load_ps(m->m[0]);
		m1 = _mm_load_ps(m->m[1]);
		m2 = _mm_load_ps(m->m[2]);
		m3 = _mm_load_ps(m->m[3]);

		v0 = _mm_load_ps(v->v);
		v1 = _mm_load_ps(v->v);
		v2 = _mm_load_ps(v->v);
		v3 = _mm_load_ps(v->v);

		v0 = _mm_shuffle_ps(v0, v0, 0x00);
		v1 = _mm_shuffle_ps(v1, v1, 0x55);
		v2 = _mm_shuffle_ps(v2, v2, 0xaa);
		v3 = _mm_shuffle_ps(v3, v3, 0xff);

		m0 = _mm_mul_ps(m0, v0);
		m1 = _mm_mul_ps(m1, v1);
		m2 = _mm_mul_ps(m2, v2);
		m3 = _mm_mul_ps(m3, v3);

		m0 = _mm_add_ps(m0, m1);
		m0 = _mm_add_ps(m0, m2);
		m0 = _mm_add_ps(m0, m3);

		_mm_store_ps(d->v, m0);
	} else {
#if 0
	}	/* vim */
#endif
#endif
	vec4 __d;
#define MUL_ONE(n)	\
	__d.v[n] = 		\
		m->m[0][n] * v->v[0] +	\
		m->m[1][n] * v->v[1] +	\
		m->m[2][n] * v->v[2] +	\
		m->m[3][n] * v->v[3]

	MUL_ONE(0);
	MUL_ONE(1);
	MUL_ONE(2);
	MUL_ONE(3);

	cpy_obj(d, &__d);
#undef MUL_ONE


#ifdef __SSE__
	RBRA
#endif
	return;
}

void
mulmv(vec4 * d, mat4x4 * m, vec4 * v)
{
	struct {
		mat4x4 m;
		vec4 v;
		vec4 d;
		uint8_t __padding[16];
	} ATTR(packed) align;

#define ALIGN_ELT(m)	if ((((uintptr_t)(m)) & (0xf)) != 0) {	\
		p##m = ALIGN16(&align.m);	\
		cpy_obj(p##m, m);	\
	} else {	\
		p##m = m;\
	}

	mat4x4 * pm;
	vec4 * pv, * pd;

	ALIGN_ELT(m);
	ALIGN_ELT(v);

	if ((((uintptr_t)(d)) & (0xf)) != 0) {
		pd = ALIGN16(&align.d);
	} else {
		pd = d;
	}

	real_mulmv(pd, pm, pv);

	if (d != pd)
		cpy_obj(d, pd);

	return;
}


static inline void
real_mulmm(mat4x4 * d, mat4x4 * ma, mat4x4 * mb)
{
#ifdef __SSE__
//	if (0) {
	if (cpu_cap.have_sse2) {
		__m128 v00, v01, v02, v03;
		__m128 v10, v11, v12, v13;

#define SSEMUL2VECT(a, b)	\
		v00 = _mm_load_ps(mb->m[a]);	\
		v01 = _mm_load_ps(mb->m[a]);	\
		v02 = _mm_load_ps(mb->m[a]);	\
		v03 = _mm_load_ps(mb->m[a]);	\
		v10 = _mm_load_ps(mb->m[b]);	\
		v11 = _mm_load_ps(mb->m[b]);	\
		v12 = _mm_load_ps(mb->m[b]);	\
		v13 = _mm_load_ps(mb->m[b]);	\
										\
		v00 = _mm_shuffle_ps(v00, v00, 0x00);	\
		v01 = _mm_shuffle_ps(v01, v01, 0x55);	\
		v02 = _mm_shuffle_ps(v02, v02, 0xaa);	\
		v03 = _mm_shuffle_ps(v03, v03, 0xff);	\
												\
		v10 = _mm_shuffle_ps(v10, v10, 0x00);	\
		v11 = _mm_shuffle_ps(v11, v11, 0x55);	\
		v12 = _mm_shuffle_ps(v12, v12, 0xaa);	\
		v13 = _mm_shuffle_ps(v13, v13, 0xff);	\
												\
		v00 = _mm_mul_ps(ma->__m[0], v00);	\
		v01 = _mm_mul_ps(ma->__m[1], v01);	\
		v02 = _mm_mul_ps(ma->__m[2], v02);	\
		v03 = _mm_mul_ps(ma->__m[3], v03);	\
									\
		v10 = _mm_mul_ps(ma->__m[0], v10);	\
		v11 = _mm_mul_ps(ma->__m[1], v11);	\
		v12 = _mm_mul_ps(ma->__m[2], v12);	\
		v13 = _mm_mul_ps(ma->__m[3], v13);	\
									\
		v00 = _mm_add_ps(v00, v01);	\
		v10 = _mm_add_ps(v10, v11);	\
									\
		v00 = _mm_add_ps(v00, v02);	\
		v10 = _mm_add_ps(v10, v12);	\
									\
		v00 = _mm_add_ps(v00, v03);	\
		v10 = _mm_add_ps(v10, v13);	\
									\
		_mm_store_ps(d->m[a], v00);	\
		_mm_store_ps(d->m[b], v10);

		SSEMUL2VECT(0, 1)
		SSEMUL2VECT(2, 3)
#undef SSEMUL2VECT
	} else {
#if 0
	}	/* vim */
#endif
#endif
	mulmv(&(d->v[0]), ma, &(mb->v[0]));
	mulmv(&(d->v[1]), ma, &(mb->v[1]));
	mulmv(&(d->v[2]), ma, &(mb->v[2]));
	mulmv(&(d->v[3]), ma, &(mb->v[3]));
#ifdef __SSE__
	RBRA
#endif
}


void
mulmm(mat4x4 * d, mat4x4 * m1, mat4x4 * m2)
{
	struct {
		mat4x4 m1;
		mat4x4 m2;
		mat4x4 d;
		uint8_t __padding[16];
	} ATTR(packed) align;

	mat4x4 * pm1, *pm2, *pd;

	ALIGN_ELT(m1);
	ALIGN_ELT(m2);

	if (((((uintptr_t)(d)) & (0xf)) != 0) || (d == m1) || (d == m2)) {
		pd = ALIGN16(&align.d);
	} else {
		pd = d;
	}

	real_mulmm(pd, pm1, pm2);

	if (pd != d)
		cpy_obj(d, pd);

	return;
}

void
load_identity(mat4x4 * d)
{
	memcpy(d, &E, sizeof(mat4x4));
	return;
}

void
_matrix_translate(mat4x4 * m, float x, float y, float z)
{
#ifdef __SSE__
	if (cpu_cap.have_sse2) {
		struct {
			mat4x4 m;
			vec4 v;
			uint8_t __padding[16];
		} ATTR(packed) align;

		mat4x4 * pm;
		ALIGN_ELT(m);
		vec4 * pv = ALIGN16(&align.v);
		pv->v[0] = x, pv->v[1] = y;
		pv->v[2] = z, pv->v[3] = 1.0f;

		__m128 v0, v1, v2, v3;
		__m128 m0, m1, m2, m3;

		v0 = _mm_load_ps(pv->v);
		v1 = _mm_load_ps(pv->v);
		v2 = _mm_load_ps(pv->v);
		v3 = _mm_load_ps(pv->v);

		v0 = _mm_shuffle_ps(v0, v0, 0x00);
		v1 = _mm_shuffle_ps(v1, v1, 0x55);
		v2 = _mm_shuffle_ps(v2, v2, 0xaa);
		v3 = _mm_shuffle_ps(v3, v3, 0xff);

		m0 = _mm_mul_ps(pm->__m[0], v0);
		m1 = _mm_mul_ps(pm->__m[1], v1);
		m2 = _mm_mul_ps(pm->__m[2], v2);
		m3 = _mm_load_ps(pm->m[3]);

		m3 = _mm_add_ps(m0, m3);
		m3 = _mm_add_ps(m1, m3);
		m3 = _mm_add_ps(m2, m3);

		_mm_store_ps(pm->m[3], m3);

		if (pm != m)
			cpy_obj(m, pm);
	} else {
#if 0
	}	/* vim */
#endif
#endif
	m->f[12] = m->f[0] * x + m->f[4] * y + m->f[8]  * z + m->f[12];
	m->f[13] = m->f[1] * x + m->f[5] * y + m->f[9]  * z + m->f[13];
	m->f[14] = m->f[2] * x + m->f[6] * y + m->f[10] * z + m->f[14];
	m->f[15] = m->f[3] * x + m->f[7] * y + m->f[11] * z + m->f[15];
#ifdef __SSE__
	RBRA
#endif
}

void
_matrix_rotate(mat4x4 * m, float angle, float x, float y, float z)
{
	mat4x4 R;
	load_identity(&R);
	float s, c;
	s = sin_d(angle);
	c = cos_d(angle);
#define M(row, col)	(R.m[col][row])
	if ((x == 0.0f) && (y == 0.0F) && (z != 0.0f)) {
		M(0, 0) = M(1, 1) = c;
		if (z < 0.0f) {
			M(0, 1) = s;
			M(1, 0) = -s;
		} else {
			M(0, 1) = -s;
			M(1, 0) = s;
		}
	} else if ((x == 0.0f) && (y != 0.0f) && (z == 0.0f)) {
		M(0, 0) = M(2, 2) = c;
		if (y < 0.0f) {
			M(0, 2) = -s;
			M(2, 0) = s;
		} else {
			M(0, 2) = s;
			M(2, 0) = -s;
		}
	} else if ((x != 0.0f) && (y == 0.0f) && (z == 0.0f)) {
		M(1,1) = M(2,2) = c;
		if (x < 0.0F) {
			M(1,2) = s;
			M(2,1) = -s;
		}
		else {
			M(1,2) = -s;
			M(2,1) = s;
		}
	} else {
		/* from MESA code */

		const float mag = invert_sqrt(x * x + y * y + z * z);
		x *= mag;
		y *= mag;
		z *= mag;

		float xx = x * x;
		float yy = y * y;
		float zz = z * z;
		float xy = x * y;
		float yz = y * z;
		float zx = z * x;
		float xs = x * s;
		float ys = y * s;
		float zs = z * s;
		float one_c = 1.0f - c;

		/* We already hold the identity-matrix so we can skip some statements */
		M(0,0) = (one_c * xx) + c;
		M(0,1) = (one_c * xy) - zs;
		M(0,2) = (one_c * zx) + ys;
		/*    M(0,3) = 0.0F; */

		M(1,0) = (one_c * xy) + zs;
		M(1,1) = (one_c * yy) + c;
		M(1,2) = (one_c * yz) - xs;
		/*    M(1,3) = 0.0F; */

		M(2,0) = (one_c * zx) - ys;
		M(2,1) = (one_c * yz) + xs;
		M(2,2) = (one_c * zz) + c;
		/*    M(2,3) = 0.0F; */

		/*
		   M(3,0) = 0.0F;
		   M(3,1) = 0.0F;
		   M(3,2) = 0.0F;
		   M(3,3) = 1.0F;
		   */
	}
#undef M
	mulmm(m, m, &R);
}

void
_matrix_scale(mat4x4 * m, float x, float y, float z)
{
#ifdef __SSE__
	if (cpu_cap.have_sse2) {
		struct {
			mat4x4 m;
			vec4 v;
			uint8_t __padding[16];
		} ATTR(packed) align;

		mat4x4 * pm;
		ALIGN_ELT(m);
		vec4 * pv = ALIGN16(&align.v);
		pv->v[0] = x, pv->v[1] = y, pv->v[2] = z;

		__m128 m0, m1, m2;
		__m128 v0, v1, v2;

		v0 = _mm_load_ps(pv->v);
		v1 = _mm_load_ps(pv->v);
		v2 = _mm_load_ps(pv->v);

		v0 = _mm_shuffle_ps(v0, v0, 0x00);
		v1 = _mm_shuffle_ps(v1, v1, 0x55);
		v2 = _mm_shuffle_ps(v2, v2, 0xaa);

		m0 = _mm_mul_ps(pm->__m[0], v0);
		m1 = _mm_mul_ps(pm->__m[1], v1);
		m2 = _mm_mul_ps(pm->__m[2], v2);

		_mm_store_ps(pm->m[0], m0);
		_mm_store_ps(pm->m[1], m1);
		_mm_store_ps(pm->m[2], m2);

		if (pm != m)
			cpy_obj(m, pm);
	} else {
#if 0
	}	/* vim */
#endif
#endif
	m->f[0] *= x;   m->f[4] *= y;   m->f[8]  *= z;
	m->f[1] *= x;   m->f[5] *= y;   m->f[9]  *= z;
	m->f[2] *= x;   m->f[6] *= y;   m->f[10] *= z;
	m->f[3] *= x;   m->f[7] *= y;   m->f[11] *= z;
#ifdef __SSE__
	RBRA
#endif
}

// vim:ts=4:sw=4

