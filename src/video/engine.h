/* 
 * by WN @ Feb 06, 2009
 */


#ifndef VIDEO_ENGINE_H
#define VIDEO_ENGINE_H

#include <common/defs.h>
#include <video/rlist.h>
#include <video/rcommand.h>

__BEGIN_DECLS


struct VideoContext {
	const char * driver_name;
	int width, height;
	bool_t is_full_screen;

	tick_t start_time;

	/* the render list */
	struct RenderList render_list;
};

extern int
VideoInit(void);

extern struct VideoContext *
VideoOpenWindow(void);

/* 
 * If only called VideoInit, then pass NULL
 */
extern void
VideoClose(void);


extern int
VideoPhy(dtick_t delta_time);

/* Render is for render a ``SINGLE'' frame, not animination */
/* return value indicates whether we have a successfully rendering or not,
 * 0 is OK. */
extern int
VideoRender(void);

/* RCommand operations */
/* boa: before or after */
typedef enum _BOA {
	BEFORE,
	AFTER,
} BOA_t;

extern int
VideoInsertCommand(struct RenderCommand * cmd, BOA_t boa,
	       	struct RenderCommand * pos);

/* meaning of pos:
 * lpos: null means the head of all list,
 * rpos: null means after left command */
extern int
VideoInsertCommandPair(struct RenderCommand * lcmd,
		BOA_t lboa,
		struct RenderCommand * lpos, 
		struct RenderCommand * rcmd,
		BOA_t rboa,
		struct RenderCommand * rpos);

extern int
VideoRemoveCommand(struct RenderCommand * cmd,
		RemoveReason_t reason,
		int flags);

extern int
VideoRemoveCommandPair(struct RenderCommand * lcmd,
		RemoveReason_t reason,
		int flags);

#define VideoRemoveACommand(cmd, reason, flags) \
	do {					\
		if ((cmd)->pairflag != 0)	\
			VideoRemoveCommandPair((cmd), (reason), (flags));\
		else				\
			VideoRemoveCommand((cmd), (reason), (flags));	\
	} while(0)

/* below implentmented in engine_xxx.c */

extern void
VideoSwapBuffers(void);

extern int
VideoReshape(int w, int h);

extern void
VideoSetCaption(const char * caption);

extern void
VideoSetIcon(const icon_t icon);

/* Video engine is respond to generate game time */
extern tick_t
GetGameTicks(void);

/* Some Future plan */

/* SetMousePosition */
/* SetMouseCursor */

__END_DECLS

#endif

