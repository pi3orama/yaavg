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
	GLPlatformClose(context);
	return 0;
}

struct VideoEngineContext * EngineOpenWindow(void)
{
	return NULL;
}

void EngineCloseWindow(struct VideoEngineContext * context)
{
	return;
}

