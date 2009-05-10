/* 
 * resource.c
 * by WN @ Mar. 23, 2009
 *
 * A throw-away model for resource layer
 */

#include <common/list.h>
#include <resource/resource.h>

static LIST_HEAD(res_list);

struct resource *
res_search(res_id_t id, res_type_t type)
{
	struct resource * pos;
	list_for_each_entry(pos, &res_list, list) {
		if ((pos->id == id) && (pos->type == type)) {
			return pos;
		}
	}
	return NULL;
}

void
res_birth(struct resource * r)
{
	assert(r != NULL);
	/* FIXME  */
	/* We need check the pool (dict) whether there's a same
	 * resource already */
	list_add(&(r)->list, &res_list);
}

void
res_die(struct resource * r)
{
	if (!list_head_deleted(&(r)->list))
		if (!list_empty(&(r)->list))
			list_del(&(r)->list);
}


// vim:tabstop=4:shiftwidth=4

