/* Wang Nan @ Jan 26, 2009 */

#ifndef VIDEO_RLIST
#define VIDEO_RLIST

#include <common/defs.h>
#include <common/list.h>
#include <common/debug.h>
#include <video/rcommand.h>

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

extern void
RListInit(struct RenderList * rlist, void * context);

extern void
RListClear(struct RenderList * rlist);


static inline struct RenderCommand *
RListGetFirstCommand(struct RenderList * rlist)
{
	if (list_empty(&(rlist->command_list)))
		return NULL;
	return list_entry(rlist->command_list.next,
			struct RenderCommand, list);
}

static inline struct RenderCommand *
RListGetLastCommand(struct RenderList * rlist)
{
	if (list_empty(&(rlist->command_list)))
		return NULL;
	return list_entry(rlist->command_list.prev,
			struct RenderCommand, list);
}

static inline struct RenderCommand *
RListGetNextCommand(struct RenderList * rlist,
		struct RenderCommand * rcommand)
{
	if (rcommand == NULL)
		return RListGetFirstCommand(rlist);
	if (rcommand->list.next == &(rlist->command_list))
		return NULL;
	return list_entry(rcommand->list.next,
			struct RenderCommand,
			list);
}

static inline struct RenderCommand *
RListGetPrevCommand(struct RenderList * rlist,
		struct RenderCommand * rcommand)
{
	if (rcommand == NULL)
		return RListGetLastCommand(rlist);
	if (rcommand->list.prev == &(rlist->command_list))
		return NULL;
	return list_entry(rcommand->list.prev,
			struct RenderCommand,
			list);
}

#define RListLinkHead(rlist, command) \
	list_add(&(command)->list, &(rlist)->command_list)

#define RListLinkTail(rlist, command) \
	list_add_tail(&(command)->list, &(rlist)->command_list)

static inline void
RListLinkBefore(struct RenderList * rlist,
		struct RenderCommand * dest,
		struct RenderCommand * command)
{
	if (dest == NULL) {
		RListLinkTail(rlist, command);
		return;
	}
	list_add_tail(&(command->list), &(dest->list));

}

static inline void
RListLinkAfter(struct RenderList * rlist,
		struct RenderCommand * dest,
		struct RenderCommand * command)
{
	if (dest == NULL) {
		RListLinkHead(rlist, command);
		return;
	}
	list_add(&(command->list), &(dest->list));
}


#define RListForEachCommand(pos, rlist) \
	list_for_each_entry(pos, &((rlist)->command_list), list)

#define RListRemove(cmd, reason, flags) \
	do { \
		list_del(&(cmd)->list);		\
		if ((cmd)->ops->remove != NULL)	\
			(cmd)->ops->remove((cmd), (reason), (flags)); \
	} while(0)


#define RListForEachCommandSafe(pos, n, rlist) \
	list_for_each_entry_safe(pos, n, &((rlist)->command_list), list)
/* for debug use */
extern int
rlist_sprint(char * dest, struct RenderList * list);

__END_DECLS

#endif

