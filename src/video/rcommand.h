/* 
 * rcommand.h: definition of render_command
 *
 * by WN @ Mar. 08, 2009
 */

#ifndef VIDEO_RCOMMAND_H
#define VIDEO_RCOMMAND_H

#include <common/defs.h>
#include <common/exception.h>
#include <common/list.h>

#include <video/texture.h>

__BEGIN_DECLS

/* 
 * We need the definition in this header, however, we cannot simply include
 * video.h, because video.h have to include rlist.h, and rlist.h have to include
 * rcommand.h(this header). If we include video.h here, the difinition of
 * struct render_command will go after rlist.h, then rlist.h will receive an
 * incomplete type error.
 */
struct video_context;

/* 
 * This file defines the render_command structure.
 *
 * render_command is the base render unit. It can be view as an
 * OpenGL command package. All render_command linked to a RenderList.
 * See video.tex.
 */

typedef enum _rcmd_remove_reason {
	REMOVE_CLEAR = 0,	/* clear render list */
	REMOVE_FINISH,		/* render func return REMOVE */
	REMOVE_REQUIRE,		/* Upper level remove this command */
	REMOVE_ERROR,		/* Error happend */
	REMOVE_EXCEPTION,	/* rcmd is removed before it linked */
	REMOVE_OTHER,		/*  */
} rcmd_remove_reason_t;

struct render_command;

typedef int (*render_func_t)(struct render_command * command, dtick_t delta_ticks);
struct rcmd_operations {
	/* The main render function  */
	/* render func do both phy computation and render.
	 * I don't interleave the phy and render because I believe the 
	 * render commands and phy computation can be pipelined. */
	/* return value is flags:
	 * 	whether we need to render next command or not? 
	 * 	does this command successed? */
	/* render is the default render function. if the command is part of
	 * pair command, and lrender and/or rrender is not null, those func
	 * will be called instead.
	 *  */
	render_func_t render;
	render_func_t lrender;
	render_func_t rrender;

	int (*init)(struct render_command * command);

	/* remove is a callback, notice the upper level the command has been
	 * removed */
	/* the return value of remove is useless,
	 * should always return 0 */
	int (*remove)(struct render_command * command,
			rcmd_remove_reason_t reason, int flags);

	/* destroy is called by upper level, release all resources this command
	 * alloced */
	int (*destroy)(struct render_command * command);

	/* let the command enter its finial state */
	int (*finish)(struct render_command * command);

	/* 0 means pause, 1 is normal speed. NOTICE: speedup not always a
	 * positive number, a negitive number make the command run backward */
	int (*speedup)(struct render_command * command, float speedup);

	/* for debug use. return value is the number of charas printed */
	int (*snprintf)(struct render_command * command, char * dest, int length);

};

typedef int32_t rcmd_pair_flag_t;

struct render_command {
	const char * name;

	/* NOTICE: ALL bool_t FIELDS ARE FILLED BY RENDER PROCESS, NOT THE OPERATIONS,
	 * EXCEPT revert_time */

	/* Whether the phy func is the first time called. if first is true,
	 * phy will be called with delta_time = 0 */
	bool_t first;

	/* the animate has stopped, render command should draw final picture,
	 * not a middle picture. */
	bool_t stopped;

	/* the command is activated. When first inserted, the command is not
	 * actived. */
	bool_t active;

	/* whether the command is inserted  */
	bool_t inserted;

	/* engine will give a negitive delta time to phy if revert_time is TRUE */
	/* revert_time is filled by the operations, and read by rendering process,
	 * this is different from other bool_t flags. */
	bool_t revert_time;

	/* the definition of pairflag is in video.tex */
	rcmd_pair_flag_t pairflag;

	/* the corresponding pair command */
	struct render_command * pair_rcmd;

	/* the corresponding video context */
	struct video_context * video_context;

	struct texture * texture;

	struct rcmd_operations * ops;

	struct list_head list;

	/* after rcmd inited, before it is linked onto rlist, if
	 * exception happen, make sure the command is removed. */
	struct cleanup cleanup;
	void * pprivate;
};

/* below is some rcommand's render return flags */
#define RENDER_OK	(0)
#define RENDER_FAIL	(1)	/* the command met an error */
#define RENDER_REMOVE	(2)	/* we should remove this command from list */
#define RENDER_STOP	(4)	/* we shouldn't continue rendering */

/* use memset to set each field to 0 */
extern void
rcmd_init(struct render_command * command,
		const char * name,
		bool_t revert_time,
		struct video_context * vctx,
		struct rcmd_operations * ops);
extern void
rcmd_remove(struct render_command * rcmd,
		rcmd_remove_reason_t reason,
		int flags);


extern void
rcmd_set_active(struct render_command * cmd);

extern void
rcmd_set_inserted(struct render_command * cmd);

extern void
rcmd_unset_inserted(struct render_command * cmd);

#define rcmd_is_left(cmd)	((cmd)->pairflag > 0)
#define rcmd_is_right(cmd)	((cmd)->pairflag < 0)
#define rcmd_is_pair(cmd)	(((cmd)->pairflag != 0) && ((cmd)->pair_rcmd != NULL))
#define rcmd_is_active(cmd)	((cmd)->active)
#define rcmd_is_first(cmd)	((cmd)->first)
#define rcmd_is_stopped(cmd)	((cmd)->stopped)
#define rcmd_is_inserted(cmd)	((cmd)->inserted)
#define rcmd_is_revert(cmd)	((cmd)->revert_time)

#define rcmd_set_first(cmd)	((cmd)->first = TRUE)
#define rcmd_unset_first(cmd)	((cmd)->first = FALSE)

/* There's no "alloc_rcommand", because user always alloc rcommand's
 * subclass */

__END_DECLS

#endif

