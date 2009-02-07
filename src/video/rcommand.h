/* Wang Nan @ Jan 23, 2009 */

#ifndef VIDEO_RCOMMAND
#define VIDEO_RCOMMAND

#include <common/defs.h>
#include <common/list.h>

#include <video/texture.h>

__BEGIN_DECLS

/* 
 * This file defines the RenderCommand structure.
 *
 * RenderCommand is the base render unit. It can be view as an
 * OpenGL command package. All RenderCommand linked to a RenderList.
 * See video.tex.
 */

/* RenderElement should be defined in relement.h.  RenderElement contain one or
 * more render command in different position of the render list */

struct RenderElement;

/* 
 * How to use RenderCommand?
 *
 * struct XXXRenderCommand {
 *	struct RenderCommand base;
 *	int arg1, arg2 ...
 * };
 *
 * int XXXRender(struct RenderCommand * cmd, tick_t current_time)
 * {
 *	struct XXXRenderCommand * base = container_of(cmd, struct XXXRenderCommand, base);
 *	....
 * }
 */

struct RenderCommand;

enum RemoveReason {
	REMOVE_CLEAR = 0,	/* clear render list */
	REMOVE_FINISH,		/* render func return REMOVE */
	REMOVE_ERROR,		/* Error happend */
	REMOVE_OTHER,		/*  */
};


typedef int (*phy_func)(struct RenderCommand * command, dtick_t delta_ticks);
typedef int (*render_func)(struct RenderCommand * command);
typedef	int (*sprintf_func)(struct RenderCommand * command, char * dest);
typedef	int (*remove_func)(struct RenderCommand * command, enum RemoveReason reason, int flags);

/* 
 * struct RenderCommand - The core structure.
 */
struct RenderCommand {
	const char * name;

	/* Whether the phy func is the first time called. if first is true,
	 * phy will be called with delta_time = 0 */
	bool_t first;
	bool_t stopped;		/* the animate has stopped, render command should draw final
       				   picture, not the middle picture. */

	/* if animate paused, below two fields indicate how to resume drawing */
	bool_t paused;
	tick_t pause_time;
	struct RenderElement * father;
	struct VideoContext * context;
		/* context of the engine. for OpenGL, like the avaliable of fragment shader... */

	struct texture * texture;	/* main texture */
	

	/* phy func: compute the needed value, how to do the rendering? like the
	 * position of each elements, the alpha value in a fade in/out command.
	 * the delta_ticks is the interval mss between last rendering and this
	 * rendering. if the command is marked 'paused', then phy will receive 0
	 * as delta_ticks. the return value of phy is same as render. if phy
	 * return remove, then the command will be removed immediately. */
	phy_func phy;

	/* The main render function  */
	/* This func needn't do any computation, all computation should be done
	 * in phy func.
	 * return value is flags:
	 * 	whether we need to render next command or not? 
	 * 	does this command successed? */
	render_func render;

	/* for debug use. return value is the number of charas printed */
	sprintf_func sprintf;

	/* the command is removed, do some release works */
	remove_func remove;


	struct list_head list;

	void * pprivate;
};

/* below is some rcommand's render return flags */
#define RENDER_FAIL	(1)	/* the command met an error */
#define RENDER_REMOVE	(2)	/* we should remove this command from list */
#define RENDER_STOP	(4)	/* we shouldn't continue rendering */

/* operations */
/* use memset to set each field to 0 */
extern void RCommandInit(struct RenderCommand * command,
		const char * name, 
		struct VideoContext * context,
		phy_func phy,
		render_func render,
		sprintf_func sprintf,
		remove_func remove);

/* There's no "alloc_rcommand", because user always alloc rcommand's
 * subclass */

__END_DECLS

#endif

