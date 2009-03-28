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

typedef uint64_t resid_t;

struct resource {
	resid_t id;
	/* garbage collection utils */
	struct list_head list;
	struct cleanup cleanup;
	int pin_count;
};



#endif
// vim:tabstop=4:shiftwidth=4

