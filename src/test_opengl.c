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

struct rcmd_draw {
	struct render_command base;
	tick_t total_time;
};

static int
draw_render(struct render_command * __rcmd,
		dtick_t delta_ticks)
{
	struct rcmd_draw * rcmd =
		container_of(__rcmd, struct rcmd_draw, base);
#if 0
	glBegin(GL_LINES);
	glVertex2d(0, 0);
	glVertex2d(1, 1);
	glEnd();
#endif
	return RENDER_OK;
}

static struct rcmd_operations draw_ops = {
	.render = draw_render,
};

struct rcmd_clear {
	struct render_command base;
};

static int
clear_render(struct render_command * __rcmd,
		dtick_t delta_ticks)
{
	glClear(GL_COLOR_BUFFER_BIT);
	return RENDER_OK;
}

static struct rcmd_operations clear_ops = {
	.render = clear_render,
};


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
//	DEBUG_INIT("/tmp/debug");
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

		time_controller.mspf = mspf;
		time_controller.mspf_fallback = mspf_fallback;

		/* link commands */
		struct rcmd_draw draw_cmd;
		rcmd_init(&(draw_cmd.base), "Draw", FALSE, video_ctx, &draw_ops);
		struct rcmd_clear clear_cmd;
		rcmd_init(&(clear_cmd.base), "Clear", FALSE, video_ctx, &clear_ops);
		video_insert_command(&(draw_cmd.base), AFTER, NULL);
		video_insert_command(&(clear_cmd.base), AFTER, NULL);
		rcmd_set_active(&(draw_cmd.base));
		rcmd_set_active(&(clear_cmd.base));

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

