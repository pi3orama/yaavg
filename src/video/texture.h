/* 
 * texture.h
 * definition of texture
 * by WN @ Mar. 21, 2009
 */

#include <common/defs.h>
#include <common/exception.h>

#include <resource/bitmap.h>

__BEGIN_DECLS


struct texture {
	int w, h;
	uint8_t * data;
};

__END_DECLS

// vim:tabstop=4:shiftwidth=4

