/* 
 * by WN @ Feb 05, 2009
 */

#ifndef UTILS_H
#define UTILS_H

#include <common/defs.h>
#include <common/exception.h>
#include <signal.h>
#include <stdint.h>

/* Some utils needed to be implentmented */
__BEGIN_DECLS
/* 
 * delay for some milseconds
 */
void delay(tick_t ms);
tick_t get_ticks(void);

/* specially used for GLX */
extern bool_t sigpipe_arised;

/* for posix system, unlock SIGINT with this func */
void intercept_signal(int signum);

/* solve multi reinit problem */
/* I put this func here, not in exception.h, because
 * it depend on tick */
void NORETURN throw_reinit_exception(
		int * reinited,
		tick_t * last_time,
		int rerun_exp_level, const char * rerun_msg,
		int skip_exp_level, const char * skip_msg,
		int reinit_exp_level, const char * reinit_msg,
		int fatal_exp_level, const char * fatal_msg) ATTR_NORETURN ;

static inline int
count_1s(uint32_t c1)
{
	register uint32_t c2 = (c1 >> 1) & 033333333333;
	c2 = c1 - c2 - ((c2 >> 1) & 033333333333);
	return (((c2 + (c2 >> 3)) & 030707070707) % 077);
}

static inline int
count_1s_64(uint64_t c1)
{
	uint64_t c2 = (c1 >> 1) & 0x7777777777777777ULL;
	uint64_t c3 = (c1 >> 2) & 0x3333333333333333ULL;
	uint64_t c4 = (c1 >> 3) & 0x1111111111111111ULL;
	c1 = c1 - c2 - c3 - c4;
	return ((c1 + (c1 >> 4)) & 0x0F0F0F0F0F0F0F0FULL) % 0xFF;
}

#define is_power_of_two(x)	(count_1s(x) == 1)
#define is_power_of_two64(x)	(count_1s64(x) == 1)

#ifndef max
# define max(a, b)	(((a) > (b)) ? (a) : (b))
#endif

#ifndef min
# define min(a, b)	(((a) < (b)) ? (a) : (b))
#endif

#ifndef diff
# define diff(a, b)	(max((a), (b)) - min((a), (b)))
#endif

static inline int
pow2roundup (int x)
{
    if (x < 0)
        return 0;
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x+1;
}

static inline int
pow2rounddown (int x)
{
	return pow2roundup(x) >> 1;
}

bool_t
match_word(const char * word, const char * string);

__END_DECLS

#endif
// vim:tabstop=4:shiftwidth=4
