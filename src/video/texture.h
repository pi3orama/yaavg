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
	TEXTURE_HELPER,
};

struct texture {
	GC_TAG;
	struct cleanup cleanup;
	/* texture can be part of the bitmap */
	struct rectangle rect;
	enum texture_type type;
	/* should this texture be pinned into system memory? */
	bool_t pinned;
	/* if no coorsponding bitmap, bitmap_res_id should be set to 0 */
	res_id_t bitmap_res_id;
	/* sometime the bitmap's data field is useful.
	 * this field should be set to NULL at most of the time.
	 * the refcounter of bitmap should be properly adjusted */
	struct bitmap * bitmap;
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
tex_create(res_id_t bitmap_res_id, struct rectangle rect, void * args)
	THROWS(INTERNAL_ERROR);

#define TEX_CLEANUP(t)	(t)->cleanup.function(&((t)->cleanup))


__END_DECLS
#endif

// vim:tabstop=4:shiftwidth=4

