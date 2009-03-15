/* 
 * rlist.h
 * by WN @ Mar. 08, 2009
 *
 * definition of render list
 *
 */

#ifndef VIDEO_RLIST_H
#define VIDEO_RLIST_H

#include <common/defs.h>
#include <common/list.h>
#include <common/debug.h>
#include <video/rcommand.h>

__BEGIN_DECLS

/* 
 * This file defines the struct render_list structure.
 *
 * struct render_list is the main operation unit. It is a linked list,
 * render command(s) are linked into it. The main render program call
 * rlist_render periodically (like per 1/60s), the render function
 * do some initial works, then for each RenderCommand, call its render
 * function. See video.tex for detail.
 *
 * WARNING! Before call any render function, the whole video system needs
 * to be initialized.
 */

struct render_list {
	struct list_head command_list_head;
};

extern void
rlist_init(struct render_list * rlist);

extern void
rlist_clear(struct render_list * rlist);

static inline struct render_command *
rlist_get_first_command(struct render_list * rlist)
{
	if (list_empty(&(rlist->command_list_head)))
		return NULL;
	return list_entry(rlist->command_list_head.next,
			struct render_command, list);
}

static inline struct render_command *
rlist_get_last_command(struct render_list * rlist)
{
	if (list_empty(&(rlist->command_list_head)))
		return NULL;
	return list_entry(rlist->command_list_head.prev,
			struct render_command, list);
}

static inline struct render_command *
rlist_get_next_command(struct render_list * rlist,
		struct render_command * command)
{
	if (command == NULL)
		return rlist_get_first_command(rlist);
	if (command->list.next == &(rlist->command_list_head))
		return NULL;
	return list_entry(command->list.next, struct render_command,
			list);
}

static inline struct render_command *
rlist_get_prev_command(struct render_list * rlist,
		struct render_command * rcmd)
{
	if (rcmd == NULL)
		return rlist_get_last_command(rlist);
	if (rcmd->list.prev == &(rlist->command_list_head))
		return NULL;
	return list_entry(rcmd->list.prev,
			struct render_command,
			list);
}

#define rlist_link_head(rlist, rcmd) \
	do {	\
		list_add(&(rcmd)->list, &(rlist)->command_list_head); \
		rcmd_set_inserted(rcmd);\
	} while(0)

#define rlist_link_tail(rlist, rcmd) \
	do {	\
		list_add_tail(&(rcmd)->list, &(rlist)->command_list_head);\
		rcmd_set_inserted(rcmd);\
	}while(0)

static inline void
rlist_link_before(struct render_list * rlist,
		struct render_command * dest,
		struct render_command * rcmd)
{
	if (dest == NULL) {
		rlist_link_tail(rlist, rcmd);
		return;
	}
	list_add_tail(&(rcmd)->list, &(dest->list));
	rcmd_set_inserted(rcmd);
}

static inline void
rlist_link_after(struct render_list * rlist,
		struct render_command * dest,
		struct render_command * rcmd)
{
	if (dest == NULL) {
		rlist_link_head(rlist, rcmd);
		return;
	}
	list_add(&(rcmd->list), &(dest->list));
	rcmd_set_inserted(rcmd);
}

#define rlist_remove(rcmd, reason, flags)	\
	do {					\
		list_del(&(rcmd)->list);	\
		rcmd_unset_inserted(rcmd);	\
		rcmd_remove((rcmd), (reason), (flags));\
	} while(0)

#define rlist_for_each_rcmd(pos, rlist) \
	list_for_each_entry(pos, &((rlist)->command_list_head), list)

#define rlist_for_each_rcmd_safe(pos, n, rlist) \
	list_for_each_entry_safe(pos, n, &((rlist)->command_list_head), list)

/* This is a debug facility,  */
extern int
rlist_snprintf(char * dest, int length, struct render_list * list);

__END_DECLS

#endif

