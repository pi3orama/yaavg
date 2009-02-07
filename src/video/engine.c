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
static tick_t game_ticks = 0;

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
	game_ticks = 0;
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

int
VideoPhy(dtick_t delta_time)
{
	struct RenderCommand * cmd, *n;

	game_ticks += delta_time;

	RListForEachCommandSafe(cmd, n, &VideoCtx->render_list) {
		int flags = 0;
		dtick_t t;

		t = delta_time;

		if (cmd->first) {
			t = 0;
			cmd->first = FALSE;
		}

		if (cmd->paused)
			t = 0;

		if (cmd->phy)
			flags = cmd->phy(cmd, t);

		if (flags & RENDER_FAIL) {
			ERROR(VIDEO, "Render command %s phy failed.\n", cmd->name);
			RListRemove(cmd, REMOVE_ERROR, flags);
		}
		if (flags & RENDER_REMOVE) {
			/* the macro on RListRemove contain a call to cmd->remove */
			RListRemove(cmd, REMOVE_FINISH, flags);
		}

		if (flags & RENDER_STOP)
			break;
	}
	return 0;
}

int
VideoRender(void)
{
	struct RenderCommand * cmd, *n;

	RListForEachCommandSafe(cmd, n, &VideoCtx->render_list) {
		int flags = 0;
		if (cmd->render)
			flags = cmd->render(cmd);

		if (flags & RENDER_FAIL) {
			ERROR(VIDEO, "Render command %s render failed.\n", cmd->name);
			RListRemove(cmd, REMOVE_ERROR, flags);
		}
		if (flags & RENDER_REMOVE) {
			/* the macro on RListRemove contain a call to cmd->remove */
			RListRemove(cmd, REMOVE_FINISH, flags);
		}
		if (flags & RENDER_STOP)
			break;
	}

	return 0;
}

tick_t
GetGameTicks(void)
{
	return game_ticks;
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

