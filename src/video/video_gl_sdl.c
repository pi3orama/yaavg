/* 
 * video_gl_sdl.c
 * by WN @ Mar. 9, 2009
 */

#include <common/debug.h>
#include <common/exception.h>
#include <common/utils.h>
#include <econfig/econfig.h>

#include <SDL/SDL.h>
#include <SDL/SDL_video.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <video/video_engine.h>
#include <video/video_gl.h>


#ifdef VIDEO_OPENGL_SDL_DRIVER

static struct sdl_context {
	struct gl_context base;
	int bpp;
	int samples;
	const char * gllibrary;

	/* some sdl specific fields */
	SDL_Surface *screen;
} _sdl_ctx;

/* use sdl_ctx to indicate if sdl subsystem is inited. this
 * pattern is same as video.c and video_gl.c.
 * Wow! I'm so clever.... */
static struct sdl_context * sdl_ctx = NULL;

static void
init_sdl(void);

static void
open_window(void);


static void
__gl_close(struct cleanup * str);
static struct cleanup gl_cleanup_str = {
	.function	= __gl_close,
	.list		= {NULL, NULL},
};
static void
__gl_close(struct cleanup * str)
{
	assert(str == &gl_cleanup_str);
	if (sdl_ctx != NULL) {
		TRACE(SDL, "sdl close\n");
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		sdl_ctx = NULL;
	}
}

void
gl_close(void)
{
	__gl_close(&gl_cleanup_str);
}

struct gl_context *
gl_init(void)
{
	if (sdl_ctx != NULL) {
		WARNING(OPENGL, "multi gl_init\n");
		return &sdl_ctx->base;
	}
	init_sdl();
	
	sdl_ctx = &_sdl_ctx;
	make_cleanup(&gl_cleanup_str);

	open_window();

	return &sdl_ctx->base;
}

void
gl_reinit(void)
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
	sdl_ctx = NULL;
	init_sdl();
	open_window();
}

void *
gl_get_proc_address(const char * name)
{
	return SDL_GL_GetProcAddress(name);
}


static void
init_sdl(void)
{
	int err, samples, bpp;
	const char * library_name = NULL;

	if (sdl_ctx != NULL) {
		ERROR(SDL, "sdl has already inited\n");
		THROW(EXCEPTION_FATAL, "sdl has already inited");
	}

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		FATAL(SDL, "Failed to init SDL video subsystem, SDL report: \"%s\", "
				"check for your configuration.\n", SDL_GetError());
		/* exit immediatly */
		THROW(EXCEPTION_FATAL, "init_sdl failed");
	}

	if (!conf_get_bool("video.sdl.blocksigint", TRUE)) {
		unblock_sigint();
	}

	/* ... */
	sdl_ctx = &_sdl_ctx;

	library_name = conf_get_string("video.opengl.gllibrary", NULL);
	if (library_name != NULL)
		VERBOSE(SDL, "Desire OpenGL library: %s\n", library_name);
	else
		VERBOSE(SDL, "Load default OpenGL library\n");

	if (SDL_GL_LoadLibrary(library_name) != 0) {
		FATAL(SDL, "Load OpenGL library failed: %s\n", SDL_GetError());
		/* don't use gl_close, we haven't set sdl_ctx */
		SDL_QuitSubSystem(SDL_INIT_VIDEO);
		THROW(EXCEPTION_FATAL, "Load OpenGL library failed");
	}

	sdl_ctx->gllibrary = library_name;
	
	/* check vsync */
	if (conf_get_bool("video.opengl.vsync", FALSE)) {
		err = SDL_GL_SetAttribute (SDL_GL_SWAP_CONTROL, 1);
		VERBOSE(SDL, "Turn vsync on\n");
	} else {
		err = SDL_GL_SetAttribute (SDL_GL_SWAP_CONTROL, 0);
		VERBOSE(SDL, "Turn vsync off\n");
	}

	if (err != 0)
		WARNING(SDL, "Set vsync failed.\n");

	if ((samples = conf_get_integer("video.opengl.multisample", 0)) != 0) {
		SDL_GL_SetAttribute (SDL_GL_MULTISAMPLEBUFFERS, 1);
		err = SDL_GL_SetAttribute (SDL_GL_MULTISAMPLESAMPLES, samples);
		VERBOSE(SDL, "Set multisampling: %d\n", samples);
		if (err != 0)
			WARNING(SDL, "Set multisample failed.\n");
	}
	sdl_ctx->samples = samples;

	bpp = conf_get_integer("video.opengl.bpp", 16);

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
	sdl_ctx->bpp = bpp;

	SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
	return;
}

static void
open_window(void)
{
	
	/* SDL_SetVideoMode's params */
	int w, h, flags;
	bool_t full_screen, resizable;
	SDL_Surface * screen = NULL;

	TRACE(SDL, "try SDL OpenWindow\n");
	if (sdl_ctx == NULL) {
		ERROR(SDL, "try OpenWindow before init sdl platform.\n");
		THROW(EXCEPTION_FATAL,
				"try OpenWindow before init sdl platform");
	}
	
	w = conf_get_integer("video.resolution.w", 640);
	h = conf_get_integer("video.resolution.h", 480);
	full_screen = conf_get_bool("video.fullscreen", FALSE);
	resizable = conf_get_bool("video.resizable", FALSE);

	flags = SDL_OPENGL;

	if (resizable)
		flags |= SDL_RESIZABLE;
	if (full_screen)
		flags |= SDL_FULLSCREEN;

	TRACE(SDL, "ready to init window:\n");
	TRACE(SDL, "\t width = %d\n", w);
	TRACE(SDL, "\t height = %d\n", h);
	TRACE(SDL, "\t full_screen = %d\n", full_screen);
	TRACE(SDL, "\t resizable = %d\n", resizable);

	screen = SDL_SetVideoMode(w, h, sdl_ctx->bpp, flags);
	if (screen == NULL) {
		FATAL(SDL, "Failed to set video mode to %ix%i, SDL report: \"%s\", "
				"check for your configuration!\n", w, h,
				SDL_GetError());
		THROW(EXCEPTION_FATAL, "set video mode failed");
	}

	/* Set context structure */
	sdl_ctx->base.base.width = w;
	sdl_ctx->base.base.height = h;
	sdl_ctx->base.base.full_screen = full_screen;
	sdl_ctx->base.platform = "SDL";

	sdl_ctx->screen = screen;
	return;
}

void
video_swap_buffers(void)
{
	SDL_GL_SwapBuffers();
}

void
video_set_caption(const char * caption)
{
	TRACE(SDL, "Set WM caption to %s\n", caption);
	SDL_WM_SetCaption(caption, NULL);
	return;
}

void
video_set_icon(const icon_t icon)
{
	WARNING(SDL, "Video has not implentmented\n");
	return;
}

#endif	/* VIDEO_OPENGL_SDL_DRIVER */

// vim:tabstop=4:shiftwidth=4
