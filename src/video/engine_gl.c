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

const struct GLEngineContext * GLContext = NULL;

static int init_glfunc(void)
{
#ifndef STATIC_OPENGL
	/* define the GLFuncInitList */
#define INIT_GL_FUNC_LIST
#include "gl_funcs.h"
#undef INIT_GL_FUNC_LIST
	struct glfunc_init_item * item = &GLFuncInitList[0];
	while (item->name != NULL) {
		*(item->func) = (void*)GLGetProcAddress(item->name);
		if (*item->func == NULL)
			WARNING(OPENGL, "gl function %s not found\n", item->name);
		item ++;
	}
#endif
}

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
	init_glfunc();


	TRACE(OPENGL, "Vendor: %s\n", glGetString(GL_VENDOR));

	return &gl_context->base;
}

void EngineCloseWindow(struct VideoEngineContext * context)
{
	return;
}

