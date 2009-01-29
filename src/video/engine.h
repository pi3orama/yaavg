/* 
 * by WN @ Jan 28, 2009
 *
 * engine.h - throw-away prototype of engine definition
 */
#ifndef VIDEO_ENGINE_H
#define VIDEO_ENGINE_H

#include <common/defs.h>

__BEGIN_DECLS

struct video_engine_context {
	const char * engine_name;
	int width, height;
	struct RenderList * render_list;
};

__END_DECLS

#endif


