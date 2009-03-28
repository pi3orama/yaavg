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
res_load_bitmap(resid_t resid)
{
	/* currently, we load bitmap from png file,
	 * but in the final system, one may cache
	 * the metadata of the resource. */
	struct bitmap * bitmap;
	char * filename = (char*)((uint32_t)resid);
	bitmap = read_from_pngfile(filename);
	assert(bitmap != NULL);
	bitmap->base.id = resid;
	bitmap->base.pin_count = 0;
	return bitmap;
}


void
res_release_bitmap(struct bitmap * bitmap)
{
	if (bitmap->base.pin_count <= 0)
		bitmap->base.cleanup.function(&bitmap->base.cleanup);
	else
		WARNING(SYSTEM, "won't release bitmap %p until exit\n", bitmap);
	return;
}

void
res_pin_bitmap(struct bitmap * bitmap)
{
	WARNING(RESOURCE, "not implentmented\n");
	bitmap->base.pin_count ++;
	return;
}

void
res_put_bitmap(resid_t resid)
{
	WARNING(RESOURCE, "not implentmented\n");
	return;
}

// vim:tabstop=4:shiftwidth=4

