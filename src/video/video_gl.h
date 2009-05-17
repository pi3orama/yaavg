/* 
 * video_gl.h
 * by WN @ Mar. 9, 2009
 */

#ifndef VIDEO_GL_H
#define VIDEO_GL_H

#include <common/defs.h>
#include <video/video.h>
#include <video/video_driver.h>

/* Below is opengl functions definitions */
/* define struct gl_funcs  */

/* gl_funcs includes gl.h  */
#include <video/gl_funcs.h>

__BEGIN_DECLS

struct gl_context {
	struct video_context base;
	const GLubyte * vendor;
	const GLubyte * renderer;
	const GLubyte * version;
	/* a very long string */
	const GLubyte * extensions;
	/* Which platform we used? SDL or GLX or WGL... */
	const char *platform;


	/* belong is for dynamic gl use: func pointers */
	struct gl_funcs gl_funcs;

	int max_texture_size;
#define gl_max_texture_size()	(gl_ctx->max_texture_size)
	bool_t texture_NPOT;
#define gl_tex_NPOT_enabled()	(gl_ctx->texture_NPOT)
	bool_t texture_RECT;
#define gl_tex_RECT_enabled()	(gl_ctx->texture_NPOT)
	bool_t texture_COMPRESSION;
#define gl_tex_compression_enabled()	(gl_ctx->texture_COMPRESSION)
};

extern struct gl_context * gl_ctx;
#define GLOPS	(&gl_ctx->gl_funcs)
#define gl_inited()	(gl_ctx != NULL)

extern void *
gl_get_proc_address(const char * name);

extern struct gl_context *
gl_init(void);

extern void
gl_reinit(void);

/* Like video_close and driver_close, we don't really need a gl_close */
extern void
gl_close(void);


/* for OpenGL command use */
extern void
gl_check_error(void) THROWS(all);

#define GL_POP_ERROR()	NOTHROW(gl_check_error)

/* OpenGL definition fixup */
#ifndef GL_INVALID_FRAMEBUFFER_OPERATION
# ifdef GL_INVALID_FRAMEBUFFER_OPERATION_EXT
#  define GL_INVALID_FRAMEBUFFER_OPERATION GL_INVALID_FRAMEBUFFER_OPERATION_EXT
# endif
#endif

/* OpenGL feature check */
#define gl_getint(e, s, d, op) ({		\
		int vala, valb;			\
		vala = conf_get_integer(s, d);	\
		glGetIntegerv(e, &valb);	\
	/* here: the order of vala, valb is important */\
		op(vala, valb);			\
			})

__END_DECLS

#endif

