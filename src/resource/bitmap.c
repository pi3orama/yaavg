/* 
 * bitmap.c:
 * by WN @ Mar. 27, 2009
 *
 * throw-away bitmap resource implentation
 */


#include <common/defs.h>
#include <common/exception.h>
#include <common/debug.h>
#include <common/list.h>

#include <resource/bitmap.h>

#include <assert.h>

static void
bitmap_cleanup(struct cleanup * cleanup)
{
	struct resource * base = container_of(cleanup,
			struct resource, cleanup);
	struct bitmap * bitmap = container_of(base,
			struct bitmap, base);
	/* Now, we only free once, because data and
	 * meta data are allocated once. */
	free(bitmap);
}



struct bitmap *
res_load_bitmap(resid_t resid)
{
	/* currently, we load bitmap from png file,
	 * but in the final system, one may cache
	 * the metadata of the resource. */
	return NULL;
}


void
res_release_bitmap(struct bitmap * bitmap)
{
	return;
}

void
res_pin_bitmap(struct bitmap * bitmap)
{
	WARNING(RESOURCE, "not implentmented\n");
	return;
}

void
res_put_bitmap(resid_t resid)
{
	WARNING(RESOURCE, "not implentmented\n");
	return;
}

// vim:tabstop=4:shiftwidth=4

