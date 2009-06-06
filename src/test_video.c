/* 
 * test_video.c
 * by WN @ Mar. 15, 2009
 */

#include <stdio.h>
#include <common/math.h>
#include <common/debug.h>
#include <common/exception.h>
#include <common/utils.h>
#include <econfig/econfig.h>

#include <video/video.h>
#include <video/video_gl.h>
#include <video/texture_gl.h>
#include <event/event.h>

/* Define 2 rcommands: 1. draw a line; 2. rotate */

struct rcmd_draw_line {
	struct render_command base;

	/* from (0, 0) to (x, y) */
	float x, y, r_color;
	tick_t total_time;

	struct texture_gl * tex;
	struct texture_gl * tex2;
};

static int
draw_line_render(struct render_command * __rcmd, dtick_t delta_ticks)
{
	struct rcmd_draw_line * rcmd =
		container_of(__rcmd, struct rcmd_draw_line, base);

	static float old_rcolor = 0;

	old_rcolor = rcmd->r_color;
	rcmd->total_time += delta_ticks;
	rcmd->x = (float)(rcmd->total_time % 4000) / 4000.0f;
	rcmd->y = (float)(rcmd->total_time % 6000) / 6000.0f;

	rcmd->r_color = (float)(rcmd->total_time % 1000) / 1000.0f;

	glColor3f(rcmd->r_color, 0.0f, 1.0f);
#if 0
	glBegin(GL_LINES);
	glVertex2d(-rcmd->x / 3.0f, -rcmd->y / 3.0f);
	glVertex2d(rcmd->x / 3.0f, rcmd->y / 3.0f);
	glEnd();
#endif

	/* FIXME! */
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, rcmd->tex->hwtexs[0]);
	glPolygonMode(GL_FRONT, GL_FILL);
	glPolygonMode(GL_BACK, GL_FILL);
	glBegin(GL_POLYGON);

	glTexCoord2f(1.0, 1.0);
	glVertex2d(0.0, 0.0);
	glTexCoord2f(0.0, 1.0);
	glVertex2d(-0.4, 0.0);
	glTexCoord2f(0.0, 0.0);
	glVertex2d(-0.4, -0.4);
	glTexCoord2f(1.0, 0.0);
	glVertex2d(0.0, -0.4);

	static int ttt = 0;

#if 0
	/* reinit subsystem */
	if (ttt > 2)
#endif
		glEnd();
	ttt ++;
#if 0

	/* FIXME! */
	glBindTexture(GL_TEXTURE_2D, rcmd->tex2->hwtexs[0]);
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0);
	glVertex2d(-0.4, -0.4);
	glTexCoord2f(0.0, 1.0);
	glVertex2d(-0.4, 0.4);
	glTexCoord2f(1.0, 1.0);
	glVertex2d(0.4, 0.4);
	glTexCoord2f(1.0, 0.0);
	glVertex2d(0.4, -0.4);
	glEnd();
#endif
#if 0
	glVertex2d(-1, 0);
	glVertex2d( 1, 0);
	glVertex2d(0, -1);
	glVertex2d(0, 1);
#endif
	return RENDER_OK;
}

static int
draw_line_snprintf(struct render_command * __rcmd, char * dest, int length)
{
	struct rcmd_draw_line * rcmd =
		container_of(__rcmd, struct rcmd_draw_line, base);
	return snprintf(dest, length, "draw line from (0,0) to (%f, %f)\n",
			rcmd->x, rcmd->y);
}

static int
draw_line_remove(struct render_command * __rcmd,
		rcmd_remove_reason_t reason, int flags)
{
	
	struct rcmd_draw_line * rcmd =
		container_of(__rcmd, struct rcmd_draw_line, base);
	if (rcmd->tex)
		TEXGL_RELEASE(rcmd->tex);
	if (rcmd->tex2)
		TEXGL_RELEASE(rcmd->tex2);
	VERBOSE(VIDEO, "Remove drawline cmd\n");
	free(rcmd);
	return 0;
}

static struct rcmd_operations draw_line_ops = {
	.render		= draw_line_render,
	.snprintf	= draw_line_snprintf,
	.remove		= draw_line_remove,
};

struct render_command * alloc_draw_line(struct video_context * ctx,
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

	res_id_t texres;
	char * fn = "common/rgb.png";
	char * fn2 = "common/rgba.png";
	texres = (uint64_t)(uint32_t)fn;
	struct rectangle rect = {
		0,0,4096,4096
	};


	GL_POP_ERROR();
	cmd->tex = texgl_create(texres, rect, NULL, NULL);
	TEXGL_GRAB(cmd->tex);


	GL_POP_ERROR();
#if 0
	texres = (uint64_t)(uint32_t)fn2;
	cmd->tex2 = texgl_create(texres, rect, NULL, NULL);
	TEXGL_GRAB(cmd->tex2);
#endif
	glBindTexture(GL_TEXTURE_2D, 0);

	return &(cmd->base);
}

struct rcmd_clear {
	struct render_command base;
};

static int
clear_snprintf(struct render_command * __rcmd, char * dest, int length)
{
	return snprintf(dest, length, "Clear cmd\n");
}

static int
clear_render(struct render_command * __rcmd, dtick_t delta_ticks)
{
	glClear(GL_COLOR_BUFFER_BIT);
	return 0;
}

static int
clear_remove(struct render_command * __rcmd,
		rcmd_remove_reason_t reason, int flags)
{
	struct rcmd_clear * rcmd =
		container_of(__rcmd, struct rcmd_clear, base);
	VERBOSE(VIDEO, "Remove clear cmd\n");
	free(rcmd);
	return 0;
}

static struct rcmd_operations clear_ops = {
	.render		= clear_render,
	.snprintf	= clear_snprintf,
	.remove		= clear_remove,
};

struct render_command *
alloc_clear(struct video_context * ctx)
{
	struct rcmd_clear * cmd = calloc(1, sizeof(*cmd));
	rcmd_init(&(cmd->base),
			"Clear",
			FALSE,
			ctx,
			&clear_ops);
	cmd->base.pprivate = cmd;
	return &(cmd->base);
}

struct rcmd_rotate {
	struct render_command base;
	float angle;
};

static int
rotate_render(struct render_command * __rcmd, dtick_t delta_ticks)
{
	struct rcmd_rotate * base = container_of(__rcmd, struct rcmd_rotate, base);
	base->angle += (float)delta_ticks / 1000.0f;
#if 0
	if (base->angle >= 360.0f)
		base->angle = 0.0f;
	if (base->angle <= -360.0f)
		base->angle = 0.0f;
#endif
	return 0;
}

static int
rotate_render_l(struct render_command * __rcmd, dtick_t delta_ticks)
{
	rotate_render(__rcmd, delta_ticks);
	struct rcmd_rotate * rcmd = container_of(__rcmd, struct rcmd_rotate, base);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(0.5, 0.5, 0.0);
	glRotatef(sinf((float)rcmd->angle) * (10.0f), 0,0,1);
#if 1
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex2d(-0.4, 0);
	glVertex2d(0.4, 0);
	glVertex2d(0, -0.4);
	glVertex2d(0, 0.4);
	glEnd();
#endif
	return 0;
}

static int
rotate_render_r(struct render_command * __rcmd, dtick_t delta_ticks)
{
	rotate_render(__rcmd, delta_ticks);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

#if 1
	glColor3f(1.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex2d(0.4, 0.5);
	glVertex2d(0.6, 0.5);
	glVertex2d(0.5, 0.4);
	glVertex2d(0.5, 0.6);
	glEnd();
#endif
	return 0;
}

static
int rotate_snprintf(struct render_command * __rcmd, char * dest, int length)
{
	if (rcmd_is_left(__rcmd))
		return snprintf(dest, length, "rotate (left)\n");
	else
		return snprintf(dest, length, "rotate (right)\n");
}

static int
rotate_remove(struct render_command * __rcmd,
		rcmd_remove_reason_t reason, int flags)
{
	struct rcmd_rotate * rcmd = container_of(__rcmd, struct rcmd_rotate, base);
	VERBOSE(VIDEO, "Remove rotate cmd\n");
	free(rcmd);
	return 0;
}

static struct rcmd_operations rotate_ops = {
	.render		= NULL,
	.lrender	= rotate_render_l,
	.rrender	= rotate_render_r,
	.snprintf	= rotate_snprintf,
	.remove		= rotate_remove,
};

static void
alloc_rotates(struct video_context * ctx,
		struct render_command ** left,
		struct render_command ** right)
{
	struct rcmd_rotate * lcmd, *rcmd;
	lcmd = calloc(1, sizeof(*lcmd));
	rcmd = calloc(1, sizeof(*rcmd));
	rcmd_init(&lcmd->base,
			"Rotate (left)",
			FALSE,
			ctx,
			&rotate_ops);

	rcmd_init(&rcmd->base,
			"Rotate (right)",
			FALSE,
			ctx,
			&rotate_ops);
	rcmd->angle = lcmd->angle = 0.0f;
	*left = &lcmd->base;
	*right = &rcmd->base;
}

/* ********************************************* */

static struct time_controller {
	tick_t realtime, oldrealtime;
	dtick_t deltatime;
	int mspf_fallback;
	int mspf;
} time_controller;

static void
frame(struct video_context * video_ctx, dtick_t deltatime,
		int event)
{
	volatile struct exception exp;
entry:
	TRY_CATCH(exp, MASK_SUBSYS_ALL) {
		video_render(deltatime);
		video_swap_buffers();
	}
	CATCH(exp)
	{
		case EXCEPTION_NO_ERROR:
			break;
		case EXCEPTION_SUBSYS_RERUN:
			WARNING(VIDEO, "video frame rerender: %s\n", exp.message);
			goto entry;
			break;
		case EXCEPTION_SUBSYS_SKIPFRAME:
			WARNING(VIDEO, "video frame skipped: %s\n", exp.message);
			break;
		case EXCEPTION_SUBSYS_REINIT:
			WARNING(VIDEO, "video reinit: %s\n", exp.message);
			video_reinit();
			THROW(EXCEPTION_SYS_SKIPFRAME, "video reinit, skip this frame");
			break;
		default:
			INTERNAL_ERROR(SYSTEM, "!@#$%^&\n");
			break;
	}
}

static void
render(struct video_context * video_ctx)
{

	/* start render */
	int event = event_poll();
	int frames = 0;

	tick_t realtime, oldrealtime;
	dtick_t deltatime;
	tick_t start_time;


	start_time = realtime = get_ticks();

	while((event != 1)) {
		oldrealtime = realtime;
		realtime = get_ticks();
		deltatime = realtime - oldrealtime;
		
		if (frames >= 300) {
			VERBOSE(VIDEO, "300 frames in %d ticks\n", realtime - start_time);
			VERBOSE(VIDEO, "fps=%f\n", 300.0f / ((realtime - start_time) / 1000.0f));
			frames = 0;
			start_time = realtime;
		}

		int render_time;

		if (deltatime > time_controller.mspf_fallback)
			deltatime = time_controller.mspf_fallback;
		if (deltatime < 0) {
			WARNING(VIDEO, "Time stepped backwards, from %u to %u\n",
					oldrealtime, realtime);
			deltatime = 0;
		}

		time_controller.realtime = realtime;
		time_controller.oldrealtime = oldrealtime;
		time_controller.deltatime = deltatime;

		struct exception exp;

entry:
		TRY_CATCH(exp, MASK_SYS_ALL) {
			if (event == 2) {
				/* XXX econfig is not finished yet! if econfig.c
				 * doesn't contain such entry, the set and get both do
				 * nothing! XXX */

				if (conf_get_bool("video.fullscreen", TRUE)) {
					conf_set_integer("video.resolution.w", 800);
					conf_set_integer("video.resolution.h", 600);
					conf_set_bool("video.fullscreen", FALSE);
				}
				else {
					conf_set_integer("video.resolution.w", 1280);
					conf_set_integer("video.resolution.h", 800);
					conf_set_bool("video.fullscreen", TRUE);
				}
				conf_set_string("video.opengl.gllibrary", NULL);
				THROW(EXCEPTION_SYS_REINIT, "normal reinit");
			}
			if (event == 3)
				video_screen_shot();

			if (event == 4) {
				THROW(EXCEPTION_SYS_RERUN, "normal rerun");
			}

			if (event == 5) {
				THROW(EXCEPTION_SYS_SKIPFRAME, "normal skip");
			}

			frame(video_ctx, deltatime, event);
			frames ++;
		}
		CATCH(exp) {
			case EXCEPTION_NO_ERROR:
				break;
			case EXCEPTION_SYS_RERUN:
				WARNING(SYSTEM, "rerun this frame: %s\n", exp.message);
				event = 0;
				goto entry;
				break;
			case EXCEPTION_SYS_SKIPFRAME:
				WARNING(SYSTEM, "skip this frame: %s\n", exp.message);
				delay(time_controller.mspf * 10);
				event = 0;
				break;
			case EXCEPTION_SYS_REINIT:
				VERBOSE(SYSTEM, "System reinit: %s\n", exp.message);
				video_reinit();
				break;
			default:
				print_exception(FATAL, SYSTEM, exp);
				INTERNAL_ERROR(SYSTEM, "@!#!@$%\n");
				break;
		}

		if (time_controller.mspf > 0) {
			render_time = get_ticks() - realtime;
			if (render_time < time_controller.mspf)
				delay(time_controller.mspf - render_time);
		}

		/* ???? */
		event = event_poll();
	}


}

int main(int argc, char * argv[])
{
	DEBUG_INIT(NULL);
//	DEBUG_INIT("/tmp/debug");
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

		time_controller.mspf = mspf;
		time_controller.mspf_fallback = mspf_fallback;

		/* Link commands */
		struct render_command * draw_line = alloc_draw_line(video_ctx,
				0.5, 0.6);
		struct render_command * clear = alloc_clear(video_ctx);
		struct render_command * lrotate, * rrotate;

		alloc_rotates(video_ctx, &lrotate, &rrotate);

		VERBOSE(VIDEO, "Insert draw_line\n");
		video_insert_command(draw_line, AFTER, NULL);
		VERBOSE(VIDEO, "Insert clear\n");
		video_insert_command(clear, BEFORE, draw_line);
		VERBOSE(VIDEO, "Insert pair\n");
		video_insert_command_pair(
				lrotate, BEFORE, draw_line,
				rrotate, AFTER, draw_line);
		VERBOSE(VIDEO, "Insert over\n");

		rcmd_set_active(draw_line);
		rcmd_set_active(clear);
		rcmd_set_active(lrotate);

		/* print commands */
		char * buffer = malloc(4096);
		rlist_snprintf(buffer, 4096, &(video_ctx->render_list));
		printf("%s\n", buffer);
		free(buffer);

		render(video_ctx);
	}
	CATCH(exp) {
		case (EXCEPTION_NO_ERROR):
			VERBOSE(SYSTEM, "No error!\n");
			break;
		case (EXCEPTION_USER_QUIT):
			VERBOSE(SYSTEM, "User quit: %s\n", exp.message);
			break;
		default:
			print_exception(ERROR, SYSTEM, exp);
	}

	do_cleanup();
	gc_cleanup();

	show_mem_info();

	return 0;
}
// vim:tabstop=4:shiftwidth=4

