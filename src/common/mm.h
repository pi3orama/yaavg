/* 
 * by WN @ May 08, 2009
 *
 * mm.h: memory management
 */

#ifndef MM_H
#define MM_H
#include <common/defs.h>
#include <common/exception.h>
#include <common/list.h>
#include <common/mm.h>
#include <stdint.h>

/* gc free objects. currently, there are
 * only 1 list. however, there will be more free
 * lists for different type of objects */
struct gc_free_list {
	struct list_head head;
};
extern struct gc_free_list gc_free_list;

struct gc_active_list {
	struct list_head head;
};
extern struct gc_active_list gc_active_list;

struct gc_tag {
	struct list_head gc_list;
	int size;
	uint32_t flags;
	/* ptr is the malloc returned pointer */
	void * ptr;
	void * pprivate;
	/* for block data */
	uint8_t data[0];
};

/* put a new object into gc pool */
#define GC_BIRTH(t) do {	\
	list_add(&(t)->gc_list, &gc_active_list.head);\
} while(0)

/* move the object out of the pool */
#define GC_REMOVE(t) do {	\
	if (!list_head_deleted(&(t)->gc_list))	\
	if (!list_empty(&(t)->gc_list))\
	list_del(&(t)->gc_list);	\
} while(0)

#define GC_FREE(t) do {	\
	GC_REMOVE((t));		\
	list_add(&(t)->gc_list, &gc_free_list.head);	\
} while(0)

/* free the memory the object occupied */
#define GC_DESTROY(t) do {} while(0)

void *
gc_malloc(size_t size, int tag_offset, uint32_t flags,
		void * pprivate);

void *
gc_calloc(size_t size, int tag_offset, uint32_t flags,
		void * pprivate);

void
gc_free(struct gc_tag * tag);


#define GC_XALLOC_BLOCK(s, xalloc) ({ 	\
		struct gc_tag * tag;	\
		void * ptr;				\
		if ((s) <= sizeof(*tag))	\
			ptr = NULL;			\
		else {					\
			tag = xalloc((s) + sizeof(*tag), 0, 0, NULL);	\
			if (tag == NULL)	\
				ptr = NULL;		\
			else				\
				ptr = tag->data;	\
		}						\
		ptr;					\
	   	})

#define GC_MALLOC_BLOCK(s) \
	GC_XALLOC_BLOCK(s, gc_malloc)
#define GC_CALLOC_BLOCK(s) \
	GC_XALLOC_BLOCK(s, gc_calloc)
#define GC_FREE_BLOCK_RVAL(p) \
	do {	\
		if (p == NULL)	\
			break;		\
		struct gc_tag * tag = container_of((void*)(p), struct gc_tag, data);	\
		gc_free(tag); \
	} while(0)

#define GC_FREE_BLOCK_SET(p) \
do {	\
	if (p == NULL)	\
		break;		\
	struct gc_tag * tag = container_of((void*)(p), struct gc_tag, data);	\
	gc_free(tag); \
	(p) = NULL;\
} while(0)

#define GC_TAG	struct gc_tag __gc_tag

#define GC_SIMPLE_MALLOC(ptr, member) \
	gc_malloc(sizeof(*ptr), offsetof(typeof(*ptr), member), 0, NULL, NULL)
#define GC_SIMPLE_CALLOC(ptr, member) \
	gc_calloc(sizeof(*ptr), offsetof(typeof(*ptr), member), 0, NULL)

#define GC_SIMPLE_FREE(ptr, member) \
	gc_free(&ptr->member)

#define GC_TRIVAL_MALLOC(ptr) \
	GC_SIMPLE_MALLOC(ptr, __gc_tag)
#define GC_TRIVAL_CALLOC(ptr) \
	GC_SIMPLE_CALLOC(ptr, __gc_tag)

#define GC_TRIVAL_FREE(ptr) \
	gc_free(&ptr->__gc_tag)

/* we don't use exception link */
void
gc_cleanup(void);

void *
alloc_mem(size_t size, bool_t fill);

void
free_mem(void * p);

static inline void *
xmalloc(size_t size)
{
	void * ptr;
	ptr = malloc(size);
	if (ptr == NULL)
		THROW(EXCEPTION_FATAL, "Out of memory");
	return ptr;
}

static inline void *
xcalloc(size_t count, size_t eltsize)
{
	void * ptr;
	ptr = calloc(count, eltsize);
	if (ptr == NULL)
		THROW(EXCEPTION_FATAL, "Out of memory");
	return ptr;
}


static inline void
xfree(void * ptr)
{
	free(ptr);
}

#endif
// vim:tabstop=4:shiftwidth=4

