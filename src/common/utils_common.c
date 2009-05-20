/* 
 * by WN @ May. 20, 2009
 */
#include <stdint.h>

#include <common/defs.h>
#include <common/debug.h>
#include <common/utils.h>

#include <sys/time.h>
#include <time.h>
#include <errno.h>

#ifndef HAVE_SDL

tick_t
get_ticks(void)
{
#ifdef HAVE_CLOCK_GETTIME
	static struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return now.tv_sec * 1000 + now.tv_nsec / 1000000;
#else
	static struct timeval now;
	gettimeofday(&now, NULL);
	return now.tv_sec * 1000 + now.tv_usec / 1000;
#endif /* HAVE_CLOCK_GETTIME */
}

void
delay(tick_t ms)
{
	int was_error = 0;
#if HAVE_NANOSLEEP
	struct timespec elapsed, tv;
#else
	struct timeval tv;
	tick_t then, now;
	/* unsigned value. if timer wrapped, elapsed will become a
	 * very large value. */
	uint32_t elapsed;
#endif

#if HAVE_NANOSLEEP
	elapsed.tv_sec = ms/1000;
	elapsed.tv_nsec = (ms % 1000) * 1000000;
#else
	then = get_ticks();
#endif
	do {
		errno = 0;
#if HAVE_NANOSLEEP
		tv.tv_sec = elapsed.tv_sec;
		tv.tv_nsec = elapsed.tv_nsec;
		was_error = nanosleep(&tv, &elapsed);
#else
		now = get_ticks();
		elapsed = (now - then);
		then = now;
		if (elapsed >= ms)
			break;
		ms -= elapsed;
		tv.tv_sec = ms / 1000;
		tv.tv_usec = (ms % 1000) * 1000;
		was_error = select(0, NULL, NULL, NULL, &tv);
#endif
	} while(was_error && (errno == EINTR));
}

#endif
// vim:tabstop=4:shiftwidth=4

