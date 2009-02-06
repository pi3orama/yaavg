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

	/* 2 clock value, used in rendreing */
	tick_t start_time;
	tick_t current_time;

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


extern void
VideoPrepareRender(tick_t start_time);

/* Render is for render a ``SINGLE'' frame, not animination */
/* return value indicates whether we have a successfully rendering or not,
 * 0 is OK. */
extern int
VideoRender(tick_t current_time);


/* below implentmented in engine_xxx.c */

extern void
VideoSwapBuffers(void);

extern int
VideoReshape(int w, int h);


extern void
VideoSetCaption(const char * caption);

extern void
VideoSetIcon(const icon_t icon);

/* Some Future plan */

/* SetMousePosition */
/* SetMouseCursor */

__END_DECLS

#endif

