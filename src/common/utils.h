/* 
 * by WN @ Feb 05, 2009
 */

#ifndef UTILS_H
#define UTILS_H

#include <common/defs.h>
#include <common/exception.h>
#include <stdint.h>
/* Some utils needed to be implentmented */

/* 
 * delay for some milseconds
 */
void delay(tick_t ms);
tick_t get_ticks(void);

/* for posix system, unlock SIGINT with this func */
void unblock_sigint(void);

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

/* 
 * write `buffer' to `filename'. w/h is the size of buffer. 
 */
void
write_to_pngfile_rgb(char * filename, uint8_t * buffer, int w, int h)
	THROWS(CONTINUE, FATAL);

/* 
 * same as write_to_pngfile_rgb, except buffer format is rgba
 */
void
write_to_pngfile_rgba(char * filename, uint8_t * buffer, int w, int h)
	THROWS(CONTINUE, FATAL);

#endif

