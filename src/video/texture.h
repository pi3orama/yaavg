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
	/* if refcount > 0, don't actulally delete this texture*/
	int ref_count;
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

#define TEX_CLEANUP(t)		CLEANUP(&((t)->cleanup))

#define TEX_GRAB(t)		do	{		\
	if (((t)->ref_count) == 0)		\
		remove_cleanup(&(t)->cleanup);	\
	(t)->ref_count ++;		\
} while(0)

#define TEX_RELEASE(t)		do	{		\
	if (--((t)->ref_count) == 0)			\
		TEX_CLEANUP(t);					\
} while(0)



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

