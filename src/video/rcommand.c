/* 
 * rcommand.c - by WN @ Mar. 08, 2009
 */

#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#include <video/rcommand.h>


static struct rcmd_operations null_ops = {
	.init		= NULL,
	.render		= NULL,
	.lrender	= NULL,
	.rrender	= NULL,
	.remove		= NULL,
	.destroy	= NULL,	
	.finish		= NULL,
	.speedup	= NULL,
	.snprintf	= NULL,
};

/* before the cmd inserted onto rlist and after it is inited,
 * if exception happend, use this remove func to make sure all
 * resource is freed */
static void
rcmd_simple_remove(struct cleanup * cleanup)
{
	struct render_command * rcmd = (struct render_command *)cleanup->args;
	if ((rcmd->ops) && (rcmd->ops->remove))
		rcmd->ops->remove(rcmd, REMOVE_EXCEPTION, 0);
}

void
rcmd_init(struct render_command * command,
		const char * name,
		bool_t revert_time,
		struct video_context * vctx,
		struct rcmd_operations * ops)
{
	memset(command, '\0', sizeof(*command));
	INIT_LIST_HEAD(&command->list);
	command->name		= name;
	command->revert_time	= revert_time;
	command->video_context	= vctx;

	command->first		= TRUE;
	command->stopped	= FALSE;
	command->active		= FALSE;
	command->inserted	= FALSE;

	if (ops == NULL)
		command->ops = &null_ops;
	else
		command->ops = ops;

	/* init the cleanup func */
	command->cleanup.function = rcmd_simple_remove;
	command->cleanup.args = command;
	/* we have inited cleanup's list to NULL in memset */
	make_cleanup(&command->cleanup);
	if (ops->init)
		ops->init(command);
}

void
rcmd_remove(struct render_command * rcmd,
		rcmd_remove_reason_t reason,
		int flags)
{

	/* NOTICE: remove_cleanup will detect whether
	 * the cleanup is active. */
	remove_cleanup(&rcmd->cleanup);

	if ((rcmd->ops) && (rcmd->ops->remove))
		rcmd->ops->remove(rcmd, reason, flags);
}

extern void
rcmd_set_active(struct render_command * cmd)
{
	assert(cmd != NULL);
	cmd->active = TRUE;
	if (rcmd_is_pair(cmd)) {
		struct render_command * pair = cmd->pair_rcmd;
		pair->active = TRUE;
	}
}

extern void
rcmd_set_inserted(struct render_command * cmd)
{
	(cmd)->inserted = TRUE;
	/* remove cleanup */
	remove_cleanup(&cmd->cleanup);
}

extern void
rcmd_unset_inserted(struct render_command * cmd)
{
	(cmd)->inserted = FALSE;
	/* make cleanup */
	/* make_cleanup will check whether cleanup has already beed inserted */
	make_cleanup(&cmd->cleanup);
}
// vim:tabstop=4:shiftwidth=4

