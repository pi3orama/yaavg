/* 
 * exception.c
 * by WN @ Mar. 07, 2009
 *
 * See exception.h. C execption facility.
 */

#include <config.h>

#include <common/exception.h>
#include <common/debug.h>
#include <common/list.h>

static LIST_HEAD(default_cleanup_chain);
static struct list_head * current_cleanup_chain = &default_cleanup_chain;

static struct catcher * current_catcher = NULL;


static void
catcher_pop(void);

static int
exceptions_state_mc(enum catcher_action action);


void
make_cleanup(struct cleanup * cleanup)
{
	list_add(&cleanup->list, current_cleanup_chain);
}

void
remove_cleanup(struct cleanup * cleanup)
{
	assert(cleanup != NULL);
	list_del(&cleanup->list);
}


void
do_cleanup(void)
{
	struct cleanup * pos, *n;
	assert(current_cleanup_chain != NULL);
	list_for_each_entry_safe(pos, n, current_cleanup_chain, list) {
		/* Notice: func may release the cleanup structure,
		 * therefore we first save the next cleanup data */
		list_del(&pos->list);
		pos->function(pos);
	}
}

EXCEPTIONS_SIGJMP_BUF *
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

	catcher->prev = current_catcher;

	INIT_LIST_HEAD(&catcher->cleanup_chain);
	catcher->saved_cleanup_chain = current_cleanup_chain;
	current_cleanup_chain = &catcher->cleanup_chain;

	current_catcher = catcher;
	return &(catcher->buf);
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
	if (current_catcher == NULL) {
		/* We are not in a catch block. do all cleanup then
		 * quit */
		WARNING(SYSTEM, "throw exception out of a catcher block\n");
		WARNING(SYSTEM, "exception message: %s\n", message);
		do_cleanup();
		exit(-1);
	}

	/* set exception and longjump */
	/* We needn't to do cleanup here. If the state machine is correct,
	 * the catcher_pop should be called, then do_cleanup() will be called,
	 * the cleanup_chain will be restored.*/

	current_catcher->exception->level = level;
	current_catcher->exception->message = message;

	exceptions_state_mc(CATCH_THROWING);
	/* do the longjmp! */
	EXCEPTIONS_SIGLONGJMP(current_catcher->buf, level);
}

/* Below are private funcs */

static void
catcher_pop(void)
{
	struct catcher * old_catcher = current_catcher;
	if (old_catcher == NULL)
		INTERNAL_ERROR(SYSTEM, "catcher_pop outside catche block\n");
	current_catcher = old_catcher->prev;
	/* do the cleanup */
	do_cleanup();
	/* restore cleanup chain */
	current_cleanup_chain = old_catcher->saved_cleanup_chain;
	if (current_catcher != NULL)
		assert(current_cleanup_chain == (&current_catcher->cleanup_chain));
	else
		assert(current_cleanup_chain == &default_cleanup_chain);
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

// vim:tabstop=4:shiftwidth=4
