/* 
 * utils_png.h
 * by WN @ Jul. 05, 2009
 */

#ifndef UTILS_PNG_H
#define UTILS_PNG_H

#include <common/utils.h>
__BEGIN_DECLS
/* 
 * write `buffer' to `filename'. w/h is the size of buffer. 
 */
void
write_to_pngfile_rgb(char * filename, uint8_t * buffer,
		int w, int h) THROWS(CONTINUE, FATAL);

/* 
 * same as write_to_pngfile_rgb, except buffer format is rgba
 */
void
write_to_pngfile_rgba(char * filename, uint8_t * buffer,
		int w, int h) THROWS(CONTINUE, FATAL);

void *
read_from_pngfile(char * filename,
		void * (*alloc)(int w, int h, int bpp, void ** datap))
	THROWS(RESOURCE_LOST, FATAL);

struct bitmap;
struct bitmap *
read_bitmap_from_pngfile(char * filename);

__END_DECLS
#endif

