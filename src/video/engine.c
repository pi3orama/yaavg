/* 
 * engine.c
 * by WN @ Feb 03, 2009
 */

#include <common/defs.h>
#include <common/debug.h>
#include <econfig/econfig.h>
#include <video/engine.h>
#include <video/engine_driver.h>

#include <video/rlist.h>
#include <video/rcommand.h>

static struct VideoContext * VideoCtx = NULL;

/* Some init funcs */
int
VideoInit(void)
{
	if (DriverInit()) {
		FATAL(VIDEO, "Driver init failed\n");
		exit(1);
	}
	VideoCtx = NULL;
	return 0;
}


struct VideoContext *
VideoOpenWindow(void)
{
	VideoCtx = DriverOpenWindow();
	if (VideoCtx == NULL) {
		FATAL(VIDEO, "Driver open window failed\n");
		exit(1);
	}
	RListInit(&(VideoCtx->render_list), VideoCtx);
	return VideoCtx;
}

void
VideoClose(void)
{
	DriverClose();
	/* clear render list */
	if (VideoCtx != NULL)
		RListClear(&VideoCtx->render_list);
	VideoCtx = NULL;
	return;
}



static bool_t Prepared = FALSE;

void
VideoPrepareRender(tick_t start_time)
{
	struct RenderCommand * cmd;

	TRACE(VIDEO, "Prepare render\n");
	assert(VideoCtx != NULL);

	VideoCtx->start_time = start_time;
	/* foreach already linked command, set their start time to
	 * start_time */
	RListForEachCommand(cmd, &VideoCtx->render_list) {
		cmd->start_time = start_time;
		cmd->stopped = FALSE;
	}
	Prepared = TRUE;
	return;
}

int
VideoRender(tick_t current_time)
{
	struct RenderCommand * cmd, *n;

	RListForEachCommandSafe(cmd, n, &VideoCtx->render_list) {
		int flags;
		flags = cmd->render(cmd, current_time);
		if (flags & RENDER_FAIL) {
			ERROR(VIDEO, "Render command %s failed.\n", cmd->name);
			RListRemove(cmd, REMOVE_ERROR, flags);
		}
		if (flags & RENDER_REMOVE) {
			/* the macro on RListRemove contain a call to cmd->remove */
			RListRemove(cmd, REMOVE_FINISH, flags);
		}
		if (!(flags & RENDER_CONT))
			break;
	}

	return 0;
}

/* Implentmented in engine_xx */
#if 0
extern void
VideoSwapBuffers(void);

extern int
VideoReshape(int w, int h);


extern void
VideoSetCaption(const char * caption);

extern void
VideoSetIcon(const icon_t icon);
#endif

