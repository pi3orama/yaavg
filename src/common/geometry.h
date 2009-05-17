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

#define RECT_SAME(r1, r2) \
	(((r1)->x == (r2)->x) && \
	 ((r1)->y == (r2)->y) && \
	 ((r1)->w == (r2)->w) && \
	 ((r1)->h == (r2)->h))

#define RECT_FMT	"(%d, %d)++(%d, %d)"
#define RECT_ARG(r)	(r)->x, (r)->y, (r)->w, (r)->h

#define is_square(r)	((r)->w == (r)->h)

#define lower_bound_val(x, b)	((x) > (b) ? (x) : (b))
#define lower_bound(x, b)		(x) = lower_bound_val(x, b)
#define upper_bound_val(x, b)	((x) < (b) ? (x) : (b))
#define upper_bound(x, b)		(x) = upper_bound_val(x, b)
#define bound_val(x, l, u)		lower_bound_val(upper_bound_val(x, u), l)
#define bound(x, l, u)			(x) = bound_val(x, l, u)

static inline void
geom_adjust_rect(struct rectangle * r)
{
	if (r->w < 0) {
		r->x = r->x + r->w;
		r->w = - r->w;
	}

	if (r->h < 0) {
		r->y = r->y + r->h;
		r->h = - r->h;
	}
}

static inline struct rectangle
geom_rect_cover(struct rectangle r1, struct rectangle r2)
{
	struct rectangle r;
	geom_adjust_rect(&r1);
	geom_adjust_rect(&r2);
	r.x = bound_val(r2.x, r1.x, r1.x + r1.w);
	r.y = bound_val(r2.y, r1.y, r1.y + r1.h);
	r.w = bound_val(r2.x + r2.w - r.x, 0, r1.x + r1.w - r.x);
	r.h = bound_val(r2.y + r2.h - r.y, 0, r1.y + r1.h - r.y);

	return r;
}


__END_DECLS
#endif
// vim:tabstop=4:shiftwidth=4


