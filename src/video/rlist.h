/* Wang Nan @ Jan 26, 2009 */

#ifndef VIDEO_RLIST
#define VIDEO_RLIST

#include <common/defs.h>
#include <common/list.h>
#include <video/rcommand.h>
#include <common/debug.h>

__BEGIN_DECLS

/* 
 * This file defines the RenderList structure.
 *
 * RenderList is the main operation unit. It is a linked list,
 * RenderCommand(s) linked into it. The main render program call
 * rlist_render periodically (like per 1/60s), the render function
 * do some initial works, then for each RenderCommand, call its render
 * function. See video.tex for detail.
 *
 * WARNING! Before call any render function, the whole video system needs
 * to be initialized.
 */

struct RenderList {
	struct list_head command_list;
	void * context;	/* context of the engine. See comment in rcommand.h */
};

extern void init_rlist(struct RenderList * rlist, void * context);


/* functions operate RenderList */
static inline void link_tail(struct RenderList * list,
		struct RenderCommand * command)
{
	list_add_tail(&command->list, &list->command_list);
}

static inline void link_head(struct RenderList * list,
		struct RenderCommand * command)
{
	list_add(&command->list, &list->command_list);
}

static inline void link_before(struct RenderCommand * dest,
		struct RenderCommand * command)
{
	list_add_tail(&command->list, &dest->list);
}

static inline void link_after(struct RenderCommand * dest,
		struct RenderCommand * command)
{
	list_add(&command->list, &dest->list);
}

static inline int link_foreach(struct RenderList * link, void * args,
		int (*func)(struct RenderCommand *, void * args))
{
	struct RenderCommand * pos, *n;
	int res;
	list_for_each_entry_safe(pos, n, &link->command_list, list) {
		if ((res = func(pos, args)) != 0)
			return res;
	}
	return 0;
}

static inline int link_foreach_force(struct RenderList * link, void * args,
		int (*func)(struct RenderCommand *, void * args))
{
	struct RenderCommand * pos, *n;
	int res = 0, res2;
	list_for_each_entry_safe(pos, n, &link->command_list, list) {
		res2 = func(pos, args);
		if (res2 != 0)
			res = res2;
	}
	return res;
}

/* for debug use */
extern int sprint_rlink(char * str, struct RenderCommand * command);

__END_DECLS

#endif

