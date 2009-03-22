/* 
 * resource.c
 * by WN @ Mar. 23, 2009
 *
 * A throw-away model for resource layer
 */

#include <resource/resource.h>


struct bitmap *
res_load_bitmap(const char * resid)
{
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
	return;
}

void
res_put_bitmap(const char * resid)
{
	return;
}


// vim:tabstop=4:shiftwidth=4


