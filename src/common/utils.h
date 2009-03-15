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

