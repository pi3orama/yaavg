/* 
 * by WN @ Feb 3, 2009
 */

/* 
 * engine_gl_sdl.c - SDL support OpenGL
 */

#include <common/debug.h>
#include <econfig/econfig.h>

#include <SDL/SDL.h>
#include <SDL/SDL_video.h>

#include <stdio.h>
#include <stdlib.h>
#include <video/engine_gl.h>

static struct SDLEngineContext {
	struct GLEngineContext gl_context;
	int bpp;
	int samples;
	const char * gllibrary;

	/* some sdl specific fields */
	SDL_Surface *screen;
} sdl_context;

void * GLGetProcAddress(const char * name)
{
	return SDL_GL_GetProcAddress(name);
}

/* 
 * sdl_active: prevent multi-close SDL.
 */
static bool_t sdl_active = FALSE;
/* 
 * whether SDL window is shown
 */
static bool_t sdl_window_active = FALSE;


static void sdl_close(void)
{
	if (sdl_active) {
		TRACE(SDL, "sdl close\n");
		SDL_Quit();
		sdl_active = FALSE;
	}
}

int GLPlatformInit(void)
{
	int err, samples, bpp;
	const char * library_name = NULL;

	TRACE(SDL, "sdl platform init\n");

	memset(&sdl_context, sizeof(sdl_context), '\0');

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		FATAL(SDL, "Failed to init SDL video subsystem, SDL report: \"%s\", "
				"check for your configuration.\n", SDL_GetError());
		/* exit immediatly */
		exit(1);
	}

	library_name = ConfGetString("video.opengl.driver.gllibrary", NULL);
	if (library_name != NULL)
		VERBOSE(SDL, "Desire OpenGL library: %s\n", library_name);
	else
		VERBOSE(SDL, "Load default OpenGL library\n");

	if (SDL_GL_LoadLibrary(library_name) != 0) {
		FATAL(SDL, "Load OpenGL library failed: %s\n", SDL_GetError());
		sdl_close();
		exit(1);
	}
	sdl_context.gllibrary = library_name;

	/* check vsync */
	if (ConfGetBool("video.opengl.driver.vsync", FALSE)) {
		err = SDL_GL_SetAttribute (SDL_GL_SWAP_CONTROL, 1);
		VERBOSE(SDL, "Turn vsync on\n");
	} else {
		err = SDL_GL_SetAttribute (SDL_GL_SWAP_CONTROL, 0);
		VERBOSE(SDL, "Turn vsync off\n");
	}

	if (err != 0)
		WARNING(SDL, "Set vsync failed.\n");

	if ((samples = ConfGetInteger("video.opengl.driver.multisample", 0)) != 0) {
		SDL_GL_SetAttribute (SDL_GL_MULTISAMPLEBUFFERS, 1);
		err = SDL_GL_SetAttribute (SDL_GL_MULTISAMPLESAMPLES, samples);
		VERBOSE(SDL, "Set multisampling: %d\n", samples);
		if (err != 0)
			WARNING(SDL, "Set multisample failed.\n");
	}
	sdl_context.samples = samples;

	bpp = ConfGetInteger("video.opengl.driver.bpp", 16);

	if (bpp >= 32) {
		SDL_GL_SetAttribute (SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute (SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute (SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute (SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute (SDL_GL_STENCIL_SIZE, 8);
	} else {
		SDL_GL_SetAttribute (SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute (SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute (SDL_GL_BLUE_SIZE, 5);
		SDL_GL_SetAttribute (SDL_GL_DEPTH_SIZE, 16);
	}
	sdl_context.bpp = bpp;

	SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);

	sdl_active = TRUE;

	atexit(sdl_close);
	return 0;
}

void GLPlatformClose(struct GLEngineContext * context)
{
	sdl_close();
	return;
}

struct GLEngineContext * GLOpenWindow(void)
{

	/* SDL_SetVideoMode's params */
	int w, h, bpp, flags;
	bool_t fullscreen, resizable;
	SDL_Surface * screen = NULL;

	TRACE(SDL, "try SDL OpenWindow\n");
	if (!sdl_active) {
		ERROR(SDL, "try OpenWindow before init sdl platform.\n");
		return NULL;
	}
	
	w = ConfGetInteger("video.resolution.w", 640);
	h = ConfGetInteger("video.resolution.h", 480);
	fullscreen = ConfGetBool("video.fullscreen", FALSE);
	resizable = ConfGetBool("video.resizable", FALSE);

	flags = SDL_OPENGL;

	if (resizable)
		flags |= SDL_RESIZABLE;
	if (fullscreen)
		flags |= SDL_FULLSCREEN;

	TRACE(SDL, "ready to init window:\n");
	TRACE(SDL, "\t width = %d\n", w);
	TRACE(SDL, "\t height = %d\n", h);
	TRACE(SDL, "\t fullscreen = %d\n", fullscreen);
	TRACE(SDL, "\t resizable = %d\n", resizable);

	screen = SDL_SetVideoMode(w, h, sdl_context.bpp, flags);
	if (screen == NULL) {
		FATAL(SDL, "Failed to set video mode to %ix%i, SDL report: \"%s\", "
				"check for your configuration!\n", w, h,
				SDL_GetError());
		sdl_close();
		return NULL;
	}

	/* Set context structure */
	sdl_context.gl_context.base.driver_name = "OpenGL";
	sdl_context.gl_context.base.width = w;
	sdl_context.gl_context.base.height = h;
	sdl_context.gl_context.base.is_full_screen = fullscreen;


	sdl_context.gl_context.platform = "SDL";
	sdl_context.screen = screen;


	return &sdl_context.gl_context;
}

void GLSwapBuffers(struct GLEngineContext * gl_context)
{
	SDL_GL_SwapBuffers();
}

void WMSetCaption(const char * caption)
{
	TRACE(SDL, "Set WM caption to %s\n", caption);
	SDL_WM_SetCaption(caption, NULL);
	return;
}

void WMSetIcon(const icon_t icon)
{
	WARNING(SDL, "WMSetIcon has not implentmented\n");
	return;
}

