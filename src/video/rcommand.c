/* 
 * rcommand.c - by WN @ Mar. 08, 2009
 */

#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#include <video/rcommand.h>


static struct rcmd_operations null_ops = {
	.render		= NULL,
	.lrender	= NULL,
	.rrender	= NULL,
	.remove		= NULL,
	.destroy	= NULL,	
	.finish		= NULL,
	.speedup	= NULL,
	.snprintf	= NULL,
};

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

