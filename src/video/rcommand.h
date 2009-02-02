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
	struct RenderElement * father;
	void * arguments;	/* arguments of this command */
	void * context;		/* context of the engine.
				  for OpenGL, like the avaliable of fragment render... */

	struct texture * texture;	/* main texture */
	
	/* The main render function  */
	/* current_time is the time when the main display func is called
	 * NOT the time render function be called. 
	 * return value is flags:
	 * 	whether we need to render next command or not? 
	 * 	does this command successed? */
	int (*render)(struct RenderCommand * command, tick_t current_time, bool_t stop);

	/* for debug use. return value is the number of charas printed */
	int (*sprint)(struct RenderCommand * command, char * dest);

	struct list_head list;
	void * private;		/* A pointer point to its private data, maybe container */
};

/* operations */
/* use memset to set each field to 0 */
extern void RCommandInit(struct RenderCommand * command);

/* There's no "alloc_rcommand", because user always alloc rcommand's
 * subclass */

__END_DECLS

#endif

