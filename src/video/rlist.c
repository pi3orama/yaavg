/* 
 * rlist.c
 * by WN @ Mar. 8, 2009
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <video/rlist.h>
#include <video/rcommand.h>
#include <common/debug.h>


/* 
 * init_rlist - initialize a render list.
 *
 * Program always has only 1 render list, therefore I don't define a
 * alloc_rlist.
 */
void
rlist_init(struct render_list * rlist)
{
	INIT_LIST_HEAD(&rlist->command_list_head);
}

/* 
 * remove each render command
 */
void
rlist_clear(struct render_list * rlist)
{
	struct render_command * pos, *n;
	rlist_for_each_rcmd_safe(pos, n, rlist) {
		rlist_remove(pos, REMOVE_CLEAR, 0);
	}
}

/* 
 * sprint_rlist - output whole rlist.
 *
 * This is a debug facility.
 */
int
rlist_snprintf(char * str, int length, struct render_list * rlist)
{
	/* the length of desired string */
	int x;
	char * p = str;
	struct render_command * rcmd;

#define xsnprintf(fmt...)	\
	do {				\
		x = snprintf(p, length, fmt);	\
		p += x;				\
		if (length >= x)		\
			length -= x;		\
		else				\
			length = 0;		\
	} while(0)

	xsnprintf("Render list %p:\n", rlist);


	rlist_for_each_rcmd(rcmd, rlist) {
		const char * ifactive;
		if (rcmd->active)
			ifactive = "active";
		else
			ifactive = "inactive";

		if (rcmd->name != NULL)
			xsnprintf("%s render command %s: ", ifactive, rcmd->name);
		else
			xsnprintf("%s render command %p: ", ifactive, rcmd);

		if ((rcmd->ops) && (rcmd->ops->snprintf != NULL)) {
			x = rcmd->ops->snprintf(rcmd, p, length);
			p += x;
			if (length >= x)
				length -= x;
			else
				length = 0;
		} else {
			xsnprintf("(no sprint func)\n");
		}
	}

	xsnprintf("End");
	return (p - str) / sizeof(char);
#undef xsnprintf
}

// vim:tabstop=4:shiftwidth=4


