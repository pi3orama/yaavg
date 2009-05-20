/* 
 * video_gl_glx.c
 * by WN @ May. 20, 2009
 */

#include <common/debug.h>
#include <common/exception.h>
#include <common/utils.h>
#include <econfig/econfig.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <video/video_driver.h>
#include <video/video_gl.h>

#ifdef OPENGL_GLX_DRIVER

static struct glx_context {
	struct gl_context base;

} _glx_ctx;

static struct glx_context * glx_ctx = NULL;

static gl_context *
gl_init(void)
{
	if (glx_ctx != NULL) {
		WARNING(OPENGL, "multi init OpenGL\n");
		return &glx_ctx->base;
	}

	init_glx();
	glx_ctx = &_glx_ctx;
	make_cleanup(&glx_cleanup_str);
	return &glx_ctx->base;
}

void
gl_reinit(void)
{
	THROW(EXCEPTION_FATAL, "glx reinit not implentmented");
}

void *
gl_get_proc_address(const char * name)
{
	THROW(EXCEPTION_FATAL, "glx get_proc_address not implentmented");
	return NULL;
}

void
video_swap_buffers(void)
{
	THROW(EXCEPTION_FATAL, "glx swap buffer not impled\n");
}

void
video_set_caption(const char * caption)
{
	THROW(EXCEPTION_FATAL, "glx set caption not implented\n");
	return;
}

void
video_set_icon(const icon_t icon)
{
	THROW(EXCEPTION_FATAL, "glx set icom not implented\n");
	return;
}

#endif	/* OPENGL_GLX_DRIVER */

// vim:tabstop=4:shiftwidth=4

