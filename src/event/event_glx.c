/* by WN @ Feb 05, 2009 */

#include <common/defs.h>
#include <common/debug.h>
#include <econfig/econfig.h>
#include <SDL/SDL.h>

#ifdef VIDEO_OPENGL_GLX_DRIVER

void event_init(void)
{
}

int event_poll(void)
{
	/* We'd better unselect all event here? */
	 return 0;
}

#endif

