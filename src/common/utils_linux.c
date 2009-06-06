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

bool_t sigpipe_arised = FALSE;

static void ATTR_NORETURN
cleanup(int signum)
{
	FATAL(SYSTEM, "receive signal %d\n", signum);
	switch (signum) {
		case SIGPIPE:
			sigpipe_arised = TRUE;
			break;
		default:
			break;
	}
	fatal_cleanup();
}

void
intercept_signal(int signum)
{
#ifdef HAVE_SIGACTION
	struct sigaction action;
	sigaction(signum, NULL, &action);
	action.sa_handler = cleanup;
	sigaction(signum, &action, NULL);
#else
	signal(signum, cleanup);
#endif
}

