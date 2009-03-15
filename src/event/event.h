/* 
 * by WN @ Feb 05, 2009
 */

/* Thrown-away event framework */
#ifndef EVENT_H
#define EVENT_H

void event_init(void);
/* return value == 1: exit */
int event_poll(void);

#endif

