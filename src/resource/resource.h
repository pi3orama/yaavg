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
#include <stdint.h>

typedef uint64_t res_id_t;

typedef enum _res_type_t {
	RES_BITMAP,
} res_type_t;

struct resource {
	/* garbage collection utils */
	struct list_head list;
	struct cleanup cleanup;
	int ref_count;
	uint32_t data_size;
	res_type_t type;
	res_id_t id;
};

static inline void
res_get_resource(struct resource * res)
{
	res->ref_count ++;
}

static inline void
res_put_resource(struct resource * res)
{
	res->ref_count --;
	if (res->ref_count <= 0)
		res->cleanup.function(&res->cleanup);
}

#endif
// vim:tabstop=4:shiftwidth=4

