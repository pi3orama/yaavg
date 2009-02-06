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


#define RListLinkHead(rlist, command) \
	list_add(&(command)->list, &(rlist)->command_list)

#define RListLinkTail(rlist, command) \
	list_add_tail(&(command)->list, &(rlist)->command_list)

#define RListLinkBefore(dest, command)	\
	list_add_tail(&(command)->list, &(dest)->list)

#define RListLinkAfter(dest, command)	\
	list_add(&(command)->list, &(dest)->list)

#define RListForEachCommand(pos, rlist) \
	list_for_each_entry(pos, &((rlist)->command_list), list)

/* don't call remove!  */
#define RListRemove(cmd, reason, flags) \
	do { \
		list_del(&(cmd)->list);		\
		if (cmd->remove != NULL)	\
			cmd->remove(cmd, reason, flags); \
	} while(0)


#define RListForEachCommandSafe(pos, n, rlist) \
	list_for_each_entry_safe(pos, n, &((rlist)->command_list), list)
/* for debug use */
extern int
rlist_sprint(char * dest, struct RenderList * list);

__END_DECLS

#endif

