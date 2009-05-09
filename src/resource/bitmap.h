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
#include <common/mm.h>

#include <resource/resource.h>

#include <stdint.h>

typedef enum _bitmap_format_t {
	BITMAP_RGB = 3,
	BITMAP_RGBA = 4,
	BITMAP_LUMINANCE = 1,
	BITMAP_LUMINANCE_ALPHA = 2,
} bitmap_format_t;

struct bitmap {
	struct resource base;
	int w, h;
	bitmap_format_t format;
	uint8_t data[0];
};

#define BITMAP_CLEANUP(b) \
	do {	\
		RES_CLEANUP(&((b)->base)); \
	} while(0)

static inline int
bitmap_data_size(struct bitmap * s)
{
	return s->w * s->h * s->format;
}

extern struct bitmap *
res_load_bitmap(res_id_t id) THROWS(EXCEPTION_RESOURCE_LOST);

extern void
bitmap_shrink(struct gc_tag * tag, enum gc_power p);

static inline struct bitmap *
alloc_bitmap(int width, int height, bitmap_format_t format)
{
	struct bitmap * bitmap;
	bitmap = gc_calloc(sizeof(*bitmap) + 
			width * height * format,
			offsetof(struct bitmap, base.gc_tag),
			0,
			bitmap_shrink,
			NULL);

	bitmap->w = width;
	bitmap->h = height;
	bitmap->format = format;

	bitmap->base.data_size = width * height * format;
	bitmap->base.ref_count = 1;
	bitmap->base.type = RES_BITMAP;
	bitmap->base.id = 0;

	return bitmap;
}

static inline void
dealloc_bitmap(struct bitmap * p)
{
	gc_free(&p->base.gc_tag);
}

#endif
// vim:tabstop=4:shiftwidth=4

