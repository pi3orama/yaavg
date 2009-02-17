/* 
 * by WN @ Feb 06, 2009
 */


#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H


#include <common/defs.h>
#include <video/engine.h>

__BEGIN_DECLS
extern int
DriverInit(void);

extern struct VideoContext *
DriverOpenWindow(void);

extern void
DriverReopenWindow(struct VideoContext * ctx);

extern void
DriverClose(void);
__END_DECLS
#endif

