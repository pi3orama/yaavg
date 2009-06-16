/* by WN @ Feb 05, 2009 */

#include <common/defs.h>
#include <common/debug.h>
#include <common/exception.h>
#include <video/x_common.h>
#include <video/video.h>	/* reinit hooks */
#include <econfig/econfig.h>

#include <X11/Xlib.h>

#ifdef VIDEO_OPENGL_GLX_DRIVER

static Display * display;
static Window main_win;

static void
__event_init(void)
{
	display = x_get_display();
	main_win = x_get_main_window();
}

static void
event_reinit(struct reinit_hook * h)
{
	__event_init();
}

static struct reinit_hook glx_event_reinit = {
	.fn = event_reinit,
};

void event_init(void)
{
	__event_init();
	/* hook a reinit hook */
	video_hook_reinit(&glx_event_reinit);
}

int event_poll(void)
{
	/* May be an sync event handler is better? */
	/* It is not a good idea, unless we can set only mousemove event to sync,
	 * and keep kbd and mouse button async. for kbd, it is possiable,
	 * for button, don't know. */

	/* We'd better unselect all event here? */
	/* for performance consideration, it may not be a good idea */
	/* xrand need do process XRRUpdateConfiguration */

	int xxx = 0;
	while (XPending(display)) {
		XEvent xev;
		XNextEvent(display, &xev);
		XCheckError();

		/* Check for exit event */
		//TRACE(EVENT, "Grab an event, type=%d\n", xev.type);
		switch (xev.type) {
			case KeyPress: {
				XKeyEvent * xkey = &xev.xkey;
				KeyCode kc = xkey->keycode;
				//FORCE(EVENT, "Keycode=0x%x\n", kc);
				if (kc == 0x18)
					xxx = 1;
				break;
			}
			case ButtonPress: {
				xxx = 1;
				break;
			}
			case MotionNotify: {
				break;
			}
			default:
				break;
		}
	}
	
	return xxx;
}

#endif

// vim:ts=4:sw=4
