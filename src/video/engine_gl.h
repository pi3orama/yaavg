/* 
 * by WN @ Jan 28, 2009
 */


#ifndef VIDEO_ENGINE_GL_H
#define VIDEO_ENGINE_GL_H

#include <common/defs.h>
#include <video/engine.h>
#include <video/engine_driver.h>


/* Below is opengl functions definitions */
/* define struct GLFuncs  */

#include <GL/gl.h>
#include <video/gl_funcs.h>

__BEGIN_DECLS

struct GLContext {
	struct VideoContext base;
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

extern struct GLContext * GLCtx;
#define GLOPS	(&GLCtx->gl_funcs)



extern void *
GLGetProcAddress(const char *name);

extern struct GLContext *
GLOpenWindow(void);

/* If the upper layer calls reset, finally a call to
 * glGLReopenWindow will be issued. GL layer should guarantee
 * the old ctx is useable after reopening */
extern void
GLReopenWindow(struct GLContext * ctx);

extern void
GLClose(void);

__END_DECLS

#endif

