/* 
 * x_common.h
 * by WN @ Jun 07, 2009
 */

#ifndef _X_COMMON_H
#define _X_COMMON_H

#include <common/defs.h>
#include <X11/Xlib.h>

__BEGIN_DECLS

/* X11 based video should export some infos for events processing */

Display *
x_get_display(void);

Window
x_get_main_window(void);

bool_t x_failed;

#define XCheckError()	do {			\
	if (x_failed)	{					\
		FATAL(GLX, "X11 error\n");	\
		THROW(EXCEPTION_FATAL, "X11 error");\
	}									\
} while(0)

__END_DECLS

#endif
// vim:ts=4:sw=4
