/* 
 * vector.h
 * definition of vect4
 * by WN @ Jun 20, 2009
 */
#ifndef __YAAVG_MATH_H
#define __YAAVG_MATH_H
#include <common/defs.h>
#include <common/debug.h>

#if defined(_M_IX86) || defined(i386) || defined(__i386) || defined(__i386__)
# ifndef __INTEL__
#  define __INTEL__
# endif
# ifndef __X86_32__
#  define __X86_32__
# endif
#endif /* x86 */


struct cpu_cap {
	bool_t have_sse2;
};

extern struct cpu_cap cpu_cap;

#define SSE2_ENABLE	(cpu_cap.have_sse2)

extern void
math_init(void);

#endif

