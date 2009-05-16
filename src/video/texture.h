/* 
 * texture.h
 * definition of texture
 * by WN @ Mar. 21, 2009
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#include <common/defs.h>
#include <common/exception.h>
#include <common/mm.h>
#include <common/geometry.h>

#include <resource/bitmap.h>
__BEGIN_DECLS

enum texture_type {
	/* normal texture */
	TEXTYPE_NORMAL,
	/* internal texture, no coorsponding bitmap */
	TEXTYPE_INTERNAL,
	/* helper texture, render redirection */
	TEXTYPE_HELPER,
};

struct texture_params {
	enum texture_type type;
	/* should this texture be pinned into system memory? */
	bool_t pin;
};

#define TEXTURE_PARAM_INIT {	\
	TEXTYPE_NORMAL,				\
	FALSE,						\
}

#define TEXTURE_PARAM(name) \
	struct texture_params name = TEXTURE_PARAM_INIT;

struct texture {
	GC_TAG;
	struct cleanup cleanup;
	/* if no coorsponding bitmap, bitmap_res_id should be set to 0 */
	res_id_t bitmap_res_id;
	/* sometime the bitmap's data field is useful.
	 * this field should be set to NULL at most of the time.
	 * the refcounter of bitmap should be properly adjusted */
	struct bitmap * bitmap;
	/* texture can be part of the bitmap */
	struct rectangle rect;
	struct texture_params params;
};

#define TEX_BITMAP(t)	(t)->bitmap
#define SET_TEX_BITMAP(t, b)	do {(t)->bitmap = (b);}while(0)

/* args is implementation specified. For OpenGL
 * texture, args contain:
 * 1. bool_t flat: if true, 'texture' is write to framebuffer using
 *    bitmap directly, and cannot take any special effects. default
 *    is false. No hardware texture is created.
 * 2. bool_t imm: if true, this texture is load into hardware
 *    immediately;
 * 3. int importance: when hardware memory shortage, which one
 *    be unloaded first? */
extern struct texture *
tex_create(res_id_t bitmap_res_id,
		struct rectangle rect,
		struct texture_params * params,
		void * args) THROWS(all);

#define TEX_CLEANUP(t)		CLEANUP(&((t)->cleanup))
#define TEX_SHRINK(t, p)	GC_SHRINK((&(t)->__gc_tag), (p))

extern void
tex_common_init(struct texture * tex,
		res_id_t bitmap_res_id,
		struct rectangle rect,
		struct texture_params * params);

extern void
tex_common_cleanup(struct texture * tex);

__END_DECLS
#endif

// vim:tabstop=4:shiftwidth=4

