/* 
 * exception.h
 * by WN @ Mar. 07, 2009
 *
 * C exception facility, inspired by GDB's code.
 * This C exception is not suitable for dynamic objects. 
 * I use this exception handler to link all clean-ups together,
 * and to suitable recovery when needed.
 */

#ifndef YAAVG_EXCEPTION_H
#define YAAVG_EXCEPTION_H

#include <config.h>

#include <sys/cdefs.h>
#include <common/defs.h>
#include <common/debug.h>
#include <common/list.h>
#include <memory.h>
#include <stdlib.h>
#include <assert.h>
#include <setjmp.h>


__BEGIN_DECLS


enum exception_level {
	EXCEPTION_NO_ERROR			= 0,
	EXCEPTION_USER_QUIT			= 1,
	EXCEPTION_SUBSYS_RERUN		= 2,
	EXCEPTION_SUBSYS_SKIPFRAME	= 3,
	EXCEPTION_SUBSYS_REINIT		= 4,
	EXCEPTION_SYS_RERUN			= 5,
	EXCEPTION_SYS_SKIPFRAME		= 6,
	EXCEPTION_SYS_REINIT		= 7,
	EXCEPTION_FATAL				= 8,
	EXCEPTION_RESET				= 9,
	EXCEPTION_CONTINUE			= 10,
};

#define MASK(level)				(1 << (int)(level))
#define MASK_QUIT				MASK(EXCEPTION_USER_QUIT)
#define MASK_SUBSYS_RERUN		MASK(EXCEPTION_SUBSYS_RERUN)
#define MASK_SUBSYS_SKIPFRAME	MASK(EXCEPTION_SUBSYS_SKIPFRAME)
#define MASK_SUBSYS_REINIT		MASK(EXCEPTION_SUBSYS_REINIT)
#define MASK_SYS_RERUN			MASK(EXCEPTION_SYS_RERUN)
#define MASK_SYS_SKIPFRAME		MASK(EXCEPTION_SYS_SKIPFRAME)
#define MASK_SYS_REINIT			MASK(EXCEPTION_SYS_REINIT)
#define MASK_FATAL				MASK(EXCEPTION_FATAL)
#define MASK_RESET				MASK(EXCEPTION_RESET)
#define MASK_CONTINUE			MASK(EXCEPTION_CONTINUE)
#define MASK_ALL				(0xffffffff)
#define MASK_NONFATAL			((MASK_ALL) & (~(MASK_FATAL)) &(~(MASK_RESET)) & (~(MASK_QUIT)))
#define MASK_SYS_ALL			(MASK_SYS_RERUN | MASK_SYS_SKIPFRAME | MASK_SYS_REINIT)
#define MASK_SUBSYS_ALL			(MASK_SUBSYS_RERUN | MASK_SUBSYS_SKIPFRAME | MASK_SUBSYS_REINIT)


struct exception {
	enum exception_level level;
	const char * message;
};

#if defined(HAVE_SIGSETJMP)
#define EXCEPTIONS_SIGJMP_BUF		sigjmp_buf
#define EXCEPTIONS_SIGSETJMP(buf)	sigsetjmp((buf), 1)
#define EXCEPTIONS_SIGLONGJMP(buf,val)	siglongjmp((buf), (val))
#else
#define EXCEPTIONS_SIGJMP_BUF		jmp_buf
#define EXCEPTIONS_SIGSETJMP(buf)	setjmp(buf)
#define EXCEPTIONS_SIGLONGJMP(buf,val)	longjmp((buf), (val))
#endif

enum catcher_state {
	CATCHER_CREATED,
	CATCHER_RUNNING,
	CATCHER_RUNNING_1,
	CATCHER_ABORTING,
};

enum catcher_action {
	CATCH_ITER,
	CATCH_ITER_1,
	CATCH_THROWING,
};

struct cleanup {
	struct list_head list;
	/* Cleanup func need a param, because sometime the struct cleanup
	 * is dynamically alloced, and need to be free, sometime it is static alloced. */
	void (*function)(struct cleanup * cleanup);
	void * args;
};

struct catcher {
	EXCEPTIONS_SIGJMP_BUF buf;
	enum catcher_state state;
	volatile struct exception * exception;
	uint32_t mask;
	struct list_head cleanup_chain;
	struct list_head * saved_cleanup_chain;
	struct catcher * prev;
};

typedef uint32_t return_mask;


#define TRY_CATCH(EXCEPTION,MASK) \
     { \
		 struct catcher catcher; \
		 memset(&catcher, '\0', sizeof(catcher));\
       EXCEPTIONS_SIGJMP_BUF *buf = \
		 exceptions_state_mc_init (&catcher, &(EXCEPTION), (MASK)); \
       EXCEPTIONS_SIGSETJMP (*buf); \
     } \
     while (exceptions_state_mc_action_iter ()) \
       while (exceptions_state_mc_action_iter_1 ())


/* 
 * NOTICE: cleanup MUST inactived. see cleanup_actived
 */
void
make_cleanup(struct cleanup * cleanup);

void
remove_cleanup(struct cleanup * cleanup);

static inline int
cleanup_actived(struct cleanup * cleanup)
{
	if (list_head_deleted(&cleanup->list))
		return FALSE;
	if (list_empty(&cleanup->list))
		return FALSE;
	return TRUE;
}


void
do_cleanup(void);

EXCEPTIONS_SIGJMP_BUF *
exceptions_state_mc_init(
		struct catcher * catcher,
		volatile struct exception * exception,
		return_mask mask);

int
exceptions_state_mc_action_iter(void);

int
exceptions_state_mc_action_iter_1(void);


NORETURN void
throw_exception(enum exception_level, const char * message) ATTR_NORETURN;

#define THROWS(...)

__END_DECLS


#endif
// vim:tabstop=4:shiftwidth=4

