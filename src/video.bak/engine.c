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

struct VideoContext * VideoCtx = NULL;
static tick_t game_ticks = 0;

/* Some init funcs */
int
VideoInit(void)
{
	if (DriverInit()) {
		FATAL(VIDEO, "Driver init failed\n");
		exit(-1);
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
		exit(-1);
	}
	RListInit(&(VideoCtx->render_list), VideoCtx);
	game_ticks = 0;
	return VideoCtx;
}

void
VideoReopenWindow(struct VideoContext * ctx)
{
	assert(ctx == VideoCtx);
	DriverReopenWindow(ctx);
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
		int failed = 0;
		dtick_t t;

		if (!cmd->active)
			continue;

		t = delta_time;

		if (cmd->first) {
			t = 0;
			cmd->first = FALSE;
		}

		if (cmd->stopped)
			t = 0;

		if (cmd->revert_time)
			t = -t;

		if (cmd->ops->phy)
			flags = cmd->ops->phy(cmd, t);

		if (flags & RENDER_FAIL) {
			ERROR(VIDEO, "Render command %s phy failed.\n", cmd->name);
			failed = 1;
			VideoRemoveACommand(cmd, REMOVE_ERROR, flags);
		}

		if ((flags & RENDER_REMOVE) && (!failed)) {
			/* Only render can return RENDER_REMOVE, 
			 * phy return REMOVE is an error */
			VideoRemoveACommand(cmd, REMOVE_ERROR, flags);
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
		int failed = 0;

		if (!cmd->active)
			continue;

		if ((RCMDIsLeft(cmd)) && (cmd->ops->lrender))
			flags = cmd->ops->lrender(cmd);
		else if ((RCMDIsRight(cmd)) && (cmd->ops->rrender))
			flags = cmd->ops->rrender(cmd);
		else if (cmd->ops->render)
			flags = cmd->ops->render(cmd);

		if (flags & RENDER_FAIL) {
			ERROR(VIDEO, "Render command %s render failed.\n", cmd->name);
			failed = 1;
			VideoRemoveACommand(cmd, REMOVE_ERROR, flags);
		}
		if ((flags & RENDER_REMOVE) && (!failed)) {
			/* belown func contain a call to cmd->remove */
			VideoRemoveACommand(cmd, REMOVE_FINISH, flags);
		}

		if (flags & RENDER_STOP)
			break;
	}

	return 0;
}

int
VideoInsertCommand(struct RenderCommand * cmd, BOA_t boa,
		struct RenderCommand * pos)
{
	struct RenderList * rlist = &VideoCtx->render_list;
	assert(cmd != NULL);
	
	assert(!cmd->inserted);
	assert(!cmd->active);

	cmd->pairflag = 0;
	cmd->pair_command = NULL;

	/* even if pos is NULL, RListLinkXXX can do
	 * the right thing */
	if (boa == BEFORE) {
		RListLinkBefore(rlist, pos, cmd);
	} else {
		assert(boa == AFTER);
		RListLinkAfter(rlist, pos, cmd);
	}
	cmd->inserted = TRUE;
	return 0;
}

static struct RenderCommand *
adjust_to_before(struct RenderCommand * pos,
		BOA_t boa)
{
	struct RenderList * rlist = &VideoCtx->render_list;

	if (boa == AFTER) {
		return RListGetNextCommand(rlist, pos);
	}
	return pos;
}

int
VideoInsertCommandPair(struct RenderCommand * lcmd,
		BOA_t lboa,
		struct RenderCommand * lpos, 
		struct RenderCommand * rcmd,
		BOA_t rboa,
		struct RenderCommand * rpos)
{
	struct RenderList * rlist = &VideoCtx->render_list;
	struct RenderCommand * pos = NULL;

	assert(lcmd != NULL);
	assert(rcmd != NULL);

	assert(!lcmd->inserted);
	assert(!lcmd->active);
	assert(!rcmd->inserted);
	assert(!rcmd->active);

	lpos = adjust_to_before(lpos, lboa);
	rpos = adjust_to_before(rpos, rboa);

	if (lpos == NULL) {
		/* if lpos == NULL, then we want to insert
		 * the left command "before the tail of list" */
		if (rpos != NULL)
			goto pair_error;
	} else {
		PairFlag_t pairflag_sum = 0;
		for (pos = lpos;
				((pos != rpos) && (pos != NULL));
				pos = RListGetNextCommand(rlist, pos))
		{
			pairflag_sum += pos->pairflag;
			if (pairflag_sum < 0)
				goto pair_error;
		}

		/* rpos is on the left of lpos! */
		if ((pos == NULL) && (rpos != NULL))
			goto pair_error;
		assert(pos == rpos);
		if (pairflag_sum != 0)
			goto pair_error;
	}


	/* then we go to the instertion */
	/* generate pair flag */
	/* mod a large enough prime number */
	/* 115547 is much smaller than (1 << 32)(4294967296),
	 * so don't worry about the sum revert to negitive. */
	/* +1 makes it not zero */
	int pairflag_base = (uint32_t)(lcmd) % 115547 + 1;

	lcmd->pairflag = pairflag_base;
	lcmd->pair_command = rcmd;

	rcmd->pairflag = - pairflag_base;
	rcmd->pair_command = lcmd;

	/* insert lcmd before lpos */
	RListLinkBefore(rlist, lpos, lcmd);
	lcmd->inserted = TRUE;

	RListLinkBefore(rlist, rpos, rcmd);
	rcmd->inserted = TRUE;

	return 0;

pair_error:
	WARNING(VIDEO, "Trying to insert command pair [%s](%p) and [%s](%p) but failed\n",
			lcmd->name, lcmd, rcmd->name, rcmd);
	return -1;

}

int
VideoRemoveCommand(struct RenderCommand * cmd,
		RemoveReason_t reason, int flags)
{
	assert(cmd != NULL);
	if (!cmd->inserted)
		return 0;

	cmd->inserted = FALSE;
	cmd->active = FALSE;
	cmd->first = TRUE;
	RListRemove(cmd, reason, flags);
	return 0;
}

int
VideoRemoveCommandPair(struct RenderCommand * lcmd,
		RemoveReason_t reason, int flags)
{
	struct RenderCommand * rcmd;

	assert(lcmd != NULL);

	if (lcmd->pairflag == 0)
		goto pair_error;
	if (lcmd->pair_command == NULL)
		goto pair_error;
	
	rcmd = lcmd->pair_command;
	if (rcmd->pairflag == 0)
		goto pair_error;
	if (rcmd->pair_command != lcmd)
		goto pair_error;
	if (lcmd->pairflag + rcmd->pairflag != 0)
		goto pair_error;

	VideoRemoveCommand(lcmd, reason, flags);
	VideoRemoveCommand(rcmd, reason, flags);
	return 0;

pair_error:
	if (rcmd != NULL)
		WARNING(VIDEO, "Try to remove command pair [%s](%p) and [%s](%p) but failed\n",
			lcmd->name, lcmd, rcmd->name, rcmd);
	else
		WARNING(VIDEO, "Try to remove command pair [%s](%p) but it has no partner\n",
				lcmd->name, lcmd);
	return -1;
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

