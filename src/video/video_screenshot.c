/* by WN @ Feb 19, 2009 */

#include <config.h>
#include <common/defs.h>
#include <common/debug.h>
#include <common/exception.h>
#include <common/utils.h>
#include <econfig/econfig.h>
#include <video/video.h>
#include <video/video_driver.h>


#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>

struct screenshot_cleanup {
	struct cleanup base;
	void * buffer;
};

static void
do_screenshot_cleanup(struct cleanup * base)
{
	struct screenshot_cleanup * pcleanup =
		(struct screenshot_cleanup *)base->args;
	if (pcleanup->buffer != NULL) {
		free(pcleanup->buffer);
		pcleanup->buffer = NULL;
	}
	free(pcleanup);
}

static struct screenshot_cleanup *
alloc_screenshot_cleanup(void)
{
	struct screenshot_cleanup * pcleanup;
	pcleanup = calloc(1, sizeof(*pcleanup));
	assert(pcleanup != NULL);
	pcleanup->base.function = do_screenshot_cleanup;
	pcleanup->base.args = pcleanup;

	pcleanup->buffer = NULL;
	return pcleanup;
}

void
video_screen_shot(void)
{
#ifndef ENABLE_SCREENSHOT
	WARNING(VIDEO, "Screen shot is disabled in compiling.\n");
	return;
#else
	
	int err;
	struct view_port vp;
	struct video_context * video_ctx = video_get_current_context();
	struct screenshot_cleanup * pcleanup = NULL;
	uint8_t * buffer = NULL;

	if (video_ctx == NULL) {
		WARNING(VIDEO, "Video system has not beed inited\n");
		return;
	}

	pcleanup = alloc_screenshot_cleanup();

	make_cleanup(&pcleanup->base);
	vp = video_ctx->view_port;

	buffer = malloc(sizeof(uint8_t) * vp.w * vp.h * 3);
	assert(buffer != NULL);
	pcleanup->buffer = buffer;

	driver_read_pixels_rgb(buffer, vp);

	/* Generate filename */
	struct tm tm, *ptm;
	time_t stime;
	stime = time(NULL);
	if (stime == (time_t)(-1)) {
		WARNING(VIDEO, "Get current time failed\n");
		stime = 0;
	}

	ptm = localtime_r(&stime, &tm);
	if (ptm != &tm) {
		ERROR(VIDEO, "Cover to struct tm failed\n");
		throw_exception(EXCEPTION_CONTINUE, "Cover to struct tm failed");
	}

	char * filename;
	filename = alloca(64);
	memset(filename, '\0', 64);
	strftime(filename, 64, "YAAVG-%Y%m%d%H%M%s.png", ptm);
	VERBOSE(VIDEO, "Taking screen shot to file %s\n", filename);

	write_to_pngfile_rgb(filename, buffer, vp.w, vp.h);
#endif
}


// vim:tabstop=4:shiftwidth=4

