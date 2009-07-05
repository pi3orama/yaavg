/* 
 * by WN @ May. 10, 2009
 * mm.c - implentment memory management
 */

#include <common/defs.h>
#include <common/exception.h>
#include <common/list.h>
#include <stdint.h>

#include <common/mm.h>

struct gc_free_list gc_free_list = {
	.head = LIST_HEAD_INIT(gc_free_list.head),
};

struct gc_active_list gc_active_list = {
	.head = LIST_HEAD_INIT(gc_active_list.head),
};

static struct gc_tag *
alloc_from_gc(size_t size)
{
	struct gc_tag * pos, * n;
	list_for_each_entry_safe(pos, n, &gc_free_list.head, gc_list) {
		if (pos->size == size) {
			GC_REMOVE(pos);
			return pos;
		}
	}

	return NULL;
}

void *
alloc_mem(size_t size, bool_t fill)
{
	struct gc_tag * gc;
	void * r;

	if (size <= 0)
		return NULL;

	TRACE(MEMORY, "alloc object size=%d\n", size);
	gc = alloc_from_gc(size);
	if (gc != NULL) {
		TRACE(MEMORY, "found at gc pool: %p\n", gc->ptr);
		r = gc->ptr;
		if (fill)
			memset(r, 0, size);
		return r;
	}

	TRACE(MEMORY, "not found at gc pool\n");
	if (fill)
		r = calloc(1, size);
	else
		r = malloc(size);
	TRACE(MEMORY, "alloc new block: %p\n", r);
	if (r == NULL) {
		THROW(EXCEPTION_FATAL, "Out of memory!");
	}

	return r;
}

void
free_mem(void * p)
{
	if (p == NULL)
		return;
	free(p);
}

static void *
inter_alloc(size_t size, int tag_offset, uint32_t flags,
		void * pprivate, bool_t fill)
{
	void * r;
	struct gc_tag * t;

	assert(size >= tag_offset + sizeof(struct gc_tag));

	r = alloc_mem(size, fill);

	t = (struct gc_tag*)(r + tag_offset);

	t->size = size;
	t->flags = flags;
	t->ptr = r;
	t->pprivate = pprivate;

	GC_BIRTH(t);

	return r;
}



void *
gc_malloc(size_t size, int tag_offset, uint32_t flags,
		void * pprivate)
{
	return inter_alloc(size, tag_offset, flags,
			pprivate, FALSE);
}

void *
gc_calloc(size_t size, int tag_offset, uint32_t flags,
		void * pprivate)
{
	return inter_alloc(size, tag_offset, flags,
			pprivate, TRUE);
}

void
gc_free(struct gc_tag * tag)
{
	assert(tag != NULL);
	GC_FREE(tag);
}

void
gc_cleanup(void)
{
	struct gc_tag * pos, * n;
	
	TRACE(MEMORY, "cleanup begin\n");

	list_for_each_entry_safe(pos, n, &gc_active_list.head, gc_list) {
		WARNING(MEMORY, "object %p still active when destroy\n",
				pos->ptr);
		GC_REMOVE(pos);
		TRACE(MEMORY, "free object %p\n", pos->ptr);
		free_mem(pos->ptr);
	}

	list_for_each_entry_safe(pos, n, &gc_free_list.head, gc_list) {
		GC_REMOVE(pos);
		TRACE(MEMORY, "free object %p\n", pos->ptr);
		free_mem(pos->ptr);
	}
}

// vim:tabstop=4:shiftwidth=4

