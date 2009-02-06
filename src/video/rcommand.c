/* 
 * by WN @ Jan 28, 2009
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <video/rcommand.h>

void RCommandInit(struct RenderCommand * command,
		const char * name, 
		struct VideoEngineContext * context,
		render_func render,
		sprintf_func sprintf,
		remove_func remove)
{
	memset(command, '\0', sizeof(*command));
	INIT_LIST_HEAD(&command->list);
	command->name = name;
	command->context = context;
	command->render = render;
	command->sprintf = sprintf;
	command->remove = remove;
}

