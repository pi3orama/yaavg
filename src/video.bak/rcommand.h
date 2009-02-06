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

typedef int (*render_func)(struct RenderCommand * command, tick_t current_time);
typedef	int (*sprintf_func)(struct RenderCommand * command, char * dest);
typedef	int (*remove_func)(struct RenderCommand * command);

/* 
 * struct RenderCommand - The core structure.
 */
struct RenderCommand {
	const char * name;
	/* RenderCommand renders not only statical images, it
	 * also renders animate. This because some common animatation,
	 * like move and rotate,  */
	tick_t start_time;
	bool_t stopped;		/* the animate has stopped, render command should draw final
       				   picture, not the middle picture. */

	/* if animate paused, below two fields indicate how to resume drawing */
	bool_t paused;
	tick_t pause_time;
	struct RenderElement * father;
	struct VideoEngineContext * context;
		/* context of the engine. for OpenGL, like the avaliable of fragment shader... */

	struct texture * texture;	/* main texture */
	
	/* The main render function  */
	/* current_time is the time when the main display func is called
	 * NOT the time render function be called. 
	 * return value is flags:
	 * 	whether we need to render next command or not? 
	 * 	does this command successed? */
	render_func render;

	/* for debug use. return value is the number of charas printed */
	sprintf_func sprintf;

	/* the command is removed, do some release works */
	remove_func remove;


	struct list_head list;
};

/* below is some rcommand's render return flags */
#define RENDER_FAIL	(1)	/* the command met an error */
#define RENDER_CONT	(2)	/* we should continue rendering */
#define RENDER_REMOVE	(4)	/* we should remove this command from list */

/* operations */
/* use memset to set each field to 0 */
extern void RCommandInit(struct RenderCommand * command,
		const char * name, 
		struct VideoEngineContext * context,
		render_func render,
		sprintf_func sprintf,
		remove_func remove);

/* There's no "alloc_rcommand", because user always alloc rcommand's
 * subclass */

__END_DECLS

#endif

