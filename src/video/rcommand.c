/* 
 * by WN @ Jan 28, 2009
 */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <video/rcommand.h>

void RCommandInit(struct RenderCommand * command)
{
	memset(command, 0, sizeof(*command));
	INIT_LIST_HEAD(&command->list);
}

