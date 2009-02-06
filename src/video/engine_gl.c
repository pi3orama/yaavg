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

static int init_glfunc(void)
{
#ifndef STATIC_OPENGL
	/* define the GLFuncInitList */
#define INIT_GL_FUNC_LIST
#include <video/gl_funcs.h>
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
	int w, h;


	gl_context = GLOpenWindow();
	if (gl_context == NULL) {
		WARNING(OPENGL, "Open Window failed\n");
		return NULL;
	}

	RListInit(&(gl_context->base.render_list), &gl_context->base);
	GLContext = gl_context;

	/* Init OpenGL */
	/* first, get opengl func pointer */
	init_glfunc();

	/* Set opengl context */
	GLContext->vendor     = glGetString(GL_VENDOR);
	GLContext->renderer   = glGetString(GL_RENDERER);
	GLContext->version    = glGetString(GL_VERSION);
	GLContext->extensions = glGetString(GL_EXTENSIONS);

	VERBOSE(OPENGL, "GL driver info:\n");
	VERBOSE(OPENGL, "Vendor     : %s\n", GLContext->vendor);
	VERBOSE(OPENGL, "Renderer   : %s\n", GLContext->renderer);
	VERBOSE(OPENGL, "Version    : %s\n", GLContext->version);
	VERBOSE(OPENGL, "Extensions : %s\n", GLContext->extensions);

	/* init opengl environment: */
	/* set view port and coordinator system */
	if (EngineReshape(&GLContext->base, GLContext->base.width, GLContext->base.height)) {
		GLPlatformClose(GLContext);
		return NULL;
	}

	/* Set other OpenGL properties */
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_BLEND);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_TEXTURE_1D);
	glEnable(GL_TEXTURE_2D);

	return &gl_context->base;
}

int EngineReshape(struct VideoEngineContext * ctx, int w, int h)
{
	int err;
	GLContext->base.width = w;
	GLContext->base.height = h;
	glViewport(0, 0, w, h);

	/* revert y axis */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 1, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	err = glGetError();
	if (err != GL_NO_ERROR) {
		ERROR(OPENGL, "Set Coordinator failed, errno=%d\n", err);
		EngineClose(&GLContext->base);
		return -1;
	}
	return 0;
}

void EngineCloseWindow(struct VideoEngineContext * context)
{
	EngineClose(context);
	return;
}

void EngineSwapBuffers(struct VideoEngineContext * context)
{
	GLSwapBuffers(GLContext);
	return;
}

void EngineSetCaption(struct VideoEngineContext * contest, const char * caption)
{
	WMSetCaption(caption);
}

