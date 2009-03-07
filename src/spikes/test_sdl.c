#include <SDL/SDL.h>
#include <SDL/SDL_video.h>

#include <stdio.h>
#include <stdlib.h>

int main()
{
	SDL_Surface * screen = NULL;
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	SDL_Quit();
	return 0;
}

