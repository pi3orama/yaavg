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
	glVertex2d(-1, 0);
	glVertex2d( 1, 0);
	glVertex2d(0, -1);
	glVertex2d(0, 1);
	glEnd();

	return 0;
}

int DrawLineSprintf(struct RenderCommand * cmd, char * dest)
{
	struct CommDrawLine * base = container_of(cmd, struct CommDrawLine, cmd);
	return sprintf(dest, "draw line from (0,0) to (%f, %f)\n", base->x, base->y);
}

int DrawLineRemove(struct RenderCommand * command, RemoveReason_t r, int flags)
{
	struct CommDrawLine * base = container_of(command, struct CommDrawLine, cmd);
	VERBOSE(VIDEO, "Remove drawline cmd\n");
	free(base);
	return 0;
}

struct RenderCommandOperations draw_line_ops = {
	.phy		= DrawLinePhy,
	.render		= DrawLineRender,
	.sprintf	= DrawLineSprintf,
	.remove		= DrawLineRemove,
};

struct RenderCommand * alloc_drawline(struct VideoContext * context,
		float x, float y)
{
	struct CommDrawLine * cmd = malloc(sizeof(*cmd));
	RCommandInit(&(cmd->cmd),
			"DrawLine",
			FALSE,
			context,
			&draw_line_ops);
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

int ClearRemove(struct RenderCommand * cmd, RemoveReason_t r, int flags)
{
	struct CommClear * base = container_of(cmd, struct CommClear, cmd);
	VERBOSE(VIDEO, "Remove Clear cmd\n");
	free(base);
	return 0;
}

struct RenderCommandOperations clear_ops = {
	.render		= ClearRender,
	.sprintf	= ClearSprintf,
	.remove		= ClearRemove,
};

struct RenderCommand * alloc_clear(struct VideoContext * context)
{
	struct CommClear * cmd = malloc(sizeof(*cmd));
	RCommandInit(&(cmd->cmd),
			"Clear",
			FALSE,
			context,
			&clear_ops);
	return &(cmd->cmd);
}

struct CommRotate {
	struct RenderCommand cmd;
	float angle;
};

int
RotatePhy(struct RenderCommand * cmd, dtick_t delta_time)
{
	struct CommRotate * base = container_of(cmd, struct CommRotate, cmd);
	base->angle += (float)delta_time / 10000.0f * 360.0f;
	if (base->angle >= 360.0f)
		base->angle = 0.0f;
	if (base->angle <= -360.0f)
		base->angle = 0.0f;
	return 0;
}

int
RotateRenderL(struct RenderCommand * cmd)
{
	struct CommRotate * base = container_of(cmd, struct CommRotate, cmd);
	/* we need check context first... */
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRotatef(base->angle, 0,0,1);
#if 1
	glBegin(GL_LINES);
	glVertex2d(0, 0.5);
	glVertex2d(1, 0.5);
	glVertex2d(0.5, 0);
	glVertex2d(0.5, 1);
	glEnd();
#endif
	return 0;
}

int
RotateRenderR(struct RenderCommand * cmd)
{
	struct CommRotate * base = container_of(cmd, struct CommRotate, cmd);
	/* we need check context first... */
	glMatrixMode(GL_MODELVIEW);
//	glRotatef(base->angle, 0,0,1);
	glPopMatrix();
#if 1
	glBegin(GL_LINES);
	glVertex2d(0, 0.5);
	glVertex2d(1, 0.5);
	glVertex2d(0.5, 0);
	glVertex2d(0.5, 1);
	glEnd();
#endif
	return 0;
}

int
RotateSprintf(struct RenderCommand * cmd, char * dest)
{
	struct CommRotate * base = container_of(cmd, struct CommRotate, cmd);
	return sprintf(dest, "rotate %.3f from (0,0,0)\n", base->angle);
}

int
RotateRemove(struct RenderCommand * cmd, RemoveReason_t r, int flags)
{
	struct CommRotate * base = container_of(cmd, struct CommRotate, cmd);
	VERBOSE(VIDEO, "Remove rotate\n");
	free(base);
	return 0;
}

struct RenderCommandOperations rotate_ops = {
	.phy		= RotatePhy,
	.render		= NULL,
	.lrender	= RotateRenderL,
	.rrender	= RotateRenderR,
	.sprintf	= RotateSprintf,
	.remove		= RotateRemove,
};

int alloc_rotates(struct VideoContext * context,
		struct RenderCommand ** left,
		struct RenderCommand ** right)
{
	struct CommRotate * lcmd = malloc(sizeof(*lcmd));
	assert(lcmd != NULL);

	struct CommRotate * rcmd = malloc(sizeof(*rcmd));
	assert(rcmd != NULL);

	RCommandInit(&(lcmd->cmd),
			"Rotate(left)",
			FALSE,
			context,
			&rotate_ops);

	RCommandInit(&(rcmd->cmd),
			"Rotate(right)",
			TRUE,
			context,
			&rotate_ops);

	lcmd->angle = 0;
	rcmd->angle = 0;
	*left = &lcmd->cmd;
	*right = &rcmd->cmd;
	return 0;
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
	struct RenderCommand * draw_line = alloc_drawline(vcontext,
			0.5, 0.6);
	struct RenderCommand * clear = alloc_clear(vcontext);
	struct RenderCommand * lrotate, * rrotate;

	alloc_rotates(vcontext, &lrotate, &rrotate);
	VideoSetCaption("Test");

	/* Insert commands */
	VideoInsertCommand(draw_line, AFTER, NULL);
	VideoInsertCommand(clear, AFTER, NULL);
	VideoInsertCommandPair(lrotate,
			AFTER,
			clear,
			rrotate,
			BEFORE,
			NULL);

	RCommandSetActive(draw_line);
	RCommandSetActive(clear);
	RCommandSetActive(lrotate);

	/* print commands */
	char * buffer = malloc(4096);
	rlist_sprint(buffer, &(vcontext->render_list));
	printf("%s\n", buffer);
	free(buffer);

	/* start render */
	int event = EventPoll();
	int frames = 0;

	tick_t realtime, oldrealtime;
	dtick_t deltatime;

	realtime = GetTicks();
	while((event == 0) || (event == 2) || (event == 3)) {

		if (event == 2) {
			/* XXX econfig is not finished yet! if econfig.c
			 * doesn't contain such entry, the set and get both do
			 * nothing! XXX */
#if 0
			if (ConfGetBool("video.fullscreen", TRUE))
				ConfSetBool("video.fullscreen", FALSE);
			else
				ConfSetBool("video.fullscreen", TRUE);
#endif
			ConfSetString("video.opengl.driver.gllibrary", NULL);
			
			VideoReopenWindow(vcontext);
			VideoSetCaption("Test!");
		}

		if (event == 3) {
			VideoScreenShot();
		}

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

