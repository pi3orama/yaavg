/* 
 * by WN @ Feb 06, 2009
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

#include <video/engine_driver.h>
#include <video/engine_gl.h>
#include <video/engine.h>

struct GLContext * GLCtx = NULL;

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

static int
init_driver(void)
{
	GLCtx->base.driver_name = "OpenGL";

	/* Init OpenGL */
	/* first, get opengl func pointer */
	init_glfunc();

	/* Set opengl context */
	GLCtx->vendor     = glGetString(GL_VENDOR);
	GLCtx->renderer   = glGetString(GL_RENDERER);
	GLCtx->version    = glGetString(GL_VERSION);
	GLCtx->extensions = glGetString(GL_EXTENSIONS);

	VERBOSE(OPENGL, "GL driver info:\n");
	VERBOSE(OPENGL, "Vendor     : %s\n", GLCtx->vendor);
	VERBOSE(OPENGL, "Renderer   : %s\n", GLCtx->renderer);
	VERBOSE(OPENGL, "Version    : %s\n", GLCtx->version);
	VERBOSE(OPENGL, "Extensions : %s\n", GLCtx->extensions);

	/* init opengl environment: */
	/* set view port and coordinator system */
	if (VideoReshape(GLCtx->base.width, GLCtx->base.height)) {
		ERROR(OPENGL, "Reshape failed\n");
		return -1;
	}

	/* Set other OpenGL properties */
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_BLEND);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_TEXTURE_1D);
	glEnable(GL_TEXTURE_2D);
	return 0;
}

struct VideoContext *
DriverOpenWindow(void)
{
	int w, h, err;

	GLCtx = GLOpenWindow();
	if (GLCtx == NULL) {
		WARNING(OPENGL, "Open Window failed\n");
		return NULL;
	}

	err = init_driver();
	if (err != 0) {
		WARNING(OPENGL, "Init driver failed\n");
		return NULL;
	}

	return &GLCtx->base;
}

void
DriverReopenWindow(struct VideoContext * ctx)
{
	int err;
	if (ctx != &GLCtx->base) {
		FATAL(OPENGL, "ctx %p is not the one previous return(%p)!",
				ctx, &GLCtx->base);
		exit(-1);
	}
	GLReopenWindow(GLCtx);

	err = init_driver();
	if (err != 0) {
		FATAL(OPENGL, "init driver failed...\n");
		exit(-1);
	}
}

void
DriverClose(void)
{
	GLClose();
	GLCtx = NULL;
	return;
}

int VideoReshape(int w, int h)
{
	int err;
	GLCtx->base.width = w;
	GLCtx->base.height = h;
	glViewport(0, 0, w, h);
//	glViewport(0, 0, w, h);
//	glViewport(100, 100, w-200, h-200);

	/* revert y axis */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 1, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	err = glGetError();
	if (err != GL_NO_ERROR) {
		ERROR(OPENGL, "Set Coordinator failed, errno=%d\n", err);
		VideoClose();
		return -1;
	}
	return 0;
}



/* Implentmented in engine_gl_xxx */
#if 0
extern int
DriverInit(void);

extern void
VideoSwapBuffers(void);

extern void
VideoSetCaption(const char * caption);

extern void
VideoSetIcon(const icon_t icon);
#endif

