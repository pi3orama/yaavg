/* by WN @ Jan 29, 2009 */

#include <stdio.h>
#include <common/debug.h>
#include <common/utils.h>
#include <econfig/econfig.h>

#include <video/video.h>
#include <video/engine_gl.h>

#include <event/event.h>


/* Define 2 rcommand: 1. draw a line; 2. rotate */

struct CommDrawLine {
	struct RenderCommand cmd;
	/* from (0, 0) to (x, y) */
	float x, y;
};

int DrawLineRender(struct RenderCommand * cmd, tick_t current_time)
{
	struct CommDrawLine * base = container_of(cmd, struct CommDrawLine, cmd);

	float r_color = (float)((current_time - cmd->start_time) % 4000) / 4000.0f;

	float y = r_color;

	glColor3f(r_color, 1.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex2d(0, 0);
	glVertex2d(base->x, y);
	glEnd();

	return RENDER_CONT;
}

int DrawLineSprintf(struct RenderCommand * cmd, char * dest)
{
	struct CommDrawLine * base = container_of(cmd, struct CommDrawLine, cmd);
	return sprintf(dest, "draw line from (0,0) to (%f, %f)\n", base->x, base->y);
}

int DrawLineRemove(struct RenderCommand * command, enum RemoveReason r, int flags)
{
	struct CommDrawLine * base = container_of(command, struct CommDrawLine, cmd);
	VERBOSE(VIDEO, "Remove drawline cmd\n");
	free(base);
	return 0;
}

struct RenderCommand * alloc_drawline(struct VideoContext * context,
		float x, float y)
{
	struct CommDrawLine * cmd = malloc(sizeof(*cmd));
	RCommandInit(&(cmd->cmd),
			"DrawLine",
			context,
			DrawLineRender,
			DrawLineSprintf,
			DrawLineRemove);
	cmd->x = x;
	cmd->y = y;
	return &(cmd->cmd);
}

struct CommClear {
	struct RenderCommand cmd;
};

int ClearRender(struct RenderCommand * cmd, tick_t current_time)
{
	glClear(GL_COLOR_BUFFER_BIT);
	return RENDER_CONT;
}

int ClearSprintf(struct RenderCommand * cmd, char * dest)
{
	return sprintf(dest, "clear color bit\n");
}

int ClearRemove(struct RenderCommand * cmd, enum RemoveReason r, int flags)
{
	struct CommClear * base = container_of(cmd, struct CommClear, cmd);
	VERBOSE(VIDEO, "Remove Clear cmd");
	free(base);
	return 0;
}

struct RenderCommand * alloc_clear(struct VideoContext * context)
{
	struct CommClear * cmd = malloc(sizeof(*cmd));
	RCommandInit(&(cmd->cmd),
			"Clear",
			context,
			ClearRender,
			ClearSprintf,
			ClearRemove);
	return &(cmd->cmd);
}


/* 
 * test video-engine, rlist and rcommand
 */

int main(int argc, char * argv[])
{

	struct VideoContext * vcontext = NULL;

	DEBUG_INIT(NULL);
	DEBUG_MSG(VERBOSE, SYSTEM, "System start!!!\n");

	ConfInit(argc, argv);
	VideoInit();

	vcontext = VideoOpenWindow();
	if (!vcontext) {
		FATAL(SYSTEM, "Unable to open window!\n");
		goto err_eclose;
	}

	EventInit();

	int fps = ConfGetInteger("video.fps", 30);
	VERBOSE(VIDEO, "Desired fps is %d\n", fps);
	int interval = 1000 / fps;


	/* Link commands */
	{
		struct RenderCommand * draw_line = alloc_drawline(vcontext,
				0.5, 0.6);
		struct RenderCommand * clear = alloc_clear(vcontext);
		VideoSetCaption("Test");
		RListLinkHead(&(vcontext->render_list), draw_line);
		RListLinkHead(&(vcontext->render_list), clear);
		char * buffer = malloc(4096);
		rlist_sprint(buffer, &(vcontext->render_list));
		printf("%s\n", buffer);
		free(buffer);
	}

	int event = EventPoll();
	tick_t ticks_start, ticks_end, ticks_T0;
	int frames = 0;
	ticks_T0 = GetTicks();

	while(!event) {
		ticks_start = GetTicks();

		VideoRender(ticks_start);
#if 0
		glColor3f (0.2, 0.4, 0.7);
		glBegin(GL_LINES);
			glVertex2d(0,0);
			glVertex2d(1,1);
		glEnd();
#endif
		VideoSwapBuffers();

		ticks_end = GetTicks();

		frames ++;

		/* per 100 frames, check fps */
		if (frames > 100) {
			tick_t T1 = ticks_end;
			float real_fps = 100.0 / ((float)(T1 - ticks_T0) / 1000.0f);

			frames = 0;
			ticks_T0 = ticks_end;
			TRACE(VIDEO, "FPS: %f\n", real_fps);
		}

		/* Check fps */
		if (ticks_end - ticks_start < interval)
			Delay(interval - (ticks_end - ticks_start));
			
		event = EventPoll();
	}

err_eclose:
	VideoClose();
	show_mem_info();
	return 0;
}

