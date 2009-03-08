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

extern void
driver_close(void);

extern void
driver_read_pixels(uint8_t * buffer, int x, int y, int w, int h);

__END_DECLS

#endif

