/* 
 * utils_png.c
 * by WN @ Mar. 15, 2009
 *
 * png related utils
 */

#include <png.h>

#include <config.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <common/exception.h>
#include <common/list.h>
#include <common/utils_png.h>
#include <common/mm.h>

#include <resource/bitmap.h>

static void
png_bitmap_cleanup(struct cleanup * pcleanup)
{
	remove_cleanup(pcleanup);

	struct bitmap * b;
	b = container_of(pcleanup, struct bitmap, base.cleanup);;
	dealloc_bitmap(b);
}

static bitmap_format_t formats[5] = {
	[1]	=	BITMAP_LUMINANCE,
	[2] =	BITMAP_LUMINANCE_ALPHA,
	[3] =	BITMAP_RGB,
	[4] =	BITMAP_RGBA,
};

static void *
png_alloc_bitmap(int w, int h, int bpp, void ** datap)
{
	bitmap_format_t f;
	if ((bpp < 1) || (bpp > 4))
		THROW(EXCEPTION_FATAL, "bitmap bpp unknown: %d", bpp);
	f = formats[bpp];

	struct bitmap * b;
	b = alloc_bitmap(w, h, f);
	assert(b != NULL);

	*datap = b->data;

	b->base.cleanup.function = png_bitmap_cleanup;

	/* build the bitmap */
	b->base.cleanup.function = png_bitmap_cleanup;
	make_cleanup(&b->base.cleanup);

	return b;
}

struct bitmap *
read_bitmap_from_pngfile(char * filename)
{
	struct bitmap * p;
	p = (struct bitmap*)read_from_pngfile(filename, png_alloc_bitmap);
	if (p == NULL)
		THROW(EXCEPTION_RESOURCE_LOST, "read bitmap from pngfile failed");

	return p;
}

// vim:tabstop=4:shiftwidth=4

