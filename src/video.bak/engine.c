/* 
 * engine.c
 * by WN @ Feb 03, 2009
 */

#include <common/defs.h>
#include <common/debug.h>
#include <econfig/econfig.h>
#include <video/engine.h>

#include <video/rlist.h>
#include <video/rcommand.h>

static bool_t Prepared = FALSE;

/* 
 * engine.c implentment the interfaces defined in engine.h.
 *
 * If the only operations is Init, OpenWindow, Close..., then there's no
 * need for a specific 'engine.c', just implentment them in engine_driver.c
 * is OK. However, the render list operations and plugin operations are
 * important and shared by all driver. Therefore we use a separate engine.c.
 */

int EngineRender(struct VideoEngineContext * context,
		tick_t current_time)
{
	struct RenderCommand * cmd, *n;
	if (!Prepared)
		EnginePrepareRender(context, current_time);

	RListForEachCommandSafe(cmd, n, &context->render_list) {
		int flags;
		flags = cmd->render(cmd, current_time);
		if (flags & RENDER_FAIL)
			ERROR(VIDEO, "Render command %s failed.\n", cmd->name);
		if (flags & RENDER_REMOVE) {
			/* the macro on RListRemove contain a call to cmd->remove */
			RListRemove(cmd);
		}
		if (!(flags & RENDER_CONT))
			break;
	}

	return 0;
}

void EnginePrepareRender(struct VideoEngineContext * context,
		tick_t start_time)
{
	struct RenderCommand * cmd;

	TRACE(VIDEO, "Prepare render\n");
	context->start_time = start_time;
	/* foreach already linked command, set their start time to
	 * start_time */
	RListForEachCommand(cmd, &context->render_list) {
		cmd->start_time = start_time;
		cmd->stopped = FALSE;
	}
	Prepared = TRUE;
	return;
}

