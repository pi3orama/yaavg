/* 
 * matrix.h
 * by WN @ Jun 20, 2009
 */
#ifndef __MATRIX_H
#define __MATRIX_H
#include <common/defs.h>
#ifdef __SSE__
# include <xmmintrin.h>
#endif

#include <math/vector.h>
#include <math/math.h> /* for cpu_cap */
#include <common/debug.h>

__BEGIN_DECLS

typedef union __mat4x4 {
	__m128 __m[4];
	vec4 v[4];
	float m[4][4];
	float f[16];
} mat4x4 ATTR(aligned (128));

extern void
mulmv(vec4 * d, mat4x4 * m, vec4 * v);

extern void
mulmm(mat4x4 * d, mat4x4 * m1, mat4x4 * m2);

extern void
load_identity(mat4x4 * d);

extern void
invert_matrix(mat4x4 * d, mat4x4 * s);

extern void
_matrix_translate(mat4x4 * m, float x, float y, float z);

extern void
_matrix_rotate(mat4x4 * m, float angle, float x, float y, float z);

extern void
_matrix_scale(mat4x4 * m, float x, float y, float z);

static inline void
print_vector(vec4 * v)
{
	FORCE(SYSTEM, "%f %f %f %f\n",
			v->v[0],
			v->v[1],
			v->v[2],
			v->v[3]
	     );
}


static inline void
print_matrix(mat4x4 * m)
{
	FORCE(SYSTEM, "%f %f %f %f\n",
		m->m[0][0],
		m->m[1][0],
		m->m[2][0],
		m->m[3][0]
		);
	FORCE(SYSTEM, "%f %f %f %f\n",
		m->m[0][1],
		m->m[1][1],
		m->m[2][1],
		m->m[3][1]
		);
	FORCE(SYSTEM, "%f %f %f %f\n",
		m->m[0][2],
		m->m[1][2],
		m->m[2][2],
		m->m[3][2]
		);
	FORCE(SYSTEM, "%f %f %f %f\n",
		m->m[0][3],
		m->m[1][3],
		m->m[2][3],
		m->m[3][3]
		);
}

__END_DECLS



#endif

