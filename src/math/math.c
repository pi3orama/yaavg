/* 
 * math.c
 * by WN @ Jun 20, 2009
 */

#include <common/debug.h>
#include <math/math.h>
#include <math/trigon.h>
#include <common/math.h>
#include <stdint.h>
#include <signal.h>
#ifdef __INTEL__
/* some very old gcc doesn't has cpuid.h */
# include <common/cpuid.h>
# ifdef __SSE__
#  include <xmmintrin.h>
# endif
#endif

struct cpu_cap cpu_cap = {
	FALSE,
};

#ifdef __INTEL__
static bool_t sigill_happened = FALSE;
static void ATTR(unused)
sigill_handler_sse(int signal, struct sigcontext sc)
{
	WARNING(SYSTEM, "SSE instructor cause SIGILL\n");
	/* Both the "xorps %%xmm0,%%xmm0" and "divps %xmm0,%%xmm1"
	 * instructions are 3 bytes long.  We must increment the instruction
	 * pointer manually to avoid repeated execution of the offending
	 * instruction.
	 *
	 * If the SIGILL is caused by a divide-by-zero when unmasked
	 * exceptions aren't supported, the SIMD FPU status and control
	 * word will be restored at the end of the test, so we don't need
	 * to worry about doing it here.  Besides, we may not be able to...
	 */
	sc.eip += 3;
	sigill_happened = TRUE;
}

/* code copy from mplayer, it copy from libavcodec */
static bool_t ATTR(unused)
has_cpuid(void)
{
	long a, c;

	// code from libavcodec:
	__asm__ __volatile__ (
			/* See if CPUID instruction is supported ... */
			/* ... Get copies of EFLAGS into eax and ecx */
			"pushf\n\t"
			"pop %0\n\t"
			"mov %0, %1\n\t"

			/* ... Toggle the ID bit in one copy and store */
			/*     to the EFLAGS reg */
			"xor $0x200000, %0\n\t"
			"push %0\n\t"
			"popf\n\t"

			/* ... Get the (hopefully modified) EFLAGS */
			"pushf\n\t"
			"pop %0\n\t"
			: "=a" (a), "=c" (c)
			:
			: "cc" 
			);

	return (a!=c);
}

static inline void
do_cpuid(uint32_t level, uint32_t regs[4])
{
	__get_cpuid(level, regs, regs+1, regs+2, regs+3);
}
#endif

static void
cpu_detect(void)
{
#ifndef __INTEL__
	TRACE(SYSTEM, "Not intel specific compilation\n");
	return;
#else
# ifndef __SSE__
	TRACE(SYSTEM, "This compilation doesn't support SSE\n");
	return;
# else
	if (!has_cpuid())
		return;

	TRACE(SYSTEM, "System support CPUID\n");

	uint32_t regs[4];

	do_cpuid(0x00000000, regs);
	TRACE(SYSTEM, "CPU vendor name: %.4s%.4s%.4s; max cpu level: %d\n",
			(char*)(regs+1), (char*)(regs+3), (char*)(regs+2), regs[0]);
	if (regs[0] < 0x00000001)
		return;

	do_cpuid(0x00000001, regs);
	if (regs[3] & bit_SSE2) {
		TRACE(SYSTEM, "CPU support SSE2\n");
		/* cpu has SSE2, check OS support */

		struct sigaction saved_sigill;

		/* Save the original signal handlers.
		*/
		sigaction(SIGILL, NULL, &saved_sigill);
		signal(SIGILL, (void (*)(int))sigill_handler_sse);

		/* Emulate test for OSFXSR in CR4.  The OS will set this bit if it
		 * supports the extended FPU save and restore required for SSE.  If
		 * we execute an SSE instruction on a PIII and get a SIGILL, the OS
		 * doesn't support Streaming SIMD Exceptions, even if the processor
		 * does.
		 */
		TRACE(SYSTEM, "Testing OS support for SSE...\n");

		asm volatile ("xorps %xmm0, %xmm0");

		sigaction(SIGILL, &saved_sigill, NULL);
		if (sigill_happened) {
			WARNING(SYSTEM, "OS doesn't support SSE\n" );
			return;
		}
		cpu_cap.have_sse2 = TRUE;
		TRACE(SYSTEM, "OS support SSE\n" );
	}
# endif
#endif
}

void
math_init(void)
{
	cpu_detect();
	/* init sin table and tan table */
	for (int i = 0; i < TRIGON_TABLE_SIZE; i++) {
		/* although libc's manual says that tan generate overflow
		 * when X near its singularities,
		 * it is not true in my machine. */
		float v = 2.0f * M_PI / (float)(TRIGON_TABLE_SIZE) * (float)(i);
		sin_table[i] = sinf(v);
		tan_table[i] = tanf(v);
	}
}

// vim:ts=4:sw=4

