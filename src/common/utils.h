/* 
 * by WN @ Feb 05, 2009
 */

#ifndef UTILS_H
#define UTILS_H

#include <common/defs.h>
#include <stdint.h>
/* Some utils needed to be implentmented */

/* 
 * delay for some milseconds
 */
void delay(tick_t ms);
tick_t get_ticks(void);

#endif

