/* 
 * modelview.h
 * by WN @ Jun. 21, 2009
 */
#ifndef __MODELVIEW_H
#define __MODELVIEW_H

#include <common/defs.h>
#include <common/debug.h>
#include <math/matrix.h>

__BEGIN_DECLS

extern void
translate(mat4x4 * m, float x, float y, float z);

extern void
rotate(mat4x4 * m, float angle, float x, float y, float z);

extern void
scale(mat4x4 * m, float x, float y, float z);

__END_DECLS

#endif

