/* 
 * resource.h
 * by WN @ Mar. 22, 2009
 */

#ifndef RESOURCE_H
#define RESOURCE_H

#include <common/defs.h>
#include <common/exception.h>
#include <common/debug.h>
#include <common/list.h>
#include <common/mm.h>
#include <stdint.h>

typedef uint64_t res_id_t;

typedef enum _res_type_t {
	RES_BITMAP,
} res_type_t;

struct resource {
	/* garbage collection utils */
	struct gc_tag gc_tag;
	/* resource list, may be a key, or a
	 * rbtree node is better */
	struct list_head list;
	struct cleanup cleanup;
	int ref_count;
	uint32_t data_size;
	res_type_t type;
	/* resid should not be 0 */
	res_id_t id;
};

#define RES_CLEANUP(r) ((r)->cleanup.function(&((r)->cleanup)))

static inline void
res_grab_resource(struct resource * res)
{
	if (res->ref_count < 0)
		res->ref_count = 0;
	res->ref_count ++;
}


static inline void
res_put_resource(struct resource * res)
{
	res->ref_count --;
	if (res->ref_count <= 0)
		RES_CLEANUP(res);
}

#endif
// vim:tabstop=4:shiftwidth=4

