/* by WN @ Feb 05, 2009 */

#include <common/defs.h>
#include <common/debug.h>
#include <econfig/econfig.h>
#include <SDL/SDL.h>

#ifdef VIDEO_OPENGL_SDL_DRIVER

void event_init(void)
{
	/* SDL_VIDEO MUST inited! */
}

int event_poll(void)
{
	SDL_Event event;
	while (SDL_PollEvent (&event)) {
		/* We'd better unselect all event here? */
		/* or at least we don't catch mouse move action, and
		 * query mouse position in each frame? */
	 	switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_q)
					return 1;
				if (event.key.keysym.sym == SDLK_f)
					return 2;
				if (event.key.keysym.sym == SDLK_c)
					return 3;
				if (event.key.keysym.sym == SDLK_z)
					return 4;
				if (event.key.keysym.sym == SDLK_x)
					return 5;
				if (event.key.keysym.sym == SDLK_c)
					return 6;
				if (event.key.keysym.sym == SDLK_v)
					return 7;
				if (event.key.keysym.sym == SDLK_b)
					return 8;
				if (event.key.keysym.sym == SDLK_n)
					return 9;
				if (event.key.keysym.sym == SDLK_m)
					return 10;
				break;
			case SDL_QUIT:
				return 1;
			case SDL_VIDEORESIZE:
				return 0;
		}
	 }
	 return 0;
}

#endif

