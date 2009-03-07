#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <setjmp.h>

#include <assert.h>

#include <common/defs.h>
#include <common/debug.h>

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
#define MASK_ALL				(0xffffffff)
#define MASK_SUBSYS_ALL			MASK(EXCEPTION_SUBSYS_RERUN)


struct exception {
	enum exception_level level;
	const char * message;
};

struct cleanup {
	struct cleanup * next;
	void (*function)(void*);
	void * arg;
};

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

struct catcher {
	sigjmp_buf buf;
	enum catcher_state state;
	volatile struct exception * exception;
	uint32_t mask;
	struct cleanup * saved_cleanup_chain;
	struct catcher * prev;
};

typedef uint32_t return_mask;

NORETURN void
throw_exception(enum exception_level, const char * message) ATTR_NORETURN;

static struct cleanup * current_cleanup_chain = NULL;
static struct catcher * current_catcher = NULL;

void
make_cleanup(struct cleanup * cleanup)
{
	cleanup->next = current_cleanup_chain;
	current_cleanup_chain = cleanup;
}

sigjmp_buf *
exceptions_state_mc_init(
		struct catcher * catcher,
		volatile struct exception * exception,
		return_mask mask)
{
	exception->level = EXCEPTION_NO_ERROR;
	exception->message = NULL;

	catcher->exception = exception;
	catcher->mask = mask;
	catcher->state = CATCHER_CREATED;

	catcher->saved_cleanup_chain = current_cleanup_chain;
	current_cleanup_chain = NULL;

	catcher->prev = current_catcher;
	current_catcher = catcher;
	return &(catcher->buf);
}

void
do_cleanup(void)
{
	struct cleanup * cleanup = current_cleanup_chain;
	while (cleanup != NULL) {
		struct cleanup * new_cleanup = cleanup->next;
		/* Notice: func may release the cleanup structure,
		 * therefore we first save the next cleanup data */
		cleanup->function(cleanup->arg);
		cleanup = new_cleanup;
	}
}

static void
catcher_pop(void)
{
	struct catcher * old_catcher = current_catcher;
	if (old_catcher == NULL)
		INTERNAL_ERROR(SYSTEM, "catcher_pop outside catche block\n");
	current_catcher = old_catcher->prev;
	/* do the cleanup */
	do_cleanup();
	current_cleanup_chain = old_catcher->saved_cleanup_chain;
}

static int
exceptions_state_mc(enum catcher_action action)
{
	switch (current_catcher->state) {
		case CATCHER_CREATED: 
			switch (action) {
				case CATCH_ITER:
					current_catcher->state = CATCHER_RUNNING;
					return 1;
				default:
					INTERNAL_ERROR(SYSTEM, "inner exception processing\n");
			}
		case CATCHER_RUNNING:
			switch (action) {
				case CATCH_ITER:
					/* No error occured */
					/* catcher_pop will run cleanup.
					 * cleanup should be made idempotent */
					catcher_pop();
					return 0;
				case CATCH_ITER_1:
					current_catcher->state = CATCHER_RUNNING_1;
					return 1;
				case CATCH_THROWING:
					/* I don't believe a CATCH_THROWING
					 * action can happen at CATCHER_RUNNING state,
					 * but gdb write this way. */
					current_catcher->state = CATCHER_ABORTING;
					return 1;
				default:
					INTERNAL_ERROR(SYSTEM, "inner exception processing\n");
			}
		case CATCHER_RUNNING_1:
			switch (action) {
				case CATCH_ITER:
					/* break out! */
					catcher_pop();
					return 0;
				case CATCH_ITER_1:
					/* exit normally. transfer to the outer loop */
					current_catcher->state = CATCHER_RUNNING;
					return 0;
				case CATCH_THROWING:
					current_catcher->state = CATCHER_ABORTING;
					return 1;
				default:
					INTERNAL_ERROR(SYSTEM, "inner exception processing\n");
			}
		case CATCHER_ABORTING:
			switch (action) {
				case CATCH_ITER: {
					struct exception exception =
						*current_catcher->exception;

					/* NOTICE: catcher_pop is not a public sub expression,
					 * after catcher_pop, current_catcher changed. therefore
					 * we should not pop catcher before check mask! */
					if (current_catcher->mask & (MASK(exception.level))) {
						/* This catcher can handle this  */
						catcher_pop();
						return 0;
					}

					catcher_pop();
					/* This catcher cannot handle this */
					throw_exception(exception.level, exception.message);
					/* We should not be here */
					return 0;
				}
				default:
					INTERNAL_ERROR(SYSTEM, "inner exception processing\n");
			}
		default:
			INTERNAL_ERROR(SYSTEM, "bad switch");
	}
}

int
exceptions_state_mc_action_iter (void)
{
	return exceptions_state_mc(CATCH_ITER);
}

int
exceptions_state_mc_action_iter_1 (void)
{
	return exceptions_state_mc(CATCH_ITER_1);
}

NORETURN void
throw_exception (enum exception_level level, const char * message)
{
	/* set exception and longjump */
	/* We needn't to do cleanup here. If the state machine is correct,
	 * the catcher_pop should be called, then do_cleanup() will be called,
	 * the cleanup_chain will be restored.*/
	assert(current_catcher != NULL);

	current_catcher->exception->level = level;
	current_catcher->exception->message = message;

	exceptions_state_mc(CATCH_THROWING);
	/* do the longjmp! */
	siglongjmp(current_catcher->buf, level);
}

/* Here is the MAGICAL macros */

#define EXCEPTIONS_SIGJMP_BUF sigjmp_buf
#define EXCEPTIONS_SIGSETJMP(buf) sigsetjmp((buf), 1)

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



/* cleanup need 3 args */
static void * cleanup_freemem_arg = NULL;

static void
cleanup_freemem(void * arg)
{
	printf("Run %s\n", __FUNCTION__);
	if (cleanup_freemem_arg != NULL) {
		printf("do the free\n");
		free(cleanup_freemem_arg);
	}
}

static struct cleanup cleanup_freemem_str = {
	.function	= cleanup_freemem,
	.arg  		= NULL,
};

static void
cleanup_none(void * arg)
{
	printf("do cleanup_none\n");
}

static struct cleanup cleanup_none_str = {
	.function 	= cleanup_none,
	.arg		= NULL
};

void
test_func()
{
	char * buf = NULL;
	/* We alloc some resources */
	buf = malloc(1024);
	memset(buf, '1', 1024);

	/* then link a cleanup */
	cleanup_freemem_arg = buf;
	make_cleanup(&cleanup_freemem_str);

	struct exception exp;
	TRY_CATCH(exp, MASK_SYS_REINIT) {
		make_cleanup(&cleanup_none_str);
//		throw_exception(EXCEPTION_FATAL, "Fatal!!!\n");
	}

	cleanup_none(NULL);
}

int main()
{

	DEBUG_INIT(NULL);
	volatile struct exception exp;
	TRY_CATCH(exp, MASK_ALL) {
		printf("In try_cache block!\n");

		test_func();

		throw_exception(EXCEPTION_USER_QUIT, "No problem!");
		printf("Out!!!\n");
	}
	switch (exp.level) {
		case (EXCEPTION_NO_ERROR):
			printf("OK! Wow!!\n");
			break;
		case (EXCEPTION_USER_QUIT):
			printf("User want to quit\n");
			break;
		default:
			printf("Final exception: %s\n", exp.message);
			break;
	}

	return 0;
}

// vim:tabstop=4:shiftwidth=4

