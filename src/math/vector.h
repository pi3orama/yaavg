/* 
 * vector.h
 * definition of vect4
 * by WN @ Jun 20, 2009
 */
#ifndef __VECTOR_H
#define __VECTOR_H
#include <common/defs.h>

__BEGIN_DECLS
#ifdef __SSE__
# include <xmmintrin.h>
#else
typedef float __m128 __attribute__ ((__vector_size__ (16), __may_alias__));
#endif

typedef union __vec4 {
	__m128 __m;
	float v[4];
} vec4;

__END_DECLS

#endif
// vim:ts=4:sw=4

