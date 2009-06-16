/* 
 * video.c
 * by WN @ Mar. 08, 2009
 *
 * implement part of video.h
 */

#include <common/debug.h>
#include <common/exception.h>

#include <video/video.h>
#include <video/video_engine.h>

#include <video/rcommand.h>
#include <video/rlist.h>

static struct video_context * video_ctx = NULL;
static tick_t game_ticks = 0;


struct video_context *
video_get_current_context(void)
{
	return video_ctx;
}

static void __video_close(struct cleanup * str);

static struct cleanup video_cleanup_str = {
	.function 	= __video_close,
	.list = {NULL, NULL},
};

static void
__video_close(struct cleanup * str)
{
	assert(str == &video_cleanup_str);
	if (video_ctx != NULL) {
		rlist_clear(&video_ctx->render_list);
		/* NOTICE: engine_close should be made idempotent.
		 * if a internel exception is thrown, the engine_close
		 * may be called before. */
		engine_close();
		video_ctx = NULL;
	}
}

/* NOTICE: We shoud never call video_close directly */
void video_close()
{
	__video_close(&video_cleanup_str);
}


struct video_context *
video_init(void)
{
	if (video_ctx != NULL) {
		WARNING(VIDEO, "multi video_init\n");
		return video_ctx;
	}

	video_ctx = engine_init();
	if (video_ctx == NULL) {
		/* We shouldn't be here, engine should have
		 * throw an exception */
		THROW(EXCEPTION_FATAL, "Driver init failed");
	}
	make_reinitable_cleanup(&video_cleanup_str);
	rlist_init(&(video_ctx->render_list));
	game_ticks = 0;
	video_set_caption("-- YAAVG --");
	return video_ctx;
}

/* video_reinit doesn't touch the rlist, keep the
 * commands there, to for render next frame. */
void
video_reinit(void)
{
	engine_reinit();
}

int
video_render(dtick_t delta_time)
{
	struct render_command * cmd, * n;
	game_ticks += delta_time;

	engine_begin_frame();
	rlist_for_each_rcmd_safe(cmd, n, &(video_ctx->render_list)) {
		int flags = 0;
		dtick_t t = delta_time;

		if (!rcmd_is_active(cmd))
			continue;

		if (rcmd_is_first(cmd)) {
			t = 0;
			rcmd_unset_first(cmd);
		}

		if (rcmd_is_stopped(cmd))
			t = 0;

		if (rcmd_is_revert(cmd))
			t = -t;
		
		if ((rcmd_is_left(cmd)) && (cmd->ops) && (cmd->ops->lrender))
			flags = cmd->ops->lrender(cmd, t);
		else if ((rcmd_is_right(cmd)) && (cmd->ops) && (cmd->ops->rrender))
			flags = cmd->ops->rrender(cmd, t);
		else if ((cmd->ops) && (cmd->ops->render))
			flags = cmd->ops->render(cmd, t);

		if (flags & RENDER_FAIL) {
			/* rcommand doesn't throw a fatal exception, then
			 * this is only a small error. we remove related
			 * commands and continue. */
			WARNING(VIDEO, "Render command %s render failed\n",
					cmd->name);
			video_remove_a_command(cmd, REMOVE_ERROR, flags);
		}

		if ((flags & RENDER_REMOVE) && (!(flags & RENDER_FAIL))) {
			video_remove_a_command(cmd, REMOVE_FINISH, flags);
		}

		if (flags & RENDER_STOP)
			break;
	}
	engine_end_frame();
	return 0;
}

void
video_insert_command(struct render_command * cmd, BOA_t boa,
		struct render_command * pos)
{
	struct render_list * rlist = &video_ctx->render_list;
	assert(cmd != NULL);
	assert(!cmd->inserted);
	assert(!cmd->active);

	cmd->pairflag = 0;
	cmd->pair_rcmd = NULL;


	/* even if pos is NULL, RListLinkXXX can do
	 * the right thing */
	if (boa == BEFORE) {
		rlist_link_before(rlist, pos, cmd);
	} else {
		rlist_link_after(rlist, pos, cmd);
	}
	return;
}

static struct render_command *
adjust_to_before(struct render_command * pos,
		BOA_t boa)
{
	struct render_list * rlist = &video_ctx->render_list;
	if (boa == AFTER) {
		return rlist_get_next_command(rlist, pos);
	}
	return pos;
}

void
video_insert_command_pair(struct render_command * lcmd,
		BOA_t lboa,
		struct render_command * lpos,
		struct render_command * rcmd,
		BOA_t rboa,
		struct render_command * rpos)
{
	struct render_list * rlist = &video_ctx->render_list;
	struct render_command * pos = NULL;

	assert(lcmd != NULL);
	assert(rcmd != NULL);

	assert(!rcmd_is_inserted(lcmd));
	assert(!rcmd_is_inserted(rcmd));
	assert(!rcmd_is_active(lcmd));
	assert(!rcmd_is_active(rcmd));

	lpos = adjust_to_before(lpos, lboa);
	rpos = adjust_to_before(rpos, rboa);

	if (lpos == NULL) {
		/* if lpos == NULL, then we want to insert
		 * the left command "before the tail of list" */
		if (rpos != NULL)
			THROW(EXCEPTION_FATAL,
					"Insert rcmd pair failed.");
	} else {
		rcmd_pair_flag_t sum = 0;
		for (pos = lpos;
				((pos != rpos) && (pos != NULL));
				pos = rlist_get_next_command(rlist, pos))
		{
			sum += pos->pairflag;
			if (sum < 0)
				THROW(EXCEPTION_FATAL,
						"try insert rcmd pair"
						" in invalid position");
		}

		/* rpos is on the left of lpos! */
		if ((pos == NULL) && (rpos != NULL))
			THROW(EXCEPTION_FATAL,
					"try insert rcmd pair"
					" in invalid position");
		if (pos != rpos)
			THROW(EXCEPTION_FATAL,
					"try insert rcmd pair"
					" in invalid position");
		if (sum != 0)
			THROW(EXCEPTION_FATAL,
					"try insert rcmd pair"
					" in invalid position");
	}


	/* then we go to the instertion */
	/* generate pair flag */
	/* mod a large enough prime number */
	/* 115547 is much smaller than (1 << 32)(4294967296),
	 * so don't worry about the sum revert to negitive. */
	/* +1 makes it not zero */
	rcmd_pair_flag_t pairflag_base = (uint32_t)(lcmd) % 115547 + 1;

	lcmd->pairflag = pairflag_base;
	lcmd->pair_rcmd = rcmd;

	rcmd->pairflag = - pairflag_base;
	rcmd->pair_rcmd = lcmd;

	/* insert lcmd before lpos */
	rlist_link_before(rlist, lpos, lcmd);
	rlist_link_before(rlist, rpos, rcmd);

	return;
}

void
video_remove_command(struct render_command * cmd,
		rcmd_remove_reason_t reason,
		int flags)
{
	assert(cmd != NULL);
	if (!rcmd_is_inserted(cmd))
		return;
	rcmd_unset_inserted(cmd);
	/* remove cmdpair call this func,
	 * we don't need rcmd_unset_active. NOTICT:
	 * rcmd_set_active is a 'pairful' function. */
	cmd->active = FALSE;
	rcmd_set_first(cmd);
	rlist_remove(cmd, reason, flags);
	return;
}

void
video_remove_command_pair(struct render_command * lcmd,
		rcmd_remove_reason_t reason,
		int flags)
{
	struct render_command * rcmd;

	assert(lcmd != NULL);
	if (!rcmd_is_pair(lcmd)) {
		WARNING(VIDEO, "%p not a pair rcmd, pairflag=%d\n",
				lcmd->pairflag);
		video_remove_command(lcmd, reason, flags);
		return;
	}

	if (lcmd->pair_rcmd == NULL) {
		WARNING(VIDEO, "%p is a pair rcmd, but pair_rcmd == NULL\n",
				lcmd->pairflag);
		video_remove_command(lcmd, reason, flags);
		return;
	}

	rcmd = lcmd->pair_rcmd;

	if (rcmd->pairflag == 0)
		WARNING(VIDEO, "right cmd pairflag is 0\n");
	if (rcmd->pair_rcmd != lcmd)
		WARNING(VIDEO, "pair cmd pointer mismatch\n");
	if (lcmd->pairflag + rcmd->pairflag != 0)
		WARNING(VIDEO, "pairflag mismatch\n");

	video_remove_command(lcmd, reason, flags);
	video_remove_command(rcmd, reason, flags);
	return;
}

tick_t
get_game_ticke(void)
{
	return game_ticks;
}

// vim:tabstop=4:shiftwidth=4

