/* 
 * video_gl.c
 * by WN @ Mar. 09, 2009
 */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>

#include <common/defs.h>
#include <common/debug.h>
#include <common/exception.h>
#include <common/utils.h>

#include <econfig/econfig.h>

#include <video/video_engine.h>
/* video_gl.h includes gl.h */
#include <video/video_gl.h>
#include <video/video.h>

#include <stdarg.h>
#include <regex.h>

#ifdef HAVE_ALLOCA_H
# include <alloca.h>
#endif

#ifdef VIDEO_OPENGL_ENGINE

struct gl_context * gl_ctx = NULL;

static void
init_gl_engine(void);

static void
init_glfunc(void);

static const char *
glerrno_to_desc(GLenum errno)
{
	struct kvp {
		GLenum errno;
		const char * desc;
	};

	static struct kvp tb[] = {
		{GL_INVALID_ENUM, "Enum argument out of range"},
		{GL_INVALID_VALUE, "Numeric argument out of range"},
		{GL_INVALID_OPERATION, "Operation illegal in current state"},
		{GL_INVALID_FRAMEBUFFER_OPERATION, "Framebuffer object is not complete"},
		{GL_STACK_OVERFLOW, "Command would cause a stack overflow"},
		{GL_STACK_UNDERFLOW, "Command would cause a stack underflow"},
		{GL_OUT_OF_MEMORY, "Not enough memory left to execute command"},
		{GL_TABLE_TOO_LARGE, "The specified table is too large"},
		/* 0 is resvred for GL_NO_ERROR at least in nvidia opengl */
		{0, "Unknown error"}
	};

	struct kvp * ptr = tb;
	while(ptr->errno != 0) {
		if (ptr->errno == errno)
			break;
		ptr ++;
	}
	return ptr->desc;
}

static void __engine_close(struct cleanup * str);
static struct cleanup engine_cleanup_str = {
	.function = __engine_close,
	.list = {NULL, NULL},
};

static void
__engine_close(struct cleanup * str)
{
	assert(str == &engine_cleanup_str);
	remove_cleanup(str);
	if (gl_ctx != NULL) {
		gl_close();
		gl_ctx = NULL;
	}
	return;
}


void
engine_close(void)
{
	__engine_close(&engine_cleanup_str);
}


struct video_context *
engine_init(void)
{
	if (gl_ctx != NULL) {
		WARNING(OPENGL, "multi engine_init\n");
		return &gl_ctx->base;
	}
	gl_ctx = gl_init();
	if (gl_ctx == NULL) {
		/* We shouldn't be here! */
		ERROR(OPENGL, "gl_init failed\n");
		THROW(EXCEPTION_FATAL, "gl_init failed");
	} else {
		make_cleanup(&engine_cleanup_str);
		init_gl_engine();
		return &gl_ctx->base;
	}
}

static LIST_HEAD(reinit_list);

void
video_hook_reinit(struct reinit_hook * hook)
{
	assert(hook != NULL);
	if (list_head_deleted(&hook->list))
		list_add(&hook->list, &reinit_list);
	else if (list_empty(&hook->list))
		list_add(&hook->list, &reinit_list);
	return;
}

void
video_unhook_reinit(struct reinit_hook * hook)
{
	assert(hook != NULL);
	if (list_empty(&hook->list))
		return;
	if (list_head_deleted(&hook->list))
		return;
	list_del(&hook->list);
	return;
}

static void
exec_reinit_hooks(void)
{
	struct reinit_hook * pos, * n;
	TRACE(OPENGL, "OpenGL reinit, exec hooks\n");
	list_for_each_entry_safe(pos, n, &reinit_list, list) {
		pos->fn(pos);
	}
	return;
}

void
engine_reinit(void)
{
	gl_reinit();
	init_gl_engine();
	exec_reinit_hooks();
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
		TRACE(OPENGL, "init gl func %s\n", item->name);
		*(item->func) = (void*)gl_get_proc_address(item->name);
		if (*item->func == NULL)
			WARNING(OPENGL, "gl function %s not found\n", item->name);
		item ++;
	}
#endif
}

/* There may be many candidate string for an extension */
static bool_t
check_extension(const char * conf_key, ...)
{
	va_list args;
	const char * f, * e;

	e = (const char *)(gl_ctx->extensions);
	assert(e != NULL);

	bool_t retval = FALSE;

	if (conf_key != NULL) {
		retval = conf_get_bool(conf_key, TRUE);
		/* conf set this feature to FALSE */
		if (!retval)
			return FALSE;
	}

	retval = FALSE;

	va_start(args, conf_key);

	f = va_arg(args, const char *);
	while(f != NULL) {
		assert(strlen(f) < 64);

		TRACE(OPENGL, "check for feature %s\n", f);
		if (match_word(f, e)) {
			TRACE(OPENGL, "found feature %s\n", f);
			va_end(args);
			return TRUE;
		}
		f = va_arg(args, const char *);
	}

	va_end(args);
	return FALSE;
}

#ifndef STATIC_OPENGL
struct func_table {
	void ** k;
	void * v;
};
static void
replace_func_ptr(struct func_table * t)
{
	while(t->k != NULL) {
		if (*(t->k) == NULL)
			*(t->k) = t->v;
		t++;
	}
}
#endif

#define MKGLVER(a, b)	((a) * 100 + (b))

static void
check_features(void)
{
	assert(gl_ctx != NULL);

	gl_ctx->max_texture_size = gl_getint(GL_MAX_TEXTURE_SIZE,
			"video.opengl.texture.maxsize",
			0, min0);
	VERBOSE(OPENGL, "OpenGL max texture size = %d\n", gl_ctx->max_texture_size);
	if (!is_power_of_two(gl_ctx->max_texture_size)) {
		WARNING(OPENGL, "Max texture size %d not power of to. Check your configuration\n",
				gl_ctx->max_texture_size);

		int n;
		n = pow2rounddown(gl_ctx->max_texture_size);
		gl_ctx->max_texture_size = n;
		WARNING(OPENGL, "fall back to %d\n", gl_ctx->max_texture_size);
	}

#define VERBOSE_FEATURE(name, exp) do {\
	if (exp)	\
		VERBOSE(OPENGL, name " is enabled\n");	\
	else		\
		VERBOSE(OPENGL, name " is disabled\n");	\
	} while(0)

	gl_ctx->texture_NPOT = check_extension("video.opengl.texture.enableNPOT",
			"GL_ARB_texture_non_power_of_two",
			NULL);
	VERBOSE_FEATURE("NPOT texture", gl_ctx->texture_NPOT);

	gl_ctx->texture_RECT = check_extension("video.opengl.texture.enableRECT",
			"GL_ARB_texture_rectangle",
			"GL_EXT_texture_rectangle",
			"GL_NV_texture_rectangle",
			NULL);
	VERBOSE_FEATURE("RECT texture", gl_ctx->texture_RECT);

	gl_ctx->texture_COMPRESSION = check_extension("video.opengl.texture.enableCOMPRESSION",
		"GL_ARB_texture_compression",
		NULL);
	VERBOSE_FEATURE("texture compression", gl_ctx->texture_COMPRESSION);

#ifndef STATIC_OPENGL
	{
	struct func_table t[] = {
		{(void**)&glCompressedTexImage3D, 		(void*)glCompressedTexImage3DARB},
		{(void**)&glCompressedTexImage2D, 		(void*)glCompressedTexImage2DARB},
		{(void**)&glCompressedTexImage1D, 		(void*)glCompressedTexImage1DARB},
		{(void**)&glCompressedTexSubImage3D, 	(void*)glCompressedTexSubImage3DARB},
		{(void**)&glCompressedTexSubImage2D, 	(void*)glCompressedTexSubImage2DARB},
		{(void**)&glCompressedTexSubImage1D, 	(void*)glCompressedTexSubImage1DARB},
		{(void**)&glGetCompressedTexImage, 		(void*)glGetCompressedTexImageARB},
		{NULL, NULL},
	};
	replace_func_ptr(t);
	}
#endif

	/* check compactability */
	if (gl_ctx->full_version >= MKGLVER(3, 1)) {
		/* check for GL_ARB_compatibility  */
		if (!check_extension(NULL,
					"GL_ARB_compatibility",
					NULL));
		WARNING(OPENGL,
			   	"You have a very high version of OpenGL, and doesn't have GL_ARB_compatibility. Good luck...\n");
	}

#undef VERBOSE_FEATURE
}

/* set major and minor version number */
static void
update_version(void)
{
	regex_t reg;
	regmatch_t rm[3];
	int err, maj, min;
	err = regcomp(&reg, "^\\([0-9]\\+\\)\\.\\([0-9]\\+\\)", 0);
	assert(err == 0);
	err = regexec(&reg, (const char *)gl_ctx->version, 3, rm, 0);
	if (err != 0) {
		FATAL(OPENGL, "Invalid opengl version string: %s", gl_ctx->version);
		THROW(EXCEPTION_FATAL, "Invalid OpenGL library");
	}

	char s[16];
	memset(s, 0, sizeof(s));

	if (rm[0].rm_eo >= 15) {
		FATAL(OPENGL, "Strange opengl version string: %s", gl_ctx->version);
		THROW(EXCEPTION_FATAL, "Invalid OpenGL library");
	}

	strncpy(s, (const char *)gl_ctx->version, rm[0].rm_eo);
	s[rm[0].rm_eo] = '\0';
	sscanf(s, "%d.%d", &maj, &min);

	TRACE(OPENGL, "OpenGL API version: %d.%d\n", maj, min);
	gl_ctx->major_version = maj;
	gl_ctx->minor_version = min;
	gl_ctx->full_version = MKGLVER(maj, min);
	return;
}


static void
init_gl_engine(void)
{
	gl_ctx->base.engine_name = "OpenGL";

	/* Init OpenGL */
	/* first, get opengl func pointers */
	init_glfunc();

	gl_ctx->vendor     = glGetString(GL_VENDOR);
	gl_ctx->renderer   = glGetString(GL_RENDERER);
	gl_ctx->version    = glGetString(GL_VERSION);
	update_version();
	gl_ctx->extensions = glGetString(GL_EXTENSIONS);

	VERBOSE(OPENGL, "GL engine info:\n");
	VERBOSE(OPENGL, "Vendor     : %s\n", gl_ctx->vendor);
	VERBOSE(OPENGL, "Renderer   : %s\n", gl_ctx->renderer);
	VERBOSE(OPENGL, "Version    : %s\n", gl_ctx->version);
	VERBOSE(OPENGL, "Extensions : %s\n", gl_ctx->extensions);
	/* Antialiasing settings */
	int x;
	glGetIntegerv(GL_SAMPLES, &x);
	VERBOSE(OPENGL, "Samples : %d\n", x);
	glGetIntegerv(GL_SAMPLE_BUFFERS, &x);
	VERBOSE(OPENGL, "Sample buffers : %d\n", x);
	if (x > 0)
		glEnable(GL_MULTISAMPLE);
	GL_POP_ERROR();

	/* init opengl environment: */

	check_features();

	/* set view port and coordinator system */
	video_reshape(gl_ctx->base.width, gl_ctx->base.height);

	/* Set other OpenGL properties */
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_1D);
	glEnable(GL_TEXTURE_2D);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	gl_check_error();

	/* set all avaliable hints to NISTEST */
	/* FIXME */
	/* deprecated in 3.0 */
	if (gl_ctx->full_version < MKGLVER(3, 0))
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	if (gl_ctx->full_version < MKGLVER(3, 0))
		glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	if (gl_ctx->full_version < MKGLVER(3, 0))
		glHint(GL_FOG_HINT, GL_NICEST);

	if (gl_ctx->full_version >= MKGLVER(1, 3))
		glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);
	if (gl_ctx->full_version >= MKGLVER(1, 4))
			if (gl_ctx->full_version < MKGLVER(3, 0))
				glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	if (gl_ctx->full_version >= MKGLVER(2, 0))
		glHint(GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST);

	GL_POP_ERROR();
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
	glOrtho(0, 1, 0, 1, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();


	err = glGetError();
	if (err != GL_NO_ERROR) {
		ERROR(OPENGL, "video_reshape failed: \"%s\" (0x%x)\n",
				glerrno_to_desc(err), err);
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
		WARNING(OPENGL, "ReadPixels failed: \"%s\" (0x%x)\n",
				glerrno_to_desc(err), err);
		THROW(EXCEPTION_CONTINUE, "Read pixels failed\n");
	}
}

void
engine_read_pixels_rgb(uint8_t * buffer, struct view_port vp)
{
	read_pixels(buffer, vp.x, vp.y, vp.w, vp.h, GL_RGB);
}

void
engine_read_pixels_rgba(uint8_t * buffer, struct view_port vp)
{
	read_pixels(buffer, vp.x, vp.y, vp.w, vp.h, GL_RGBA);
}

void
engine_begin_frame(void)
{
	int err;

	static int reinited = 0;
	static tick_t last_time = 0;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	err = glGetError();
	if (err != GL_NO_ERROR) {
		ERROR(OPENGL, "engine_begin_frame failed: \"%s\" (0x%x)\n",
				glerrno_to_desc(err), err);
		throw_reinit_exception(&reinited, &last_time,
				EXCEPTION_SUBSYS_RERUN, "frame error, try rerender",
				EXCEPTION_SUBSYS_SKIPFRAME, "still error, skip this frame",
				EXCEPTION_SUBSYS_REINIT, "still error, reinit subsystem",
				EXCEPTION_FATAL, "fatal error");
	}

	return;
}

void
engine_end_frame(void)
{
	int err;

	static int reinited = 0;
	static tick_t last_time = 0;

	err = glGetError();
	if (err != GL_NO_ERROR) {
		ERROR(OPENGL, "OpenGL error: \"%s\" (0x%x)\n",
				glerrno_to_desc(err), err);
		throw_reinit_exception(&reinited, &last_time,
				EXCEPTION_SUBSYS_RERUN, "frame error, try rerender",
				EXCEPTION_SUBSYS_SKIPFRAME, "still error, skip this frame",
				EXCEPTION_SUBSYS_REINIT, "still error, reinit subsystem",
				EXCEPTION_FATAL, "fatal error");
	}

	return;
}

void
#ifdef YAAVG_DEBUG_OFF
gl_check_error_nodebug(void)
#else
gl_check_error_debug(const char * file, const char * func, int line)
#endif
{

	GLenum err;
	err = glGetError();
	if (err == GL_NO_ERROR)
		return;
#ifndef YAAVG_DEBUG_OFF
	WARNING(OPENGL, "glGetError() returns \"%s\" (0x%x) at %s:%s:%d\n",
			glerrno_to_desc(err), err, file, func, line);
#else
	WARNING(OPENGL, "glGetError() returns \"%s\" (0x%x)\n",
			glerrno_to_desc(err), err);
#endif
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

#endif	/* VIDEO_OPENGL_ENGINE */

// vim:tabstop=4:shiftwidth=4
