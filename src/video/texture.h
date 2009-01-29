/* Wang Nan @ Jan 23, 2009 */
#ifndef TEXTURE_H
#define TEXTURE_H

#include <common/defs.h>

__BEGIN_DECLS


/* 
 * Different platform support different texture, for example, some OpenGL
 * doesn't support non-power-of-two texture, some only support 64x64 texture.
 * We define an abstract texture class to manage textures. We don't want to
 * bind our engine on OpenGL, therefore we need another OpenGL texture manager
 * to do some GL-specific works.
 */

/* This file define texture and texture manager. */

struct texture {
	const char * engine;	/* the name of engine, OpenGL or X11... */
	int width, height;
	int pined; /* texture can be pined into video memory, prevent manager swap it out */
	/* FIXME */
	/* define an enum */
	int format; /* This field define the format of data, rgba or rgb */
	void * data; /* the image data */
	void * private; /* texture manager's private struct */
};

__END_DECLS

#endif

