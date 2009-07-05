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
#include <common/utils_png.h>
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

	char * filename = (char*)((uint32_t)resid);
	bitmap = read_bitmap_from_pngfile(filename);
	assert(bitmap != NULL);
	bitmap->base.id = resid;
	return bitmap;
}

// vim:tabstop=4:shiftwidth=4

