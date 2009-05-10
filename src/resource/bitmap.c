/* 
 * bitmap.c:
 * by WN @ Mar. 27, 2009
 *
 * throw-away bitmap resource implentation
 */


#include <common/defs.h>
#include <common/exception.h>
#include <common/debug.h>
#include <common/utils.h>
#include <common/list.h>

#include <resource/bitmap.h>

#include <assert.h>


struct bitmap *
res_load_bitmap(res_id_t resid)
{
	/* currently, we load bitmap from png file,
	 * but in the final system, one may cache
	 * the metadata of the resource. */
	struct bitmap * bitmap;

	/* search from res pool first */
	struct resource * res = res_search(resid, RES_BITMAP);
	if (res != NULL) {
		return container_of(res, struct bitmap, base);
	}

	char * filename = (char*)((uint32_t)resid);
	bitmap = read_from_pngfile(filename);
	assert(bitmap != NULL);
	bitmap->base.id = resid;
	RES_BIRTH(&bitmap->base);
	return bitmap;
}

void
bitmap_shrink(struct gc_tag * tag, enum gc_power p)
{
	struct bitmap * b;
	b = (struct bitmap *)tag->ptr;
	if (b->base.ref_count <= 0) {
		RES_DIE(&b->base);
		BITMAP_CLEANUP(b);
		return;
	}

	if (p >= GC_DESTROY) {
		WARNING(MEMORY, "bitmap %lx destroied while refcount > 0\n",
				b->base.ref_count);
		RES_DIE(&b->base);
		BITMAP_CLEANUP(b);
		return;
	}

	return;
}

// vim:tabstop=4:shiftwidth=4

