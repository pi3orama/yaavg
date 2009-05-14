/* 
 * geomertry.h
 * by WN @ May. 10, 2009
 */

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <common/defs.h>

__BEGIN_DECLS

struct point {
	int x, y;
};

#define POINT_FMT	"(%d, %d)"
#define POINT_ARG(p)	(p)->x, (p)->y

struct rectangle {
	int x, y, w, h;
};
#define RECT_FMT	"(%d, %d)++(%d, %d)"
#define RECT_ARG(r)	(r)->x, (r)->y, (r)->w, (r)->h

__END_DECLS
#endif
// vim:tabstop=4:shiftwidth=4


