/* 
 * by WN @ Jan 28, 2009
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#include <video/rcommand.h>

static struct RenderCommandOperations null_ops = {
	.phy		= NULL,
	.render		= NULL,
	.remove		= NULL,
	.destroy	= NULL,	
	.finish		= NULL,
	.speedup	= NULL,
	.sprintf	= NULL,
};

void RCommandInit(struct RenderCommand * command,
		const char * name,
		bool_t revert_time,
		struct VideoContext * context,
		struct RenderCommandOperations * ops)
{
	
	memset(command, '\0', sizeof(*command));
	INIT_LIST_HEAD(&command->list);
	command->name		= name;
	command->revert_time	= revert_time;
	command->context	= context;

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
RCommandSetActive(struct RenderCommand * cmd)
{
	assert(cmd != NULL);
	cmd->active = TRUE;
	if (cmd->pairflag != 0) {
		assert(cmd->pair_command != NULL);
		struct RenderCommand * pair_cmd = cmd->pair_command;
		pair_cmd->active = TRUE;
	}
}

