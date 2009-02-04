/* 
 * by WN @ Jan 28, 2009
 *
 * engine_gl.h - throw-away prototype of OpenGL engine definition
 *
 */

#ifndef VIDEO_ENGINE_GL_H
#define VIDEO_ENGINE_GL_H

#include <common/defs.h>
#include <video/engine.h>

__BEGIN_DECLS

/* Below is opengl functions definitions */
/* define struct GLFuncs  */

#include "gl_funcs.h"


struct GLEngineContext {
	struct VideoEngineContext base;
	const char *vendor;
	const char *renderer;
	const char *version;

	/* a very long string */
	const char *extensions;

	/* Which platform we used? SDL or GLX or WGL... */
	const char *platform;

	/* belong is for dynamic gl use: func pointers */
	struct GLFuncs gl_funcs;
};

#define GetGLCtx(x) container_of(x, struct GLEngineContext, base)

/* 
 * in engine_gl, we define a global gl engine context pointer. for
 * static plugins, it can see this pointer directly; for dynamic
 * plugins, use gcc -E can make shared-object reference symbols in main
 * program. However, let plugins offer plug-init utils should be better.
 *
 * This pointer is OK after platform inited. In fact, it is the return
 * value of PlatformInit. The real structure is defined in engine_gl_sdl/glx...
 */
extern const struct GLEngineContext * GLContext;

#define GLOPS	(&GLContext->gl_funcs)
/* OpenGL init has 2 phases: 
 * 1. platform(sdl or glx) initialization, find gl functions, check extentions;
 * 2. Windows initialization, show window, wait for render command.
 *
 * The former one correspond to DriverInit, the later one correspond to
 * DriverOpenWindow.
 *
 * We don't define GLInit,..., because engine_gl implentments engine_platform.h.
 */

/* XXX */
/* define some open gl operations which plugins needs. */

/* Below function need to be implentmented by engine_gl_xxx */

/* Some support functions which are needed in engine_gl.c */
/* 
 * reference: darkplaces - vid_sdl.c
 */

extern int GLPlatformInit(void);
extern void GLPlatformClose(struct GLEngineContext * context);
extern void *GLGetProcAddress(const char *name);
extern struct GLEngineContext * GLOpenWindow(void);

/* WM operations */
extern void WMSetCaption(const char * caption);
extern void WMSetIcon(const icon_t icon);


__END_DECLS

#endif


