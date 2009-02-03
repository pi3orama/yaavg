/* 
 * by WN @ Jan 28, 2009
 *
 * engine.h - throw-away prototype of engine definition
 */
#ifndef VIDEO_ENGINE_H
#define VIDEO_ENGINE_H

#include <common/defs.h>

__BEGIN_DECLS

struct VideoEngineContext {
	const char * driver_name;
	int width, height;
	bool_t is_full_screen;

	/* 2 clock value, used in rendreing */
	tick_t start_time;
	tick_t current_time;

	/* the render list */
	struct RenderList * render_list;
};


/* below functions should be implentmented in engine.c */
/* Render is for render a ``SINGLE'' frame, not animination */
extern void EngineRender(struct VideoEngineContext * context);
/* FIXME */

/* 
 * We need some plugin operations.
 */


/* below functions should be implentmented in engine_xxx.c */
extern struct VideoEngineContext * EngineInit();
extern void EngineClose(struct VideoEngineContext * context);

extern void EngineOpenWindow(struct VideoEngineContext * context);
extern void EngineCloseWindow(struct VideoEngineContext * context);


/* Some WM operations */
extern void EngineSetCaption(struct VideoEngineContext * context, const char * name);
extern void EngineSetIcon(struct VideoEngineContext * context, const icon_t icon);


__END_DECLS

#endif

