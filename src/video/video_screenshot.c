/* by WN @ Feb 19, 2009 */

#include <config.h>
#include <common/defs.h>
#include <common/debug.h>
#include <common/exception.h>
#include <common/utils.h>

#include <common/mm.h>

#include <econfig/econfig.h>
#include <video/video.h>
#include <video/video_engine.h>


#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>

struct screenshot_cleanup {
	GC_TAG;
	struct cleanup base;
	void * buffer;
};

static void
do_screenshot_cleanup(struct cleanup * base)
{
	struct screenshot_cleanup * pcleanup =
		(struct screenshot_cleanup *)base->args;
	if (pcleanup->buffer != NULL) {
		GC_FREE_BLOCK_SET(pcleanup->buffer);
	}
	GC_TRIVAL_FREE(pcleanup);
}

static struct screenshot_cleanup *
alloc_screenshot_cleanup(void)
{
	struct screenshot_cleanup * pcleanup;
	pcleanup = GC_TRIVAL_CALLOC(pcleanup);
	assert(pcleanup != NULL);
	pcleanup->base.function = do_screenshot_cleanup;
	pcleanup->base.args = pcleanup;

	pcleanup->buffer = NULL;
	return pcleanup;
}

static void
__video_screen_shot(void)
{
	struct view_port vp;
	struct video_context * video_ctx = video_get_current_context();
	struct screenshot_cleanup * pcleanup = NULL;
	uint8_t * buffer = NULL;

	if (video_ctx == NULL) {
		WARNING(VIDEO, "Video system has not beed inited\n");
		return;
	}

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
		return;
	}

	char * filename;

	filename = alloca(64);
	memset(filename, '\0', 64);
	strftime(filename, 64, "YAAVG-%Y%m%d%H%M%S.png", ptm);

	const char * prefix = conf_get_string("video.screenshotdir", "/tmp");

	/* Test the path */
	struct stat statb;
	if ((stat(prefix, &statb)) || (!S_ISDIR(statb.st_mode))) {
		ERROR(SYSTEM, "screenshotdir %s is not a valid directory\n",
				prefix);
		return;
	}

	char * fullname = alloca(strlen(prefix) + strlen(filename) + 1);
	sprintf(fullname, "%s/%s", prefix, filename);
	VERBOSE(VIDEO, "Taking screen shot: %s\n", fullname);

	pcleanup = alloc_screenshot_cleanup();

	make_cleanup(&pcleanup->base);
	vp = video_ctx->view_port;

	buffer = GC_MALLOC_BLOCK(sizeof(uint8_t) * vp.w * vp.h * 4);
	assert(buffer != NULL);
	pcleanup->buffer = buffer;

	engine_read_pixels_rgb(buffer, vp);

	write_to_pngfile_rgb(fullname, buffer, vp.w, vp.h);
}

void
video_screen_shot(void)
{
	
#ifndef ENABLE_SCREENSHOT
	WARNING(VIDEO, "Screen shot is disabled in compiling.\n");
	return;
#else
	volatile struct exception exp;
	TRY_CATCH(exp, MASK_CONTINUE) {
		__video_screen_shot();
	}
	CATCH(exp) {
		case (EXCEPTION_CONTINUE):
			ERROR(SYSTEM, "error when taking exceptions: %s\n",
					exp.message);
		case (EXCEPTION_NO_ERROR):
			break;
		default:
			INTERNAL_ERROR(SYSTEM, "exp.level %d shouldn't be catched here\n", exp.level);
	}
#endif
}

// vim:tabstop=4:shiftwidth=4

