/* Wang Nan @ Jan 25, 2009 */

#ifndef DEFS_H
#define DEFS_H
#include <config.h>
#include <sys/cdefs.h>
#include <stdint.h>
__BEGIN_DECLS

typedef int bool_t;
#define TRUE	(1)
#define FALSE	(!TRUE)

typedef uint32_t tick_t;
/* dtick is difference between ticks */
typedef int32_t dtick_t;

typedef void * icon_t;

/* Copy code from list.h */
#ifndef offsetof
# ifdef __compiler_offsetof
#  define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
# else
#  define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
# endif
#endif

#ifndef container_of
# define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})
#endif

__END_DECLS

#endif

