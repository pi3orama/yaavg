/* 
 * by WN @ Jan 28, 2009
 *
 * engine_gl.c - a throw-away prototype of gl engine
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/defs.h>
#include <common/debug.h>

#include <econfig/econfig.h>

#include <video/engine_gl.h>
#include <video/engine.h>

struct GLEngineContext * GLContext = NULL;

int EngineInit(void)
{
	if (GLPlatformInit()) {
		FATAL(OPENGL, "OpenGL Platform init failed\n");
		exit(1);
	}
	return 0;
}

void EngineClose(struct VideoEngineContext * context)
{
	GLPlatformClose(GetGLCtx(context));
	return;
}

struct VideoEngineContext * EngineOpenWindow(void)
{
	struct GLEngineContext * gl_context;
	gl_context = GLOpenWindow();
	if (gl_context == NULL) {
		WARNING(OPENGL, "Open Window failed\n");
		return NULL;
	}

	GLContext = gl_context;

	/* Init OpenGL */
	/* first, get opengl func pointer */

	/* ??? */

	TRACE(OPENGL, "Vendor: %s\n", glGetString(GL_VENDOR));

	return &gl_context->base;
}

void EngineCloseWindow(struct VideoEngineContext * context)
{
	return;
}

