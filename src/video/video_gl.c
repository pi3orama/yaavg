/* 
 * video_gl.c
 * by WN @ Mar. 09, 2009
 */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>

#include <GL/gl.h>
#include <GL/glu.h>

#include <common/defs.h>
#include <common/debug.h>
#include <common/exception.h>
#include <common/utils.h>

#include <econfig/econfig.h>

#include <video/video_driver.h>
#include <video/video_gl.h>
#include <video/video.h>

struct gl_context * gl_ctx = NULL;

static void
init_gl_driver(void);

static void
init_glfunc(void);


static void __driver_close(struct cleanup * str);
static struct cleanup driver_cleanup_str = {
	.function = __driver_close,
	.list = {NULL, NULL},
};

static void
__driver_close(struct cleanup * str)
{
	assert(str == &driver_cleanup_str);
	remove_cleanup(str);
	if (gl_ctx != NULL) {
		gl_close();
		gl_ctx = NULL;
	}
	return;
}


void
driver_close(void)
{
	__driver_close(&driver_cleanup_str);
}


struct video_context *
driver_init(void)
{
	if (gl_ctx != NULL) {
		WARNING(OPENGL, "multi driver_init\n");
		return &gl_ctx->base;
	}
	gl_ctx = gl_init();
	if (gl_ctx == NULL) {
		/* We shouldn't be here! */
		ERROR(OPENGL, "gl_init failed\n");
		THROW(EXCEPTION_FATAL, "gl_init failed");
	} else {
		make_cleanup(&driver_cleanup_str);
		init_gl_driver();
		return &gl_ctx->base;
	}
}

void
driver_reinit(void)
{
	gl_reinit();
	init_gl_driver();
	return;
}



static void
init_glfunc(void)
{
#ifndef STATIC_OPENGL
	/* define the gl_func_init_list */
#define INIT_GL_FUNC_LIST
#include <video/gl_funcs.h>
#undef INIT_GL_FUNC_LIST
	struct glfunc_init_item * item = &gl_func_init_list[0];
	while (item->name != NULL) {
		*(item->func) = (void*)gl_get_proc_address(item->name);
		if (*item->func == NULL)
			WARNING(OPENGL, "gl function %s not found\n", item->name);
		item ++;
	}
#endif
}

static void
init_gl_driver(void)
{
	GLenum err;
	gl_ctx->base.driver_name = "OpenGL";

	/* Init OpenGL */
	/* first, get opengl func pointers */
	init_glfunc();

	gl_ctx->vendor     = glGetString(GL_VENDOR);
	gl_ctx->renderer   = glGetString(GL_RENDERER);
	gl_ctx->version    = glGetString(GL_VERSION);
	gl_ctx->extensions = glGetString(GL_EXTENSIONS);

	VERBOSE(OPENGL, "GL driver info:\n");
	VERBOSE(OPENGL, "Vendor     : %s\n", gl_ctx->vendor);
	VERBOSE(OPENGL, "Renderer   : %s\n", gl_ctx->renderer);
	VERBOSE(OPENGL, "Version    : %s\n", gl_ctx->version);
	VERBOSE(OPENGL, "Extensions : %s\n", gl_ctx->extensions);

	/* init opengl environment: */
	/* set view port and coordinator system */
	video_reshape(gl_ctx->base.width, gl_ctx->base.height);

	/* Set other OpenGL properties */
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_BLEND);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_TEXTURE_1D);
	glEnable(GL_TEXTURE_2D);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	err = glGetError();
	if (err != GL_NO_ERROR) {
		ERROR(OPENGL, "init_gl_driver failed, errno=%d\n", err);
		THROW(EXCEPTION_FATAL, "init_gl_driver failed");
	}
}

void
video_reshape(int w, int h)
{
	GLenum err;
	int vp_w, vp_h, vp_x, vp_y;

	gl_ctx->base.width = w;
	gl_ctx->base.height = h;

	vp_w = conf_get_integer("video.viewport.w", w);
	vp_h = conf_get_integer("video.viewport.h", h);

	if (vp_w > w) {
		WARNING(OPENGL, "viewport width %d larger than"
				" window width %d\n", vp_w, w);
		vp_w = w;
	}

	if (vp_h > h) {
		WARNING(OPENGL, "viewport height %d larger than"
				" window height %d\n", vp_h, h);
		vp_h = h;
	}

	vp_x = (w - vp_w) / 2;
	vp_y = (h - vp_h) / 2;

	gl_ctx->base.view_port.x = vp_x;
	gl_ctx->base.view_port.y = vp_y;
	gl_ctx->base.view_port.w = vp_w;
	gl_ctx->base.view_port.h = vp_h;

	TRACE(OPENGL, "Set viewport to (%d, %d, %d, %d)\n",
			vp_x, vp_y, vp_w, vp_h);
	glViewport(vp_x, vp_y, vp_w, vp_h);

	/* revert y axis */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 1, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	err = glGetError();
	if (err != GL_NO_ERROR) {
		ERROR(OPENGL, "video_reshape failed, errno=%d\n", err);
		THROW(EXCEPTION_FATAL, "video_reshape failed");
	}
}

static void
read_pixels(uint8_t * buffer, int x, int y, int w, int h, GLenum format)
{
	GLenum err;
	if (gl_ctx == NULL) {
		ERROR(OPENGL, "OpenGL has not been inited\n");
		THROW(EXCEPTION_FATAL, "OpenGL has not been inited");
	}

	if (x + w > gl_ctx->base.width) {
		WARNING(OPENGL, "Width (%d + %d) out of range (%d)\n",
				x, w, gl_ctx->base.width);
		THROW(EXCEPTION_CONTINUE, "Width out of range");
	}

	if (y + h > gl_ctx->base.height) {
		WARNING(OPENGL, "Height (%d + %d) out of range (%d)\n",
				y, h, gl_ctx->base.height);
		THROW(EXCEPTION_CONTINUE, "Height out of range");
	}

	glReadPixels(x, y, w, h, format, GL_UNSIGNED_BYTE, buffer);

	err = glGetError();
	if (err != GL_NO_ERROR) {
		WARNING(OPENGL, "ReadPixels failed, errno=%d\n", err);
		THROW(EXCEPTION_CONTINUE, "Read pixels failed\n");
	}
}

void
driver_read_pixels_rgb(uint8_t * buffer, struct view_port vp)
{
	read_pixels(buffer, vp.x, vp.y, vp.w, vp.h, GL_RGB);
}

void
driver_read_pixels_rgba(uint8_t * buffer, struct view_port vp)
{
	read_pixels(buffer, vp.x, vp.y, vp.w, vp.h, GL_RGBA);
}

void
driver_begin_frame(void)
{
	int err;

	static int reinited = 0;
	static tick_t last_time = 0;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	err = glGetError();
	if (err != GL_NO_ERROR) {
		ERROR(OPENGL, "driver_begin_frame failed, errno=%d\n", err);
		throw_reinit_exception(&reinited, &last_time,
				EXCEPTION_SUBSYS_RERUN, "frame error, try rerender",
				EXCEPTION_SUBSYS_SKIPFRAME, "still error, skip this frame",
				EXCEPTION_SUBSYS_REINIT, "still error, reinit subsystem",
				EXCEPTION_FATAL, "fatal error");
	}

	return;
}

void
driver_end_frame(void)
{
	int err;

	static int reinited = 0;
	static tick_t last_time = 0;

	err = glGetError();
	if (err != GL_NO_ERROR) {
		ERROR(OPENGL, "GLError, errno=%d\n", err);
		throw_reinit_exception(&reinited, &last_time,
				EXCEPTION_SUBSYS_RERUN, "frame error, try rerender",
				EXCEPTION_SUBSYS_SKIPFRAME, "still error, skip this frame",
				EXCEPTION_SUBSYS_REINIT, "still error, reinit subsystem",
				EXCEPTION_FATAL, "fatal error");
	}

	return;
}

void
gl_check_error(void)
{

	struct kvp {
		GLenum errno;
		const char * description;
	};

	static struct kvp tb[] = {
		{GL_INVALID_ENUM, "Enum argument out of range"},
		{GL_INVALID_VALUE,"Numeric argument out of range"},
		{GL_INVALID_OPERATION, "Operation illegal in current state"},
		{GL_INVALID_FRAMEBUFFER_OPERATION, "Framebuffer object is not complete"},
		{GL_STACK_OVERFLOW, "Command would cause a stack overflow"},
		{GL_STACK_UNDERFLOW, "Command would cause a stack underflow"},
		{GL_OUT_OF_MEMORY, "Not enough memory left to execute command"},
		{GL_TABLE_TOO_LARGE, "The specified table is too large"},
		/* 0 is resvred for GL_NO_ERROR at least in nvidia opengl */
		{0, "Unknown error"}
	};

	GLenum err;
	err = glGetError();
	if (err == GL_NO_ERROR)
		return;

	struct kvp * ptr = tb;
	while(ptr->errno != 0)
		if (ptr->errno == err)
			break;


	VERBOSE(OPENGL, "glGetError() returns %s (0x%X)\n", ptr->description, err);
	THROW_VAL(EXCEPTION_RENDER_ERROR, "OpenGL error", err);
	return;
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

// vim:tabstop=4:shiftwidth=4
