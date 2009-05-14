/* 
 * texture_gl.h
 *
 * texture OpenGL definition, for cmd use
 */

#ifndef TEXTURE_GL_H
#define TEXTURE_GL_H

#include <common/defs.h>
#include <common/exception.h>
#include <common/mm.h>
#include <common/geometry.h>

#include <resource/bitmap.h>

#include <video/texture.h>
#include <video/video_gl.h>
#include <stdint.h>

__BEGIN_DECLS

#define NR_HWTEX_LMT	(64)

struct texture_gl_params {
	/* flat texture is treated as bitmap, no hw texture is generated */
	bool_t flat;

	/* imm texture is load into hw mem immediately */
	bool_t imm;

	/* more important, more unlikely be free when hwmem shortage. */
	/* if importance larger than 100, the texture will never be removed */
	int importance;

	/* below are openGL specified options */

	GLenum target;	/* 1D or 2D. 3D is special. */
	GLenum mag_filter;
	GLenum min_filter;
	GLenum wrap_s;
	GLenum wrap_t;
	GLenum wrap_r;
};

#define TEXTURE_GL_PARAM_INIT {	\
	FALSE,						\
	FALSE,						\
	50,							\
	GL_TEXTURE_2D,				\
	GL_LINEAR,					\
	GL_LINEAR,					\
	GL_CLAMP_TO_EDGE,			\
	GL_CLAMP_TO_EDGE,			\
	GL_CLAMP_TO_EDGE,			\
}

#define TEXTURE_GL_PARAM(name) \
	struct texture_gl_params name = TEXTURE_GL_PARAM_INIT;

struct texture_gl {
	struct texture base;
	struct texture_gl_params params;
	/* gl textures is linked into a list, sorted by importance */
	struct list_head list;
	/* if texture too large, one need split the origin
	 * bitmap into small tiles. phy_bitmap is used for this.
	 * */
	void * phy_bitmap;

	/* if true, texture need to hold the bitmap */
	bool_t use_bitmap_data;

	/* below 2 arrays is used to store hw textures gened
	 * by glGenTextures. if nr_hwtexs are more than NR_HWTEX_LMT,
	 * hwtexs_extends is allocated and used, then hwtexts is useless. */
	GLuint _hwtexs_save[NR_HWTEX_LMT];
	GLuint * hwtexs;

	int nr_hwtexs;

	/* for opengl texture size profile */
	int occupied_hwmem;
};

#define TEXGL_BITMAP(t)	TEX_BITMAP(&((t)->base))
#define SET_TEXGL_BITMAP(t, b)	SET_TEX_BITMAP(&((t)->base), (b))

#define TEXGL_CLEANUP(t)	TEX_CLEANUP(&((t)->base))
#define TEXGL_SHRINK(t, p)	TEX_SHRINK(&((t)->base), (p))

__END_DECLS

#endif
// vim:tabstop=4:shiftwidth=4

