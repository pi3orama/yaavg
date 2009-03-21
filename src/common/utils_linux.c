/* 
 * utils_linux.c
 * by WN @ Mar. 15, 2009
 */

#include <config.h>
#include <common/exception.h>
#include <common/debug.h>
#include <common/utils.h>

#include <signal.h>
#include <stdlib.h>

static void ATTR_NORETURN
cleanup(int signum)
{
	FATAL(SYSTEM, "receive signal %d\n", signum);
	fatal_cleanup();
}

void
unblock_sigint(void)
{
#ifdef HAVE_SIGACTION
	struct sigaction action;
	sigaction(SIGINT, NULL, &action);
	action.sa_handler = cleanup;
	sigaction(SIGINT, &action, NULL);
#else
	signal(SIGINT, cleanup);
#endif
}

