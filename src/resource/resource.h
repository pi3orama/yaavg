/* 
 * resource.h
 * by WN @ Mar. 22, 2009
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include <common/defs.h>
#include <common/exception.h>
#include <common/debug.h>
#include <common/list.h>

struct resource {
	char * id;
	/* garbage collection utils */
	struct list_head list;
	struct cleanup cleanup;
};

enum bitmap_fotmat {
	BITMAP_RGA,
	BITMAP_RGBA,
	BITMAP_LUMINANCE,
};

struct bitmap {
	struct resource base;
	int w, h;
	uint8_t * data;
};

extern struct bitmap *
res_load_bitmap(const char * resid) THROWS(EXCEPTION_RESOURCE_LOST);

extern void
res_release_bitmap(struct bitmap * bitmap);

/* NOTICE: the param of pin should be a struct, because
 * when up level call pin, it wants to pin the data in a
 * valid object. Accept resid may cause resource layer
 * pin a object which hasn't be loaded. */
extern void
res_pin_bitmap(struct bitmap * bitmap);

extern void
res_put_bitmap(const char * resid);


#endif
// vim:tabstop=4:shiftwidth=4

