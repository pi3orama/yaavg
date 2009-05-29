/* 
 * video.h
 * by WN @ Mar. 08, 2009
 */

#ifndef VIDEO_H
#define VIDEO_H

#include <common/defs.h>
#include <common/exception.h>
#include <video/rlist.h>
#include <video/rcommand.h>

__BEGIN_DECLS

struct video_context {
	const char * engine_name;
	int width, height;
	struct view_port {
		int x, y, w, h;
	} view_port;
	bool_t full_screen;

	tick_t start_time;
	struct render_list render_list;
};

extern struct video_context *
video_get_current_context(void);

extern struct video_context *
video_init(void);

/* In the final stage of video reinit,
 * call each hook in a hook list, to notify
 * the objects which sensable of video reinit */
extern void
video_reinit(void);


extern void
video_hook_reinit(struct reinit_hook * hook);

extern void
video_unhook_reinit(struct reinit_hook * hook);

/* We don't really need a video_close, because the try-catch structure
 * can do the job. However, we keep this func for consistency. */
extern void
video_close(void);

/* Render is for rendering a ``SINGLE'' frame, not animination */
/* return value indicates whether we have a successfully rendering or not,
 * 0 is OK. */
extern int
video_render(dtick_t delta_time);

/* below implentmented in engine_xxx.c */

extern void
video_swap_buffers(void);

extern void
video_reshape(int w, int h);

extern void
video_set_caption(const char * caption);

extern void
video_set_icon(const icon_t icon);

extern void
video_screen_shot(void) THROWS(CONTINUE);

/* Video engine is respond to generate game time */
extern tick_t
get_game_ticks(void);

/* Some Future plan */

/* SetMousePosition */
/* SetMouseCursor */

/* --------------------------------------------------- */
/* video command operations */
typedef enum _BOA {
	BEFORE,
	AFTER,
} BOA_t;

extern void 
video_insert_command(struct render_command * cmd, BOA_t boa,
		struct render_command * pos);

/* meaning of pos:
 * lpos: null means the head of whole list,
 * rpos: null means after left command */
extern void
video_insert_command_pair(struct render_command * lcmd,
		BOA_t lboa,
		struct render_command * lpos,
		struct render_command * rcmd,
		BOA_t rboa,
		struct render_command * rpos);

extern void
video_remove_command(struct render_command * cmd,
		rcmd_remove_reason_t reason,
		int flags);

extern void
video_remove_command_pair(struct render_command * lcmd,
		rcmd_remove_reason_t reason,
		int flags);

#define video_remove_a_command(cmd, reason, flags)	\
	do {						\
		if (rcmd_is_pair(cmd))			\
			video_remove_command_pair((cmd), (reason), (flags));\
		else					\
			video_remove_command((cmd), (reason), (flags));\
	} while(0)

__END_DECLS

#endif

// vim:tabstop=4:shiftwidth=4

