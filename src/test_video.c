/* 
 * test_video.c
 * by WN @ Mar. 15, 2009
 */

#include <stdio.h>
#include <common/debug.h>
#include <common/exception.h>
#include <common/utils.h>
#include <econfig/econfig.h>

#include <video/video.h>
#include <video/video_gl.h>

#include <event/event.h>

/* Define 2 rcommands: 1. draw a line; 2. rotate */

struct rcmd_draw_line {
	struct render_command base;

	/* from (0, 0) to (x, y) */
	float x, y, r_color;
	tick_t total_time;
};

static int
draw_line_render(struct render_command * __rcmd, dtick_t delta_ticks)
{
	struct rcmd_draw_line * rcmd =
		container_of(__rcmd, struct rcmd_draw_line, base);

	static float old_rcolor = 0;
	static int counter = 0;

	old_rcolor = rcmd->r_color;
	rcmd->total_time += delta_ticks;
	rcmd->x = (float)(rcmd->total_time % 4000) / 4000.0f;
	rcmd->y = (float)(rcmd->total_time % 6000) / 6000.0f;

	rcmd->r_color = (float)(rcmd->total_time % 1000) / 1000.0f;

	glColor3f(rcmd->r_color, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex2d(0, 0);
	glVertex2d(rcmd->x, rcmd->y);
	glVertex2d(-1, 0);
	glVertex2d( 1, 0);
	glVertex2d(0, -1);
	glVertex2d(0, 1);
	glEnd();
}

static int
draw_line_snprintf(struct render_command * __rcmd, char * dest, int length)
{
	struct rcmd_draw_line * rcmd = container_of(__rcmd, struct rcmd_draw_line, base);
	return snprintf(dest, length, "draw line from (0,0) to (%f, %f)\n",
			rcmd->x, rcmd->y);
}

static int
draw_line_remove(struct render_command * __rcmd,
		rcmd_remove_reason_t reason, int flags)
{
	
	struct rcmd_draw_line * rcmd = container_of(__rcmd, struct rcmd_draw_line, base);
	VERBOSE(VIDEO, "Remove drawline cmd\n");
	free(rcmd);
	return 0;
}

struct rcmd_operations draw_line_ops = {
	.render		= draw_line_render,
	.snprintf	= draw_line_snprintf,
	.remove		= draw_line_remove,
};

struct render_command * alloc_drawline(struct video_context * ctx,
		float x, float y)
{
	struct rcmd_draw_line * cmd = calloc(1, sizeof(*cmd));
	rcmd_init(&(cmd->base),
			"DrawLine",
			FALSE,
			ctx,
			&draw_line_ops);
	cmd->base.pprivate = cmd;
	cmd->x = x;
	cmd->y = y;
	cmd->total_time = 0;
	return &(cmd->base);
}

int main(int argc, char * argv[])
{
	DEBUG_INIT(NULL);
	VERBOSE(SYSTEM, "Start!!!\n");

	struct video_context * video_ctx = NULL;

	volatile struct exception exp;
	TRY_CATCH(exp, MASK_ALL) {
		conf_init(argc, argv);
		video_ctx = video_init();
		event_init();

		int mspf = conf_get_integer("video.mspf", 50);	/* fps=20 */
		int mspf_fallback = conf_get_integer("video.mspf.fallback", 100);

		if (mspf > mspf_fallback)
			mspf = mspf_fallback;

		VERBOSE(VIDEO, "Desired fps is %f\n", 1000.0 / mspf);
		VERBOSE(VIDEO, "Fps fallback is %f\n", 1000.0 / mspf_fallback);

		/* Link commands */
		struct render_command * draw_line = alloc_drawline(video_ctx,
				0.5, 0.6);
	}
	switch (exp.level) {
		case (EXCEPTION_NO_ERROR):
			VERBOSE(SYSTEM, "No error!\n");
			break;
		case (EXCEPTION_USER_QUIT):
			VERBOSE(SYSTEM, "User quit\n");
			break;
		default:
			ERROR(SYSTEM, "Error out\n");
	}

	show_mem_info();

	return 0;
}

