/* by WN @ Feb 05, 2009 */

#include <common/defs.h>
#include <common/debug.h>
#include <SDL/SDL.h>

void EventInit(void)
{
	/* SDL_VIDEO MUST inited! */
}

int EventPoll(void)
{
	SDL_Event event;
	 while (SDL_PollEvent (&event)) {
	 	switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_q)
					return 1;
				break;
			case SDL_QUIT:
				return 1;
			case SDL_VIDEORESIZE:
				return 0;
		}
	 }
	 return 0;
}

