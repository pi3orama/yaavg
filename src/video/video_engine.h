/* 
 * video_engine.h
 * by WN @ Mar. 08, 2009
 *
 * data structure needed in opengl level
 */

#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include <common/defs.h>
#include <video/video.h>

__BEGIN_DECLS

extern struct video_context *
engine_init(void);

extern void
engine_reinit(void);

/* 
 * the same as video_close, we don't really need a engine_close.
 */
extern void
engine_close(void);

/* 
 * reset some vars. for examplem in opengl,
 * begin_frame should reload ModelView matrix
 */
extern void
engine_begin_frame(void);

/* 
 * after all render cmds finish.
 * in opengl, end_frame check glError
 */
extern void
engine_end_frame(void);

extern void
engine_read_pixels_rgb(uint8_t * buffer, struct view_port vp)
	THROWS(CONTINUE, FATAL);

extern void
engine_read_pixels_rgba(uint8_t * buffer, struct view_port vp)
	THROWS(CONTINUE, FATAL);

__END_DECLS

#endif

