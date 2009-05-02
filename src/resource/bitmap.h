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

static inline int
bitmap_data_size(struct bitmap * s)
{
	return s->w * s->h * s->format;
}

extern struct bitmap *
res_load_bitmap(res_id_t id) THROWS(EXCEPTION_RESOURCE_LOST);

#endif
// vim:tabstop=4:shiftwidth=4

