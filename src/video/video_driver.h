/* 
 * video_driver.h
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
driver_init(void);

extern void
driver_reinit(void);

/* 
 * the same as video_close, we don't really need a driver_close.
 */
extern void
driver_close(void);

extern void
driver_read_pixels_rgb(uint8_t * buffer, struct view_port vp)
	THROWS(CONTINUE, FATAL);

extern void
driver_read_pixels_rgba(uint8_t * buffer, struct view_port vp)
	THROWS(CONTINUE, FATAL);

__END_DECLS

#endif

