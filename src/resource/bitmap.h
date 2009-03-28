/* 
 * bitmap.h
 * by WN @ Mar. 22, 2009
 */

#ifndef RESOURCE_BITMAP_H
#define RESOURCE_BITMAP_H

#include <common/defs.h>
#include <common/exception.h>
#include <common/debug.h>
#include <common/list.h>

#include <resource/resource.h>

enum bitmap_format {
	BITMAP_RGB = 3,
	BITMAP_RGBA = 4,
	BITMAP_LUMINANCE = 1,
	BITMAP_LUMINANCE_ALPHA = 2,
};


/* NOTICE:
 * the sizeof operator take the padding bytes into consideration,
 * so if use uint32_t data[0], the padding bytes will add into
 * the size of whole structure.  */
struct bitmap {
	struct resource base;
	int w, h;
	enum bitmap_format format;
	uint8_t * data;
};

static inline int
bitmap_data_size(struct bitmap * s)
{
	return s->w * s->h * s->format;
}

extern struct bitmap *
res_load_bitmap(resid_t resid) THROWS(EXCEPTION_RESOURCE_LOST);

extern void
res_release_bitmap(struct bitmap * bitmap);

/* NOTICE: the param of pin should be a struct, because
 * when up level call pin, it wants to pin the data in a
 * valid object. Accept resid may cause resource layer
 * pin a object which hasn't been loaded. */
extern void
res_pin_bitmap(struct bitmap * bitmap);

/* 
 * when put, the structure may has already been released by
 * a res_release_bitmap, so it must accept a resid.
 */
extern void
res_put_bitmap(resid_t resid);
#endif
// vim:tabstop=4:shiftwidth=4

