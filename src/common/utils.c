/* utils.c
 * by WN @ Mar. 16, 2009 */

#include <common/debug.h>
#include <common/exception.h>
#include <common/utils.h>


void throw_reinit_exception(
		int * reinited,
		tick_t * last_time,
		int rerun_exp_level, const char * rerun_msg,
		int skip_exp_level, const char * skip_msg,
		int reinit_exp_level, const char * reinit_msg,
		int fatal_exp_level, const char * fatal_msg)
{
	tick_t current = get_ticks();

	if ((*reinited == 0) || (current - *last_time >= 5000)) {
		*last_time = current;
		*reinited = 1;
		THROW(rerun_exp_level, rerun_msg);
	} else {
		*last_time = current;
		switch ((*reinited)++) {
			case 1:
				THROW(skip_exp_level, skip_msg);
			case 2:
				THROW(reinit_exp_level, reinit_msg);
			case 3:
				THROW(fatal_exp_level, fatal_msg);
			default:
				ERROR(SYSTEM, "!@#$%\n");
				THROW(EXCEPTION_FATAL, "Unrecoverable error");
		}
	}
}

