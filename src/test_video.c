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
	float x, y, r_color;
	tick_t total_time;
};

int DrawLinePhy(struct RenderCommand * cmd, dtick_t delta_time)
{
	struct CommDrawLine * base = container_of(cmd, struct CommDrawLine, cmd);

	static float old_rcolor = 0;
	static int counter = 0;

	old_rcolor = base->r_color;

	base->total_time += delta_time;

	base->x = (float)(base->total_time % 4000) / 4000.0f;
	base->y = (float)(base->total_time % 6000) / 6000.0f;
	base->r_color = (float)(base->total_time % 1000) / 1000.0f;
#if 0
	if (base->r_color < old_rcolor) {
		counter ++;
		printf("counter=%d\n", counter);
	}
#endif
	return 0;
}

int DrawLineRender(struct RenderCommand * cmd)
{
	struct CommDrawLine * base = container_of(cmd, struct CommDrawLine, cmd);

	glColor3f(base->r_color, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex2d(0, 0);
	glVertex2d(base->x, base->y);
	glEnd();

	return 0;
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
			DrawLinePhy,
			DrawLineRender,
			DrawLineSprintf,
			DrawLineRemove);
	cmd->x = x;
	cmd->y = y;
	cmd->total_time = 0;
	return &(cmd->cmd);
}

struct CommClear {
	struct RenderCommand cmd;
};

int ClearRender(struct RenderCommand * cmd)
{
	glClear(GL_COLOR_BUFFER_BIT);
	return 0;
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
			NULL,
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

//	int fps = ConfGetInteger("video.fps", 30);
//	VERBOSE(VIDEO, "Desired fps is %d\n", fps);
//	int interval = 1000 / fps;

	int mspf = ConfGetInteger("video.mspf", 50);	/* fps=20 */
	int mspf_fallback = ConfGetInteger("video.mspf.fallback", 100);

	if (mspf > mspf_fallback)
		mspf = mspf_fallback;

	VERBOSE(VIDEO, "Desired fps is %f\n", 1000.0 / mspf);
	VERBOSE(VIDEO, "Fps fallback is %f\n", 1000.0 / mspf_fallback);

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
	int frames = 0;

	tick_t realtime, oldrealtime;
	dtick_t deltatime;

	realtime = GetTicks();
	while(!event) {
		int render_time;
		oldrealtime = realtime;
		realtime = GetTicks();
		deltatime = realtime - oldrealtime;

		if (deltatime > mspf_fallback)
			deltatime = mspf_fallback;
		if (deltatime < 0) {
			WARNING(VIDEO, "Time stepped backwards, from %u to %u\n",
					oldrealtime, realtime);
			deltatime = 0;
		}

		VideoPhy(deltatime);
		VideoRender();

		render_time = GetTicks() - realtime;

		if (render_time < mspf)
			Delay(mspf - render_time);
	
//		Delay(1000);

//		VideoRender(ticks_start);
#if 0
		glColor3f (0.2, 0.4, 0.7);
		glBegin(GL_LINES);
			glVertex2d(0,0);
			glVertex2d(1,1);
		glEnd();
#endif
		VideoSwapBuffers();
#if 0
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
		if (ticks_end - ticks_start < mspf)
			Delay(mspf - (ticks_end - ticks_start));
#endif			
		event = EventPoll();
	}

err_eclose:
	VideoClose();
	show_mem_info();
	return 0;
}

