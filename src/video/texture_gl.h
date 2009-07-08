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
	/* save a dummy bitmap, sometime texture need its layout
	 * to do some computation */
	struct bitmap * dummy_bitmap;
	struct bitmap __dummy_bitmap;
	struct reinit_hook reinit_hook;
	struct texture_gl_params gl_params;
	/* gl textures is linked into a list, sorted by importance */
	struct list_head list;
	/* if texture too large, one need split the origin
	 * bitmap into small tiles. phy_bitmap is used for this.
	 * */
	uint8_t * phy_bitmap;

	/* if true, texture need to hold the bitmap */
	bool_t use_bitmap_data;

	/* below 2 arrays is used to store hw textures gened
	 * by glGenTextures. if nr_hwtexs are more than NR_HWTEX_LMT,
	 * hwtexs_extends is allocated and used, then __hwtexs_save is useless. */
	GLuint _hwtexs_save[NR_HWTEX_LMT];
	GLuint * hwtexs;

	/* tex_data_format is determined by bitmap->format */
	GLenum tex_data_format;
	int nr_hwtexs;
	int nw, nh;

	enum {
		TEXGL_NPOT,
		TEXGL_RECT,
		TEXGL_NORMAL,
	} internal_type;
	/* GL_TEXTURE_2D or GL_TEXTURE_RECTANGLE */
	GLenum texgl_target;
	int tile_w, tile_h;

	/* for opengl texture size profile */
	int occupied_hwmem;
};

#define TEXGL_BITMAP(t)	TEX_BITMAP(&((t)->base))
#define SET_TEXGL_BITMAP(t, b)	SET_TEX_BITMAP(&((t)->base), (b))

#define TEXGL_DUMMY_BITMAP(t)	({assert((t)->dummy_bitmap != NULL); (t)->dummy_bitmap;})

#define TEXGL_CLEANUP(t)	TEX_CLEANUP(&((t)->base))

#define TEXGL_GRAB(t)		TEX_GRAB(&(t)->base)
#define TEXGL_RELEASE(t)	TEX_RELEASE(&(t)->base)

extern struct texture_gl *
texgl_create(res_id_t bitmap_res_id, struct rectangle rect,
	struct texture_params * params,
	struct texture_gl_params * gl_params);


/* struct tex_point describe a transformation
 * from physical coordinator to texture coordinator.
 * px, py, pz is space coord, tx, ty is tex coord. 
 * for RECT texture, use itx and ity.
 * */
struct tex_point {
	float px, py, pz;
	union {
		struct {
			float tx, ty;
		} f;
		struct {
			int itx, ity;
		} i;
	} u;
};

/* 
 * texgl_fillmesh:
 *
 * for each hwtex, create a point list. OpenGL can
 * use those lists to render the whole texture.
 * 
 * i_points is a struct tex_point array, at least 3 elements,
 * at most 4 elements.
 * i_points use uniform texcoord, that is, the width and height
 * are both 1.0.
 *
 * if nr_ipoints == 4, then each points must attach to a texture corner,
 * the order is: (0,0), (1,0), (1,1), (0,1)
 *
 * if nr_ipoints == 3, fillmesh will still form a whole texture mesh.
 *
 * o_points is output tex_point array. It shall has at least(and most)
 * tex->nr_hwtexs * 4 elements, each element describe a
 * point.
 */
extern void
texgl_fillmesh(struct texture_gl * tex,
		struct tex_point * o_points,
		int nr_ipoints,
		struct tex_point * i_points);

#define texgl_destroy(t)	TEXGL_CLEANUP(t)

__END_DECLS

#endif
// vim:tabstop=4:shiftwidth=4

